# Makefile for sysalerter

all: 
	@echo Valid targets are: linux linux64 solaris solaris64 freebsd freebsd64 linux-mindep

dbinit:
	@perl createdbhdr.pl

sqlite-gcc:	dbinit
	OLDPWD=`pwd`; \
	cd misc ; \
	make -f Makefile.gcc ; \
	cd ${OLDPWD} 

sqlite-suncc:	dbinit
	OLDPWD=`pwd`; \
	@cd misc; \
	make -f Makefile.suncc; \
	cd ${OLDPWD}

linux: dbinit
	@${MAKE} -f Makefile.linux-gcc

linux64:	sqlite-gcc
	@${MAKE} -f Makefile.linux64-gcc-nodynamic

solaris: dbinit
	@${MAKE} -f Makefile.solaris-suncc

solaris64:	dbinit sqlite-suncc
	@${MAKE} -f Makefile.solaris64-suncc

freebsd: dbinit
	@${MAKE} -f Makefile.FreeBSD

freebsd64: dbinit sqlite-gcc
	@${MAKE} -f Makefile.FreeBSD64

linux-mindep: dbinit
	@${MAKE} -f Makefile.linux32-gcc-nodynamic

clean:
	rm -f *.o *.core misc/*.o dbinit.h
