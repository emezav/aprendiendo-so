SRCDIR = src
INCLUDEDIR = include
BUILDDIR = build

GCC = gcc
LD = ld

arch := $(shell uname -s)
ARCH :=
ifneq "$(arch)" "Linux"
	ARCH := i386-elf-
endif

USER_OBJS = \
	$(BUILDDIR)/crt0.o \
	$(BUILDDIR)/stdio.o \
	$(BUILDDIR)/stdlib.o \
	$(BUILDDIR)/syscall.o \
	$(BUILDDIR)/string.o

all: $(BUILDDIR)/hello.elf $(BUILDDIR)/shell.elf

$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

$(BUILDDIR)/crt0.o: $(SRCDIR)/crt0.S | $(BUILDDIR)
	$(ARCH)$(GCC) -m32 -ffreestanding -nostdlib -c -o $@ $<

$(BUILDDIR)/stdlib.o: $(SRCDIR)/stdlib.c | $(BUILDDIR)
	$(ARCH)$(GCC) -m32 -ffreestanding -nostdlib -fno-builtin -I$(INCLUDEDIR) -c -o $@ $<

$(BUILDDIR)/stdio.o: $(SRCDIR)/stdio.c | $(BUILDDIR)
	$(ARCH)$(GCC) -m32 -ffreestanding -nostdlib -fno-builtin -I$(INCLUDEDIR) -c -o $@ $<

$(BUILDDIR)/syscall.o: $(SRCDIR)/syscall.c | $(BUILDDIR)
	$(ARCH)$(GCC) -m32 -ffreestanding -nostdlib -fno-builtin -I$(INCLUDEDIR) -c -o $@ $<

$(BUILDDIR)/string.o: $(SRCDIR)/string.c | $(BUILDDIR)
	$(ARCH)$(GCC) -m32 -ffreestanding -nostdlib -fno-builtin -I$(INCLUDEDIR) -c -o $@ $<

$(BUILDDIR)/hello.o: $(SRCDIR)/hello.c | $(BUILDDIR)
	$(ARCH)$(GCC) -m32 -ffreestanding -nostdlib -fno-builtin -I$(INCLUDEDIR) -c -o $@ $<

$(BUILDDIR)/shell.o: $(SRCDIR)/shell.c | $(BUILDDIR)
	$(ARCH)$(GCC) -m32 -ffreestanding -nostdlib -fno-builtin -I$(INCLUDEDIR) -c -o $@ $<

$(BUILDDIR)/hello.elf: $(USER_OBJS) $(BUILDDIR)/hello.o link.ld
	$(ARCH)$(LD) -m elf_i386 -T link.ld -o $@ $(USER_OBJS) $(BUILDDIR)/hello.o

$(BUILDDIR)/shell.elf: $(USER_OBJS) $(BUILDDIR)/shell.o link.ld
	$(ARCH)$(LD) -m elf_i386 -T link.ld -o $@ $(USER_OBJS) $(BUILDDIR)/shell.o

clean:
	-@rm -f $(BUILDDIR)/crt0.o $(BUILDDIR)/stdio.o $(BUILDDIR)/stdlib.o $(BUILDDIR)/syscall.o $(BUILDDIR)/string.o $(BUILDDIR)/hello.o $(BUILDDIR)/shell.o $(BUILDDIR)/hello.elf $(BUILDDIR)/shell.elf
