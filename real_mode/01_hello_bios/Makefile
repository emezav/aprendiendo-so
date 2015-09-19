#Archivo de configuracion de la utilidad make.
#Author: Erwin Meza Vega
#/** @verbatim */

BOOTSECTOR_OBJS= bootsect/src/bootsect.o

DOCFILES = $(wildcard dox/*.dox) $(wildcard dox/*.md)

INCLUDEDIR = bootsect/include

DOXYGEN=doxygen
GCC=gcc
LD=ld

#Detectar si se requiere usar un compilador cruzado
arch := $(shell uname -s)
machine := $(shell uname -m)
x86found := false
os := $(shell uname -o)

ARCH :=
ifeq "$(arch)" "Linux"
	ifeq "$(machine)" "i386"
		x86found := true
	endif	
	ifeq "$(machine)" "i486"
		x86found := true
	endif	
	ifeq "$(machine)" "i586"
		x86found := true
	endif	
	ifeq "$(machine)" "i686"
		x86found := true
	endif	
else
	ARCH := i386-elf-
endif

ifeq "$(x86found)" "false"
	ARCH := i386-elf-
endif

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

all: bootsector
	cp bootsect/build/bootsect build/floppy.img

bootsector: $(BOOTSECTOR_OBJS)
	$(ARCH)$(LD) -T bootsect/build/link_bootsector.ld -o bootsect/build/bootsect $(BOOTSECTOR_OBJS)
	
.S.o:
	$(ARCH)$(GCC) -nostdinc -nostdlib -fno-builtin -c -I$(INCLUDEDIR) -o $@ $<

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
	rm -f bootsect/build/bootsect $(BOOTSECTOR_OBJS) build/floppy.img
	-if test -d docs; then \
	   rm -r -f docs; \
	   else true; fi

#/** @endverbatim */
