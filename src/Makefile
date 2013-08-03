BINDIR =	/bin/
MANDIR =	/usr/local/man/man1
CC =		gcc
CFLAGS =	-O
#CFLAGS =	-g
#SYSVLIBS =	-lnsl -lsocket
LDFLAGS =	-s ${SYSVLIBS}
#LDFLAGS =	-g ${SYSVLIBS}
PRONAME	=	tmdb
EXEFILE =	tmdb
SOFILE	=	libtmdb.so
AFILE	=	libtmdb.a
HEADER	=	tmdb.h
TESTF	=	tmdb_test
OUTPUT	=	output



all:	test
	@echo ""
	@echo "==========Don't forget=========="
	@echo "Please cp ${OUTPUT} file to your use path"


buildlib: 
	${CC} ${CFLAGS} -g -c tm_util.c
builddb:
	${CC} ${CFLAGS} -g -c tmdb.c

tmdb:	buildlib builddb
	${CC} ${CFLAGS} tm_util.o tmdb.o  ${LDFLAGS} -o ${EXEFILE}

test:	buildlib builddb lib liba 
	${CC} ${CFLAGS} ${LDFLAGS} -o ${TESTF} tmdb_test.c -L. -l${PRONAME} -I.
	@mkdir output
	@mv ${SOFILE} ${AFILE} ${TESTF} ${OUTPUT}
	@cp ${HEADER} ${OUTPUT}

so:	lib
lib:	so1

so1:	buildlib builddb
	${CC} -shared ${CFLAGS} tm_util.o tmdb.o  ${LDFLAGS} -o ${SOFILE} 


a:	liba
liba:	a1

a1:	buildlib builddb
	@ar cr ${AFILE} tm_util.o tmdb.o



installall: all	 install

install:		
	@if [ ! -f "$(EXEFILE)" ]; then \
		echo "Please first execute: make\n"; \
		exit 1;\
	fi
	@if [ ! -d "$(MANDIR)" ]; then \
		mkdir "$(MANDIR)"; \
	fi
	rm -f ${BINDIR}/${EXEFILE}
	rm -f ${MANDIR}/${EXEFILE}.1
	cp ${EXEFILE} ${BINDIR}/${EXEFILE}
	cp ${EXEFILE}.1 ${MANDIR}/${EXEFILE}.1
	@echo "\nInstall ${EXEFILE} success."
	@echo "Thank you for the use of ${EXEFILE}"
	@echo "\n--------------------------------"
	@echo "[ Help ]"
	@echo " * man ${EXEFILE}"
	@echo " * ${BINDIR}/${EXEFILE} -h"
	@echo " * ${BINDIR}/${EXEFILE} --help"
	@echo "--------------------------------\n"

uninstall: 
	rm -f ${BINDIR}/${EXEFILE}
	rm -f ${MANDIR}/${EXEFILE}.1
	@echo "\n--------------------------------"
	@echo "Uninstall ${EXEFILE} success."
	@echo "\nThank you for the use of ${EXEFILE}"
	@echo "--------------------------------\n"


c:	clean
clean:
	rm -f  ${EXEFILE} *.o core core.* *.core test/* *.so *.a 
	rm -rf ${OUTPUT}

