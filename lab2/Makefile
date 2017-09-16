PROGRAM=proxy

CFLAGS=-Wall -Wextra -pedantic -Iinc -DDEBUG_MODE
LDFLAGS=-lpthread -lm

all: ${PROGRAM}

${PROGRAM}: src/proxy.c inc/definitions.h inc/log.h
	gcc ${CFLAGS} src/proxy.c ${LDFLAGS} -o ${PROGRAM}

clean:
	rm -rf ${PROGRAM}
