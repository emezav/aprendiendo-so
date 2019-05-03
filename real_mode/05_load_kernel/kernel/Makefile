#Archivo de configuracion de la utilidad make.
#Author: Erwin Meza Vega
#/** @verbatim */

SRCDIR = src
INCLUDEDIR = include
BUILDDIR = build

KERNEL_OBJS = $(patsubst %.S,%.o,$(wildcard $(SRCDIR)/*.S)) \
			$(patsubst %.c,%.o,$(wildcard $(SRCDIR)/*.c)) \

KERNEL_SOURCES = $(wildcard $(SRCDIR)/*.S)

KERNEL_INCLUDES = $(wildcard $(INCLUDEDIR)/*.h)

GCC=gcc
LD=ld

#Detectar el tipo de sistema
arch := $(shell uname -s)
machine := $(shell uname -m)
x86found := false
os := $(shell uname -o)

ARCH :=
ifneq "$(arch)" "Linux"
	ARCH := i386-elf-
endif

all: $(KERNEL_OBJS)
	@$(ARCH)$(LD) -m elf_i386 -T link_kernel.ld -o $(BUILDDIR)/kernel $(KERNEL_OBJS)

%.o: %.S $(KERNEL_SOURCES) $(KERNEL_INCLUDES)
	@$(ARCH)$(GCC) -m32 -nostdinc -nostdlib -fno-builtin -c -I$(INCLUDEDIR) -o $@ $<

clean:
	@rm -f $(BUILDDIR)/kernel $(KERNEL_OBJS)

#/** @endverbatim */
