CMAKE ?= cmake
BUILD_DIR ?= build
BUILD_TYPE ?= Release
PREFIX ?=
CMAKE_ARGS ?=
BUILD_ARGS ?=
ACT ?= act
ACT_RUNNER_IMAGE ?= catthehacker/ubuntu:act-latest
ACT_ARGS ?=

CONFIGURE_ARGS = -S . -B "$(BUILD_DIR)" -DCMAKE_BUILD_TYPE="$(BUILD_TYPE)"
ifneq ($(strip $(PREFIX)),)
CONFIGURE_ARGS += -DCMAKE_INSTALL_PREFIX="$(PREFIX)"
endif

.PHONY: all configure test ci-local install clean

all: configure
	$(CMAKE) --build "$(BUILD_DIR)" $(BUILD_ARGS)

configure:
	$(CMAKE) $(CONFIGURE_ARGS) $(CMAKE_ARGS)

test:
	$(CMAKE) $(CONFIGURE_ARGS) -DBUILD_TESTING=ON $(CMAKE_ARGS)
	$(CMAKE) --build "$(BUILD_DIR)" $(BUILD_ARGS)
	ctest --test-dir "$(BUILD_DIR)" --output-on-failure

ci-local:
	$(ACT) push -W .github/workflows/ci.yml -j linux \
		-P ubuntu-latest=$(ACT_RUNNER_IMAGE) $(ACT_ARGS)

install: all
	$(CMAKE) --install "$(BUILD_DIR)"

clean:
	$(CMAKE) -E remove_directory "$(BUILD_DIR)"
