#Archivo de configuracion de la utilidad make.
#Author: Erwin Meza <emezav@gmail.com>

DOXYGEN=doxygen
BASH=bash

HELP_OBJS = README.md $(wildcard dox/*.md) $(wildcard dox/*.dox)

#Generar documentación automáticamente
all: docs

docs: $(HELP_OBJS)
	$(DOXYGEN) dox/Doxyfile

clean:
	-@if test -d docs; then \
	rm -r -f docs; \
	else true; fi

all-clean:
	$(BASH) -c "cd real_mode; ./all_projects clean"
	$(BASH) -c "cd real_mode; ./all_projects clean"
