CMAKE ?= cmake

GENERATOR ?= $(shell (which ninja > /dev/null 2> /dev/null && echo Ninja) || \
						 echo 'Unix Makefiles')
prefix ?= 
PREFIX ?= $(prefix)

ifeq ($(GENRATOR),Ninja)
	BUILDFILE = build.ninja
else
	BUILDFILE = Makefile
endif

all: .begin build/tf

.PHONY: .begin
.begin:
	@which $(CMAKE) > /dev/null 2> /dev/null || \
		(echo 'Please install CMake and then re-run the "make" command!' 1>&2 && false)

.PHONY: build/tf
build/tf: build/$(BUILDFILE)
	$(CMAKE) --build build

build/$(BUILDFILE): | build
	cd build; $(CMAKE) .. -G "$(GENERATOR)" \
		$(if $(PREFIX),-DCMAKE_INSTALL_PREFIX="$(PREFIX)",) -DCMAKE_EXPORT_COMPILE_COMMANDS=1

build:
	mkdir -p build

.PHONY: clean
clean:
	rm -rf build

.PHONY: install
install: build/tf
	$(CMAKE) --build build --target install
