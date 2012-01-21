
default: wii-release

all: debug release

debug: wii-debug

release: wii-release

wii-debug:
	$(MAKE) -C source  PLATFORM=wii BUILD=wii_debug

wii-release:
	$(MAKE) -C source  PLATFORM=wii BUILD=wii_release

clean: 
	$(MAKE) -C source clean

install: wii-release
	$(MAKE) -C source install

run: install
	$(MAKE) -C example
	$(MAKE) -C example run

