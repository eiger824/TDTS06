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

SRC=src/proxy.c \
	src/filtering_common.c \
	src/url_filtering.c \
    src/content_filtering.c

INC=inc/log.h \
	inc/definitions.h \
	inc/filtering_common.h\
	inc/url_filtering.h \
	inc/content_filtering.h

OBJ=${OBJDIR}/proxy.o \
	${OBJDIR}/filtering_common.o \
	${OBJDIR}/url_filtering.o \
	${OBJDIR}/content_filtering.o


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
				   inc/log.h \
				   inc/url_filtering.h \
				   inc/content_filtering.h \
				   inc/filtering_common.h
	gcc ${CFLAGS} src/proxy.c -o ${OBJDIR}/proxy.o

${OBJDIR}/filtering_common.o: src/filtering_common.c \
							  inc/filtering_common.h
	gcc ${CFLAGS} src/filtering_common.c -o ${OBJDIR}/filtering_common.o

${OBJDIR}/url_filtering.o: src/url_filtering.c \
						   inc/url_filtering.h \
						   inc/filtering_common.h
	gcc ${CFLAGS} src/url_filtering.c -o ${OBJDIR}/url_filtering.o

${OBJDIR}/content_filtering.o: src/content_filtering.c \
							   inc/content_filtering.h \
							   inc/filtering_common.h
	gcc ${CFLAGS} src/content_filtering.c -o ${OBJDIR}/content_filtering.o

clean:
	rm -rf ${PROGRAM} ${BINDIR} ${OBJDIR} *~ inc/*~ src/*~
