import { copyFileSync, readFileSync } from 'node:fs';
import { pathToFileURL } from 'node:url';

const [tfJs, tfWasm, tfLib] = process.argv.slice(2);
if (!tfJs || !tfWasm || !tfLib) {
  throw new Error('usage: wasm_smoke.mjs <tf.js> <tf.wasm> <tf-lib>');
}

const modulePath = `${tfJs}.smoke.mjs`;
copyFileSync(tfJs, modulePath);

const { default: createTinyFugue } = await import(pathToFileURL(modulePath));

function bytes(text) {
  return [...Buffer.from(text, 'utf8')];
}

function createRelay(worlds = [], permissions = {}) {
  const state = {
    sends: [],
    closed: [],
    denied: [],
    resolved: [],
    opened: [],
    connected: [],
    nextFd: 64,
    sockets: new Map()
  };
  const relay = {
    resolve(destination) {
      state.resolved.push(destination);
    },
    open(domain, type, protocol, destination) {
      const fd = state.nextFd++;
      state.opened.push({ domain, type, protocol, destination });
      state.sockets.set(fd, {
        connected: false,
        inbound: [Buffer.from('relay inbound')]
      });
      return fd;
    },
    connect(fd) {
      const socket = state.sockets.get(fd);
      if (!socket) {
        return -1;
      }
      socket.connected = true;
      state.connected.push(fd);
      return 0;
    },
    close(fd) {
      state.closed.push(fd);
      state.sockets.delete(fd);
      return 0;
    },
    recv(fd, len) {
      const socket = state.sockets.get(fd);
      if (!socket || socket.inbound.length === 0) {
        return new Uint8Array();
      }
      const data = socket.inbound[0];
      const chunk = data.subarray(0, len);
      if (chunk.length === data.length) {
        socket.inbound.shift();
      } else {
        socket.inbound[0] = data.subarray(chunk.length);
      }
      return chunk;
    },
    send(fd, data) {
      if (!state.sockets.has(fd)) {
        return -1;
      }
      state.sends.push(Buffer.from(data).toString('utf8'));
      return data.length;
    },
    readReady(fd) {
      const socket = state.sockets.get(fd);
      return !!socket && socket.inbound.length > 0;
    },
    writeReady(fd) {
      return state.sockets.has(fd);
    },
    worlds() {
      return worlds;
    },
    permissions() {
      return permissions;
    },
    permissionDenied(details) {
      state.denied.push(details);
    }
  };
  return { relay, state };
}

const lowLevel = createRelay();
const output = [];
const wakeups = [];

const module = await createTinyFugue({
  noInitialRun: true,
  arguments: ['-L', tfLib, '-n', '-v'],
  wasmBinary: readFileSync(tfWasm),
  print: (text) => output.push(text),
  printErr: (text) => output.push(text),
  tfInputQueue: [],
  tfWriteStdout: (bytes) => output.push(Buffer.from(bytes).toString('utf8')),
  tfScheduleWake: (delayMs) => wakeups.push(delayMs),
  tfRelay: lowLevel.relay
});

for (const name of [
  '_main',
  '_tf_tick',
  '_tf_next_deadline_ms',
  '_tf_wasm_resize',
  '_tf_wasm_smoke',
  'ccall',
  'cwrap',
  'FS'
]) {
  if (!(name in module)) {
    throw new Error(`missing module export: ${name}`);
  }
}

