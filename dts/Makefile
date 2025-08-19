obj-m = hello.o
KVERSION = $(shell uname -r)

build:
	dtc -@ -I dts -O dtb -o hello-spi.dtbo hello-spi-overlay.dts

all: build

install: build
	cp hello-spi.dtbo /boot/overlays
clean:
	rm *.dtbo
