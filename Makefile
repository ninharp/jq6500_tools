# jq6500flash - JQ6500 Flash Tool Makefile

VERSION = 0.1

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# includes and libs
INCS = -I. -I/usr/include
LIBS = -L/usr/lib -lc

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\" -D_GNU_SOURCE
CFLAGS = -std=c99 -pedantic -Wall -Os ${INCS} ${CPPFLAGS}
LDFLAGS = -s ${LIBS}

# compiler and linker
CC = cc

SRC = jq6500flash.c jq6500fs.c
OBJ = ${SRC:.c=.o}

all: options jq6500flash

options:
	@echo jq6500flash build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

jq6500flash: ${OBJ}
	@echo CC -o $@ $(SRC) $(LDFLAGS)
	@echo

	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f jq6500flash ${OBJ} jq6500flash-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p jq6500-${VERSION}
	@cp -R LICENSE Makefile README.md jq6500flash.c jq6500fs.c jq6500fs.h jq6500flash-${VERSION}
	@tar -cf jq6500flash-${VERSION}.tar jq6500flash-${VERSION}
	@gzip jq6500flash-${VERSION}.tar
	@rm -rf jq6500flash-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f jq6500flash ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/jq6500flash

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/jq6500flash

.PHONY: all options clean dist install uninstall
