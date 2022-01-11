#!/bin/sh

set -e

mkdir -p build/glfw
cd build/glfw

# Wayland
# cmake ../../lib/glfw \
#     -DGLFW_USE_WAYLAND=ON \
#     -DGLFW_BUILD_DOCS=OFF \
#     -DGLFW_BUILD_EXAMPLES=OFF \
#     -DGLFW_BUILD_TESTS=OFF

# X11
cmake ../../lib/glfw \
    -DGLFW_USE_WAYLAND=OFF \
    -DGLFW_BUILD_DOCS=OFF \
    -DGLFW_BUILD_EXAMPLES=OFF \
    -DGLFW_BUILD_TESTS=OFF

make -j8

cd ../..

APP_NAME="example"
SOURCES="src/glfw_platform.c src/main.c"

LIBRARY_SOURCES="lib/glad/*.c build/glfw/src/libglfw3.a"
DEBUG_FLAGS="-g -O0"
COMPILER_FLAGS="-std=c99 -Os -flto -Ilib -Ilib/glfw/include"
# WARNING_FLAGS="-Wall -Wextra -Wpedantic"
# LINKER_FLAGS="-flto -lm -ldl -lpthread -lX11 -lxcb -lGL -lGLX -lXext -lGLdispatch -lXau -lXdmcp -lwayland-client -lglfw -lrt"
LINKER_FLAGS="-flto -lm -ldl -lpthread -lX11 -lxcb -lGL -lGLX -lXext -lGLdispatch -lXau -lXdmcp -lrt"

gcc -o "build/$APP_NAME" $LIBRARY_SOURCES $SOURCES $COMPILER_FLAGS $DEBUG_FLAGS $WARNING_FLAGS $GLFW_FLAGS $LINKER_FLAGS
