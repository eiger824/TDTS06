BINDIR=bin
OBJDIR=obj
PROGRAM=netninny
PROGPATH=${BINDIR}/${PROGRAM}

MACRO=-DDEBUG_MODE
ifeq ($(RELEASE),1)
	MACRO=
endif

CFLAGS=-c -Wall -Wextra -pedantic -Iinc ${MACRO}
LDFLAGS=-lpthread

SRC=src/filtering_common.c \
	src/ubf.c \
	src/cbf.c \
	src/utils.c

INC=inc/log.h \
	inc/definitions.h \
	inc/filtering_common.h\
	inc/ubf.h \
	inc/cbf.h \
	inc/utils.h \
	inc/http.h

OBJ=${OBJDIR}/proxy.o \
	${OBJDIR}/filtering_common.o \
	${OBJDIR}/ubf.o \
	${OBJDIR}/cbf.o \
	${OBJDIR}/utils.o


all: ${OBJDIR} ${BINDIR} ${PROGPATH}

${OBJDIR}:
	test -d ${OBJDIR} || mkdir ${OBJDIR}

${BINDIR}:
	test -d ${BINDIR} || mkdir ${BINDIR}

${PROGPATH}: ${OBJ}
	rm -f ${PROGRAM}
	gcc ${OBJ} ${LDFLAGS} -o ${PROGPATH}
	ln -s ${PROGPATH} ${PROGRAM}

${OBJDIR}/proxy.o: src/proxy.c \
			${SRC} \
			${INC}
	gcc ${CFLAGS} src/proxy.c -o ${OBJDIR}/proxy.o

${OBJDIR}/filtering_common.o: ${SRC} \
				${INC}
	gcc ${CFLAGS} src/filtering_common.c -o ${OBJDIR}/filtering_common.o

${OBJDIR}/ubf.o: ${SRC} \
		${INC}
	gcc ${CFLAGS} src/ubf.c -o ${OBJDIR}/ubf.o

${OBJDIR}/cbf.o: ${SRC} \
		${INC}
	gcc ${CFLAGS} src/cbf.c -o ${OBJDIR}/cbf.o

${OBJDIR}/utils.o: inc/utils.h \
		src/utils.c
	gcc ${CFLAGS} src/utils.c -o ${OBJDIR}/utils.o

clean:
	rm -rf ${PROGRAM} ${BINDIR} ${OBJDIR} *~ inc/*~ src/*~


