#Archivo de configuracion de la utilidad make.
#Author: Erwin Meza Vega
#/** @verbatim */

SRCDIR = src
INCLUDEDIR = include
BUILDDIR = build

BOOTSECTOR_OBJS = $(SRCDIR)/bootsect.o

BOOTSECTOR_SOURCES = $(SRCDIR)/bootsect.S

BOOTSECTOR_INCLUDES = $(INCLUDEDIR)/bootsect.h

GCC=gcc
LD=ld

#Detectar el tipo de sistema
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

all: $(BOOTSECTOR_OBJS)
	@$(ARCH)$(LD) -T link_bootsector.ld -o $(BUILDDIR)/bootsect $(BOOTSECTOR_OBJS)

%.o: %.S $(BOOTSECTOR_SOURCES) $(BOOTSECTOR_INCLUDES)
	@$(ARCH)$(GCC) -nostdinc -nostdlib -fno-builtin -c -I$(INCLUDEDIR) -o $@ $<

clean:
	@rm -f $(BUILDDIR)/bootsect $(BOOTSECTOR_OBJS)

#/** @endverbatim */
