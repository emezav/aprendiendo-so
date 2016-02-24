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

all: $(KERNEL_OBJS)
	@$(ARCH)$(LD) -T link_kernel.ld -o $(BUILDDIR)/kernel $(KERNEL_OBJS)

%.o: %.S $(KERNEL_SOURCES) $(KERNEL_INCLUDES)
	@$(ARCH)$(GCC) -nostdinc -nostdlib -fno-builtin -c -I$(INCLUDEDIR) -o $@ $<

clean:
	@rm -f $(BUILDDIR)/kernel $(KERNEL_OBJS)

#/** @endverbatim */
