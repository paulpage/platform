LIBSRC = lib/glad/glad.c
SRC = src/x11_platform.c src/gl_drawing.c
OBJ = ${SRC:.c=.o}

CFLAGS = -g -std=c99 -Ilib -D_POSIX_C_SOURCE=199309L
LDFLAGS= -lrt -lGL -lX11 -lm

.c.o:
	${CC} -c ${CFLAGS} $<
