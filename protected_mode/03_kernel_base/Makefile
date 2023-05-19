#Archivo de configuracion de la utilidad make.
#Author: Erwin Meza <emezav@gmail.com>
#/** @verbatim */

KERNEL_DIR = kernel

DOCFILES = $(wildcard dox/*.dox)

GCC=gcc
LD=ld
JAVA=java
DOXYGEN=doxygen

#Detectar si se requiere usar un compilador cruzado
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
	@cd $(KERNEL_DIR);make
	-@if test ! -f build/disk_template.gz; then \
	   cp -f ../disk_templates/5m_disk_template.gz build/disk_template.gz; \
	   else true; fi
	-@if test ! -f build/disk_template -a -f build/disk_template.gz; then \
	   gunzip build/disk_template.gz; \
	   else true; fi
	@#Copiar el kernel al directorio filesys/kernel
	@cp -f $(KERNEL_DIR)/build/kernel build/filesys/boot/kernel
	@#Copiar la plantilla a la imagen de disco
	@cp -f build/disk_template build/disk_image
	@#Extraer la particion inicial
	@dd if=build/disk_template of=build/first_partition bs=512 skip=63
	@#Copiar el kernel al sistema de archivos
	@e2fsimage -f build/first_partition -d build/filesys -n
	@#Copiar la particion actualizada a la imagen de disco
	@dd if=build/first_partition of=build/disk_image bs=512 seek=63
	@#Borrar la particion actualizada
	@rm -f build/first_partition
	

bochs: all
	-bochs -q 'boot:disk' \
	'ata0-master: type=disk, path="build/disk_image", cylinders=10, heads=16, spt=63'\
	'megs:32'
	

bochs-cd: iso
	-bochs -q 'boot:cdrom' \
	'ata0-master: type=cdrom, path="build/cd_image.iso", status=inserted'\
  'megs: 32'

bochsdbg: all
	-$(BOCHSDBG) -q 'boot:disk' \
	'ata0-master: type=disk, path="build/disk_image", cylinders=10, heads=16, spt=63'\
	'megs:32' 'display_library:$(BOCHSDISPLAY), options="gui_debug"'
	
bochsdbg-cd: iso
	-$(BOCHSDBG) -q 'boot:cdrom' \
	'ata0-master: type=cdrom, path="build/cd_image.iso", status=inserted'\
  'megs:32' 'display_library:$(BOCHSDISPLAY), options="gui_debug"'


qemu: all
	qemu -hda build/disk_image -boot c
	
qemu-cd: iso
	qemu -boot d -cdrom build/cd_image.iso

iso: all
	mkisofs -J -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o build/cd_image.iso build/filesys

vbox: all
	-VBoxManage unregistervm  "Aprendiendo_SO" --delete
	VBoxManage createvm --name="Aprendiendo_SO" --basefolder="./build" --default --register
	VBoxManage modifyvm "Aprendiendo_SO"  --memory=128 --acpi=on --ioapic=on --cpus=2 --pae=on --long-mode=on --firmware=bios --vram 128
	@-rm -f build/disk_image.vdi
	vboxmanage convertfromraw build/disk_image build/disk_image.vdi --format=vdi
	VBoxManage storagectl "Aprendiendo_SO" --name "SATA Controller" --add sata --controller IntelAHCI
	VBoxManage storageattach "Aprendiendo_SO" --storagectl "SATA Controller" --port 0 --device 0 --type hdd --medium build/disk_image.vdi
	VBoxManage startvm "Aprendiendo_SO" --type=gui

docs: $(DOCFILES)
	$(DOXYGEN) dox/Doxyfile

clean:
	@cd kernel;make clean
	@-rm -f build/kernel $(KERNEL_OBJS) build/disk_image build/disk_image.vdi build/cd_image.iso build/filesys/boot/kernel build/disk_template build/disk_template.gz
	@-rm -rf docs
	@-VBoxManage unregistervm  "Aprendiendo_SO" --delete

#/** @endverbatim */


