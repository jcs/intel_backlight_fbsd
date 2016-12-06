SRC=intel_reg_map.c intel_mmio.c intel_backlight.c intel_drm.c intel_pci.c

all: intel_backlight

intel_backlight: $(SRC)
	${CC} -o intel_backlight -I/usr/local/include -I/usr/X11R6/include \
		-I/usr/X11R6/include/libdrm -I/usr/include/dev/pci/drm \
		-I/usr/local/include/libdrm -L/usr/local/lib \
		-L/usr/X11R6/lib -lpciaccess $(SRC)
	strip intel_backlight

install: intel_backlight
	install -m4555 intel_backlight /usr/local/bin

clean:
	rm -f intel_backlight
