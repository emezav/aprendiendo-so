#Archivo de configuracion de la utilidad make.
#Author: Erwin Meza Vega
#/** @verbatim */

SRCDIR = src
INCLUDEDIR = include
BUILDDIR = build

BOOTSECTOR_OBJS = $(SRCDIR)/bootsect.o

BOOTSECTOR_SOURCES = $(SRCDIR)/bootsect.S

GCC=gcc
LD=ld

#Detectar el tipo de sistema
arch := $(shell uname -s)
os := $(shell uname -o)

ARCH :=
ifneq "$(arch)" "Linux"
	ARCH := i386-elf-
endif

all: $(BOOTSECTOR_OBJS)
	$(ARCH)$(LD) -m elf_i386 -T link_bootsector.ld -o $(BUILDDIR)/bootsect $(BOOTSECTOR_OBJS)

%.o: %.S $(BOOTSECTOR_SOURCES)
	$(ARCH)$(GCC) -m32 -nostdinc -nostdlib -fno-builtin -c -I$(INCLUDEDIR) -o $@ $<

clean:
	@rm -f $(BUILDDIR)/bootsect $(BOOTSECTOR_OBJS)

#/** @endverbatim */
