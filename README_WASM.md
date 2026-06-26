TinyFugue WebAssembly Build
===========================

TinyFugue can be compiled with Emscripten.  This repository keeps the core
WebAssembly support: the C platform hooks, the CMake option, the Emscripten
JS library imports, the artifact packaging target, and a Node smoke test for
the compiled module.

Build
-----

Install and activate Emscripten.  Build and install PCRE2 with Emscripten:

    git clone https://github.com/PCRE2Project/pcre2.git /tmp/pcre2
    emcmake cmake -S /tmp/pcre2 -B /tmp/pcre2-build \
        -DCMAKE_INSTALL_PREFIX=/tmp/pcre2-wasm \
        -DPCRE2_BUILD_PCRE2_8=ON \
        -DPCRE2_BUILD_PCRE2_16=OFF \
        -DPCRE2_BUILD_PCRE2_32=OFF \
        -DPCRE2_BUILD_TESTS=OFF \
        -DPCRE2_BUILD_PCRE2GREP=OFF \
        -DPCRE2_BUILD_PCRE2TEST=OFF \
        -DPCRE2_SUPPORT_JIT=OFF \
        -DPCRE2_SUPPORT_UNICODE=OFF
    cmake --build /tmp/pcre2-build
    cmake --install /tmp/pcre2-build

Configure and build TinyFugue:

    emcmake cmake -S . -B build-wasm -DTF_WASM=ON \
        -DCMAKE_PREFIX_PATH=/tmp/pcre2-wasm
    cmake --build build-wasm
    cmake --build build-wasm --target wasm-dist
    ctest --test-dir build-wasm --output-on-failure

The expected Emscripten outputs are `tf.js` and `tf.wasm`.
The `wasm-dist` target also writes `build-wasm/tinyfugue-wasm.tar.gz`, which
contains:

```text
tinyfugue-wasm/
  tf.js
  tf.wasm
  tf-lib/
```

Core Contract
-------------

The core module exports:

* `_main`
* `_tf_tick`
* `_tf_next_deadline_ms`
* `_tf_wasm_resize`
* `_tf_wasm_smoke`

The Emscripten JS library at `wasm/tf_jslib.js` supplies hooks for:

* stdin and stdout bytes
* scheduler wakeups
* relay-backed socket operations
* relay-provided startup script generation

The Node smoke test in `tests/wasm_smoke.mjs` verifies the compiled module can
start, read input, write output, load `tf-lib`, provision relay worlds, and
exercise relay-backed socket hooks.
