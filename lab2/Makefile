PROGRAM=netninny

CFLAGS=-Wall -Wextra -pedantic -Iinc -DDEBUG_MODE
LDFLAGS=-lpthread

all: ${PROGRAM}

SRC=src/proxy.c \
	src/filtering_common.c \
	src/url_filtering.c \
    src/content_filtering.c

INC=inc/log.h \
	inc/definitions.h \
	inc/filtering_common.h\
	inc/url_filtering.h \
	inc/content_filtering.h

${PROGRAM}: ${INC} ${SRC}
	gcc ${CFLAGS} ${SRC} ${LDFLAGS} -o ${PROGRAM}

clean:
	rm -rf ${PROGRAM}