const smokeResult = module._tf_wasm_smoke();
if (smokeResult !== 0) {
  throw new Error(`tf_wasm_smoke failed: ${smokeResult}`);
}
if (module._tf_wasm_resize(132, 43) !== 1) {
  throw new Error('tf_wasm_resize rejected a valid terminal size');
}
if (module._tf_wasm_resize(0, 43) !== 0) {
  throw new Error('tf_wasm_resize accepted an invalid terminal size');
}
if (!Number.isInteger(module.tfLastWakeDelayMs)) {
  throw new Error('wasm smoke did not report a scheduler wake delay');
}
if (wakeups.length === 0) {
  throw new Error('tfScheduleWake hook was not called');
}
if (!output.join('').includes('tinyfugue wasm smoke')) {
  throw new Error('tfWriteStdout hook was not called');
}
if (lowLevel.state.sends.join('') !== 'relay outbound') {
  throw new Error(`relay send hook was not called correctly: ${lowLevel.state.sends}`);
}
if (lowLevel.state.closed.length !== 1) {
  throw new Error('relay close hook was not called');
}
if (lowLevel.state.resolved.length !== 1 ||
    lowLevel.state.resolved[0].host !== 'mud.example' ||
    lowLevel.state.resolved[0].port !== '4000') {
  throw new Error(`relay resolve hook was not called correctly: ${JSON.stringify(lowLevel.state.resolved)}`);
}
if (lowLevel.state.opened.length !== 1 ||
    lowLevel.state.opened[0].destination?.host !== 'mud.example' ||
    lowLevel.state.opened[0].destination?.port !== '4000') {
  throw new Error(`relay open did not receive destination: ${JSON.stringify(lowLevel.state.opened)}`);
}

const mainRun = createRelay([
  { name: 'relay_world', type: 'telnet', host: 'mud.example', port: '4000' }
]);
const mainOutput = [];
const mainOptions = {
  arguments: ['-L/tf-lib', '-f', '-v', 'relay_world'],
  wasmBinary: readFileSync(tfWasm),
  print: (text) => mainOutput.push(text),
  printErr: (text) => mainOutput.push(text),
  tfInputQueue: bytes('/quit\n'),
  tfWriteStdout: (data) => mainOutput.push(Buffer.from(data).toString('utf8')),
  tfRelay: mainRun.relay
};
mainOptions.preRun = () => {
  mainOptions.FS.mkdir('/tf-lib');
  mainOptions.FS.mount(mainOptions.FS.filesystems.NODEFS, { root: tfLib }, '/tf-lib');
};
await createTinyFugue(mainOptions);

if (mainRun.state.resolved.length !== 1 ||
    mainRun.state.resolved[0].host !== 'mud.example' ||
    mainRun.state.resolved[0].port !== '4000') {
  throw new Error(`main did not resolve requested world: ${JSON.stringify(mainRun.state.resolved)}`);
}
if (mainRun.state.opened.length !== 1 ||
    mainRun.state.opened[0].destination?.host !== 'mud.example' ||
    mainRun.state.opened[0].destination?.port !== '4000') {
  throw new Error(`main did not open relay destination: ${JSON.stringify(mainRun.state.opened)}`);
}
if (mainRun.state.connected.length !== 1) {
  throw new Error('main did not connect relay socket');
}

const deniedRun = createRelay(
  [{ name: 'denied_world', type: 'telnet', host: 'denied.example', port: '4444' }],
  { allowConnections: [{ host: 'mud.example', port: '4000' }] }
);
const deniedOptions = {
  arguments: ['-L/tf-lib', '-f', '-v', 'denied_world'],
  wasmBinary: readFileSync(tfWasm),
  print: () => {},
  printErr: () => {},
  tfInputQueue: bytes('/quit\n'),
  tfRelay: deniedRun.relay
};
deniedOptions.preRun = () => {
  deniedOptions.FS.mkdir('/tf-lib');
  deniedOptions.FS.mount(deniedOptions.FS.filesystems.NODEFS, { root: tfLib }, '/tf-lib');
};
await createTinyFugue(deniedOptions);

if (deniedRun.state.denied.length !== 1 ||
    deniedRun.state.denied[0].type !== 'connect' ||
    deniedRun.state.denied[0].destination.host !== 'denied.example' ||
    deniedRun.state.denied[0].destination.port !== '4444') {
  throw new Error(`denied connection was not reported: ${JSON.stringify(deniedRun.state.denied)}`);
}
if (deniedRun.state.resolved.length !== 1 ||
    deniedRun.state.resolved[0].host !== 'denied.example' ||
    deniedRun.state.resolved[0].port !== '4444') {
  throw new Error(`denied world should still resolve by host/port: ${JSON.stringify(deniedRun.state.resolved)}`);
}
if (deniedRun.state.opened.length !== 0 ||
    deniedRun.state.connected.length !== 0) {
  throw new Error('denied connection should not open a relay socket');
}
