mergeInto(LibraryManager.library, {
  tf_wasm_write_stdout: function(ptr, len) {
    var bytes = HEAPU8.slice(ptr, ptr + len);
    if (Module['tfWriteStdout']) {
      Module['tfWriteStdout'](bytes);
    } else {
      var text = '';
      for (var i = 0; i < bytes.length; i++) {
        text += String.fromCharCode(bytes[i]);
      }
      if (Module['print']) {
        Module['print'](text);
      }
    }
    return len;
  },

  tf_wasm_read_stdin: function(ptr, len) {
    var queue = Module['tfInputQueue'];
    if (!queue || !queue.length) {
      return 0;
    }

    var count = Math.min(len, queue.length);
    for (var i = 0; i < count; i++) {
      HEAPU8[ptr + i] = queue.shift();
    }
    return count;
  },

  tf_wasm_stdin_ready: function() {
    var queue = Module['tfInputQueue'];
    return queue && queue.length ? 1 : 0;
  },

  tf_wasm_schedule_wake: function(delay_ms) {
    Module['tfLastWakeDelayMs'] = delay_ms;
    if (Module['tfScheduleWake']) {
      Module['tfScheduleWake'](delay_ms);
    }
    return 0;
  },

  tf_wasm_socket_open: function(domain, type, protocol, is_ssl) {
    var relay = Module['tfRelay'];
    var destination = Module['tfPendingRelayDestination'];
    Module['tfPendingRelayDestination'] = null;
    if (!relay) {
      return -1;
    }
    var permissions = relay.permissions;
    if (typeof permissions == 'function') {
      permissions = permissions();
    }
    var sameDestination = function(rule) {
      if (!destination || !rule) {
        return false;
      }
      return String(rule.host) == String(destination.host) &&
        String(rule.port) == String(destination.port);
    };
    if (permissions && permissions.denyConnections &&
        permissions.denyConnections.some(sameDestination)) {
      if (relay.permissionDenied) {
        relay.permissionDenied({ type: 'connect', destination: destination });
      }
      return -1;
    }
    if (permissions && permissions.allowConnections &&
        !permissions.allowConnections.some(sameDestination)) {
      if (relay.permissionDenied) {
        relay.permissionDenied({ type: 'connect', destination: destination });
      }
      return -1;
    }
    return relay.open(domain, type, protocol, destination, is_ssl);
  },

  tf_wasm_socket_connect: function(fd) {
    var relay = Module['tfRelay'];
    if (!relay) {
      return -1;
    }
    return relay.connect(fd);
  },

  tf_wasm_socket_close: function(fd) {
    var relay = Module['tfRelay'];
    if (!relay) {
      return 0;
    }
    return relay.close(fd);
  },

  tf_wasm_socket_recv: function(fd, ptr, len, flags) {
    var relay = Module['tfRelay'];
    if (!relay) {
      return -1;
    }
    var bytes = relay.recv(fd, len, flags);
    if (!bytes) {
      return 0;
    }
    var count = Math.min(len, bytes.length);
    for (var i = 0; i < count; i++) {
      HEAPU8[ptr + i] = bytes[i];
    }
    return count;
  },

  tf_wasm_socket_send: function(fd, ptr, len, flags) {
    var relay = Module['tfRelay'];
    if (!relay) {
      return -1;
    }
    var bytes = HEAPU8.slice(ptr, ptr + len);
    return relay.send(fd, bytes, flags);
  },

  tf_wasm_socket_read_ready: function(fd) {
    var relay = Module['tfRelay'];
    return relay && relay.readReady(fd) ? 1 : 0;
  },

  tf_wasm_socket_write_ready: function(fd) {
    var relay = Module['tfRelay'];
    return relay && relay.writeReady(fd) ? 1 : 0;
  },

  tf_wasm_resolve: function(name_ptr, name_len, port_ptr, port_len) {
    var name = UTF8ArrayToString(HEAPU8, name_ptr, name_len);
    var port = UTF8ArrayToString(HEAPU8, port_ptr, port_len);
    var destination = { host: name, port: port };
    Module['tfPendingRelayDestination'] = destination;
    if (Module['tfRelay'] && Module['tfRelay'].resolve) {
      Module['tfRelay'].resolve(destination);
    }
    return 0;
  },

  tf_wasm_get_startup_script: function(ptr, len) {
    var relay = Module['tfRelay'];
    var worlds = relay && relay.worlds;
    if (typeof worlds == 'function') {
      worlds = worlds();
    }
    if (!worlds || !worlds.length || len <= 0) {
      return 0;
    }

    var quote = function(value) {
      return String(value || '').replace(/\\/g, '\\\\').replace(/"/g, '\\"');
    };
    var script = '';
    for (var i = 0; i < worlds.length; i++) {
      var world = worlds[i];
      if (!world || !world.name || !world.host || !world.port) {
        continue;
      }
      var type = world.type || 'telnet';
      script += '/if (!world_exists("' + quote(world.name) + '")) ';
      script += '/addworld -T"' + quote(type) + '" ';
      script += quote(world.name) + ' ' + quote(world.host) + ' ' +
        quote(world.port) + '%; /endif\n';
    }
    if (!script) {
      return 0;
    }
    var needed = lengthBytesUTF8(script);
    stringToUTF8(script, ptr, len);
    return Math.min(needed, len - 1);
  }
});
