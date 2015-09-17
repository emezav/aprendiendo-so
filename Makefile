#Archivo de configuracion de la utilidad make.
#Author: Erwin Meza <emezav@gmail.com>

DOXYGEN=doxygen

all:
	$(DOXYGEN) dox/Doxyfile

clean:
	-@if test -d docs; then \
	rm -r -f docs; \
	else true; fi
