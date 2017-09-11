PROGRAM=proxy

CFLAGS=-Wall -Wextra -Wpedantic -Iinc -DDEBUG_MODE
LDFLAGS=-lpthread

all: ${PROGRAM}

${PROGRAM}: src/proxy.c inc/definitions.h
	gcc ${CFLAGS} src/proxy.c ${LDFLAGS} -o ${PROGRAM}

clean:
	rm -rf ${PROGRAM}
