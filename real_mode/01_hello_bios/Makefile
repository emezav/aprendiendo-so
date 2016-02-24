#Archivo de configuracion de la utilidad make.
#Author: Erwin Meza Vega
#/** @verbatim */

BOOTSECTOR_DIR = bootsect

DOXYGEN=doxygen
DOCFILES = $(wildcard dox/*.dox) $(wildcard dox/*.md)

#Detectar el tipo de sistema
arch := $(shell uname -s)
machine := $(shell uname -m)
x86found := false
os := $(shell uname -o)

BOCHSDBG := bochsdbg
ifeq "$(arch)" "Linux"
    BOCHSDBG := bochs
endif

BOCHSDISPLAY := x
ifeq "$(os)" "Msys"
	BOCHSDISPLAY := win32
endif

ifeq "$(os)" "Cygwin"
	BOCHSDISPLAY := win32
endif

all: 
	@cd $(BOOTSECTOR_DIR);make
	@cp -f $(BOOTSECTOR_DIR)/build/bootsect build/floppy.img

bochs: all
	-bochs -q 'boot:a' \
	'floppya: 1_44=build/floppy.img, status=inserted' 'megs:32'
	
bochsdbg: all
	-$(BOCHSDBG) -q 'boot:a' \
	'floppya: 1_44=build/floppy.img, status=inserted' 'megs:32' \
	'display_library:$(BOCHSDISPLAY), options="gui_debug"'
	
qemu: all
	qemu -fda build/floppy.img -boot a

docs: $(DOCFILES)
	$(DOXYGEN) dox/Doxyfile

clean:
	@cd $(BOOTSECTOR_DIR);make clean
	@rm -f build/floppy.img
	@-if test -d docs; then \
	   rm -r -f docs; \
	   else true; fi

#/** @endverbatim */
