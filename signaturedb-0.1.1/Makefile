#binary files
OBJ=sdbscan
#NB source files
SRC= 
# DOCUMENTATION
DOCS= Makefile
#Other files that need to be checked eg configs
LOCATION=/usr/local
VERSION=0.1.0
CC=/usr/bin/gcc
#CFLAGS= -Wall -DDEBUG
CFLAGS= -Wall -O3
PROFILE=
CFLAGS += ${PROFILE}
SDBDIR=/usr/local/signaturedb

default: sdbscan

.c.o:
	${CC} ${CFLAGS} -c $*.c
	
all: ${OBJ} 


sdbscan: sdbscan.c ringsearch.o
	${CC} ${CFLAGS} sdbscan.c ringsearch.o -o sdbscan

	
clean:
	rm -rf *.o *core ${OBJ}

install: sdbscan
	mkdir -p ${SDBDIR}
	cp sdbscan ${SDBDIR}
	cp signaturedb.db ${SDBDIR}
	cp signaturedb.conf ${SDBDIR}
	ln -s ${SDBDIR}/sdbscan /usr/local/bin
