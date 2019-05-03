#Archivo de configuracion de la utilidad make.
#Author: Erwin Meza <emezav@gmail.com>
#/** @verbatim */

SRCDIR = src
INCLUDEDIR = include
BUILDDIR = build


#KERNEL_OBJS = src/start.o $(patsubst %.c,%.o,$(wildcard src/*.c))
KERNEL_OBJS = $(SRCDIR)/start.o $(patsubst %.c,%.o,$(wildcard $(SRCDIR)/*.c))


DOCFILES = $(wildcard dox/*.dox)

GCC=gcc
LD=ld

#Detectar si se requiere usar un compilador cruzado
arch := $(shell uname -s)

ARCH :=
ifneq "$(arch)" "Linux"
	ARCH := i386-elf-
endif

all: kernel
	@sh util/check_kernel_parameters.sh $(BUILDDIR)/kernel
	
kernel: $(KERNEL_OBJS)
	$(ARCH)$(LD) -m elf_i386 -T link_kernel.ld -o $(BUILDDIR)/kernel $(KERNEL_OBJS)

.S.o:
	$(ARCH)$(GCC) -m32 -ffreestanding -nostdinc -nostdlib -fno-builtin -fno-pie -c -Iinclude -o $@ $<
	
.c.o:
	$(ARCH)$(GCC) -m32 -ffreestanding -nostdinc -nostdlib -fno-builtin -fno-pie -c -Iinclude  -o $@ $<

clean:
	@rm -f $(BUILDDIR)/kernel $(KERNEL_OBJS)
	
#/** @endverbatim */
