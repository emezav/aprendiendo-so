#Archivo de configuracion de la utilidad make.
#Author: Erwin Meza Vega
#/** @verbatim */

BOOTSECTOR_DIR = bootsect
KERNEL_DIR = kernel

DOXYGEN=doxygen
DOCFILES = $(wildcard dox/*.dox) $(wildcard dox/*.md)

#Detectar el tipo de sistema
arch := $(shell uname -s)
machine := $(shell uname -m)
x86found := false
os := $(shell uname -o)

BOCHSDBG := bochsdbg

BOCHSDISPLAY := x
ifeq "$(os)" "Msys"
	BOCHSDISPLAY := win32
endif

ifeq "$(os)" "Cygwin"
	BOCHSDISPLAY := win32
endif

all: 
	@cd $(BOOTSECTOR_DIR);make
	@cd $(KERNEL_DIR);make
	@cat $(BOOTSECTOR_DIR)/build/bootsect $(KERNEL_DIR)/build/kernel > build/floppy.img

bochs: all
	-bochs -q 'boot:a' \
	'floppya: 1_44=build/floppy.img, status=inserted' 'megs:32'
	
bochsdbg: all
	-$(BOCHSDBG) -q 'boot:a' \
	'floppya: 1_44=build/floppy.img, status=inserted' 'megs:32' \
	'display_library:$(BOCHSDISPLAY), options="gui_debug"'

qemu: all
	qemu -fda build/floppy.img -boot a

vbox: all
	-VBoxManage unregistervm  "Aprendiendo_SO" --delete
	VBoxManage createvm --name="Aprendiendo_SO" --basefolder="./build" --default --register
	VBoxManage modifyvm "Aprendiendo_SO"  --memory=128 --cpus=1 --firmware=bios --vram 128
	VBoxManage storagectl "Aprendiendo_SO" --name "Floppy" --add floppy --controller I82078
	VBoxManage storageattach "Aprendiendo_SO" --storagectl "Floppy" --port 0 --device 0 --type fdd --medium build/floppy.img
	VBoxManage startvm "Aprendiendo_SO" --type=gui

docs: $(DOCFILES)
	$(DOXYGEN) dox/Doxyfile

clean:
	@cd $(BOOTSECTOR_DIR);make clean
	@cd $(KERNEL_DIR);make clean
	@rm -f build/floppy.img
	@-rm -rf docs
	@-VBoxManage unregistervm  "Aprendiendo_SO" --delete


#/** @endverbatim */
