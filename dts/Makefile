obj-m = hello.o
KVERSION = $(shell uname -r)

INC := $(shell find /usr /lib /usr/src -type d -path '*/include' -exec test -e '{}/dt-bindings/gpio/gpio.h' \; -print 2>/dev/null | head -n1)

DT := hello-spi-overlay.dts
DTBO := hello-spi.dtbo

$(DTBO): $(DT)
	cpp -nostdinc -I "$(INC)" -undef -x assembler-with-cpp $< | \
	dtc -@ -I dts -O dtb -o hello-spi.dtbo -

#dtc -@ -I dts -O dtb -o hello-spi.dtbo hello-spi-overlay.dts

all: build

install: build
	cp hello-spi.dtbo /boot/overlays
clean:
	rm *.dtbo
