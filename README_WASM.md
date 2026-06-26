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

Unicode support can use either ICU or utf8proc.  The default
`TF_UNICODE_BACKEND=AUTO` prefers ICU when the Emscripten ICU data file is
available, then falls back to utf8proc when a WebAssembly build of utf8proc is
found.  Select a backend explicitly with `TF_UNICODE_BACKEND=ICU` or
`TF_UNICODE_BACKEND=UTF8PROC`.

ICU gives full charset conversion support, but Emscripten embeds the ICU data
file into the output.  That makes the `.wasm` much larger.  utf8proc is UTF-8
only, but is much smaller and is the preferred browser build when UTF-8-only
world encodings are acceptable.

To use utf8proc, build and install utf8proc with Emscripten into a prefix that
CMake can search, for example the same prefix used for PCRE2:

    git clone https://github.com/JuliaStrings/utf8proc.git /tmp/utf8proc
    emcmake cmake -S /tmp/utf8proc -B /tmp/utf8proc-build \
        -DCMAKE_INSTALL_PREFIX=/tmp/pcre2-wasm \
        -DBUILD_SHARED_LIBS=OFF
    cmake --build /tmp/utf8proc-build
    cmake --install /tmp/utf8proc-build

Configure and build TinyFugue:

    emcmake cmake -S . -B build-wasm -DTF_WASM=ON \
        -DCMAKE_PREFIX_PATH=/tmp/pcre2-wasm \
        -DTF_UNICODE_BACKEND=UTF8PROC
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

Unicode Backend Size
--------------------

The exact artifact size depends on the Emscripten version, optimization level,
and whether the files are served compressed.  Local builds with the default
WASM link options produced these approximate `tf.wasm` sizes:

```text
Backend   Uncompressed .wasm   gzip .wasm   Notes
ICU       30.7 MB              12.2 MB      embeds the Emscripten ICU data file
utf8proc  1.6 MB               0.6 MB       UTF-8 only
```

Use ICU when TinyFugue must convert non-UTF-8 world charsets in the browser.
Use utf8proc when the browser build can require UTF-8 and startup/download size
matters.

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
