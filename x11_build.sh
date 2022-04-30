#!/bin/sh

# APP_NAME=editor
APP_NAME=x11main

LIBSRC="lib/glad/glad.c"
SRC="src/x11_platform.c src/gl_drawing.c"

CFLAGS="-g -std=c99 -Ilib -D_POSIX_C_SOURCE=199309L"
# CFLAGS="-Os -std=c99 -Ilib -D_POSIX_C_SOURCE=199309L"
LDFLAGS="-lrt -lGL -lX11 -lm"

gcc -o "$APP_NAME" src/main.c $SRC $LIBSRC $CFLAGS $LDFLAGS
# gcc -o "$APP_NAME" src/editor.c $SRC $LIBSRC $CFLAGS $LDFLAGS
