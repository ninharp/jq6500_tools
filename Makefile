# jq6500 tools - JQ6500 Toolset Makefile

VERSION = 0.11

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

SRC_FLASH = jq6500flash.c jq6500fs.c
OBJ_FLASH = ${SRC_FLASH:.c=.o}

SRC_CMD = jq6500cmd.c
OBJ_CMD = ${SRC_CMD:.c=.o}

all: options jq6500flash jq6500cmd

options:
	@echo jq6500 tools build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"
	@echo

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

jq6500flash: ${OBJ_FLASH}
	@echo CC -o $@ $(SRC_FLASH) $(LDFLAGS)
	@echo

	@${CC} -o $@ ${OBJ_FLASH} ${LDFLAGS}

jq6500cmd: ${OBJ_CMD}
	@echo CC -o $@ $(SRC_CMD) $(LDFLAGS)
	@echo

	@${CC} -o $@ ${OBJ_CMD} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f jq6500flash ${OBJ_FLASH} jq6500cmd ${OBJ_CMD} jq6500flash-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p jq6500-${VERSION}
	@cp -R LICENSE Makefile README.md jq6500flash.c jq6500fs.c jq6500fs.h jq6500cmd.c jq6500flash-${VERSION}
	@tar -cf jq6500flash-${VERSION}.tar jq6500flash-${VERSION}
	@gzip jq6500flash-${VERSION}.tar
	@rm -rf jq6500flash-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f jq6500flash ${DESTDIR}${PREFIX}/bin
	@cp -f jq6500cmd ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/jq6500flash

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/jq6500flash
	@rm -f ${DESTDIR}${PREFIX}/bin/jq6500cmd

.PHONY: all options clean dist install uninstall
