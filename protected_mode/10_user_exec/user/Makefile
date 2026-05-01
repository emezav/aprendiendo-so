SRCDIR = src
BUILDDIR = build

GCC = gcc
LD = ld

arch := $(shell uname -s)
ARCH :=
ifneq "$(arch)" "Linux"
	ARCH := i386-elf-
endif

all: $(BUILDDIR)/hello.elf

$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

$(BUILDDIR)/hello.o: $(SRCDIR)/hello.S | $(BUILDDIR)
	$(ARCH)$(GCC) -m32 -ffreestanding -nostdlib -c -o $@ $<

$(BUILDDIR)/hello.elf: $(BUILDDIR)/hello.o link.ld
	$(ARCH)$(LD) -m elf_i386 -T link.ld -o $@ $(BUILDDIR)/hello.o

clean:
	-@rm -f $(BUILDDIR)/hello.o $(BUILDDIR)/hello.elf
