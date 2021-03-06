#!/bin/sh

APP_NAME="example"
SOURCES="src/glfw_platform.c src/main.c"

LIBRARY_SOURCES="lib/*.c lib/glad/*.c"
DEBUG_FLAGS="-g -O0"
COMPILER_FLAGS="-std=c99 -Os -flto -Ilib -Ilib/glfw/include"
# WARNING_FLAGS="-Wall -Wextra -Wpedantic"
LINKER_FLAGS="-flto -lm -ldl -lpthread -lX11 -lxcb -lGL -lGLX -lXext -lGLdispatch -lXau -lXdmcp"
gcc -o "$APP_NAME" $LIBRARY_SOURCES $SOURCES $COMPILER_FLAGS $DEBUG_FLAGS $WARNING_FLAGS $LINKER_FLAGS
