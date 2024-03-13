#!/bin/sh

NAME="noe"
CC="clang"
BUILD_DIR="./build/"
LFLAGS="-lX11 -lm"

NOE_CFLAGS="-Wall -Wextra -Wpedantic -fPIC -I./src/vendors/stb/ -I./src/vendors/glad/include/ -DNOE_BUILDTYPE_SHAREDLIB"
NOE_SOURCES=(
"./src/noe_core.c"
"./src/noe_text.c"
"./src/noe_image.c"

"./src/noe_platform_desktop.c"

"./src/vendors/glad/src/glad.c"

)

GLFW_CFLAGS="-fPIC -D_GLFW_X11"
GLFW_SOURCES=(
    "./src/vendors/glfw/src/context.c"
    "./src/vendors/glfw/src/init.c"
    "./src/vendors/glfw/src/input.c"
    "./src/vendors/glfw/src/monitor.c"
    "./src/vendors/glfw/src/platform.c"
    "./src/vendors/glfw/src/vulkan.c"
    "./src/vendors/glfw/src/window.c"

    "./src/vendors/glfw/src/null_init.c"
    "./src/vendors/glfw/src/null_joystick.c"
    "./src/vendors/glfw/src/null_monitor.c"
    "./src/vendors/glfw/src/null_window.c"
    "./src/vendors/glfw/src/egl_context.c"
    "./src/vendors/glfw/src/osmesa_context.c"

    "./src/vendors/glfw/src/posix_module.c"
    "./src/vendors/glfw/src/posix_poll.c"
    "./src/vendors/glfw/src/posix_thread.c"
    "./src/vendors/glfw/src/posix_time.c"

    "./src/vendors/glfw/src/linux_joystick.c"

    "./src/vendors/glfw/src/x11_init.c"
    "./src/vendors/glfw/src/x11_monitor.c"
    "./src/vendors/glfw/src/x11_window.c"
    "./src/vendors/glfw/src/xkb_unicode.c"
    "./src/vendors/glfw/src/glx_context.c"
)

OBJECTS=()

compile_sources() {
    local cflags="$1"
    local cache_dir="$2"
    shift 2
    local sources=("$@")

    for source_file in "${sources[@]}"; do
        file_name=$(basename "$source_file")
        file_name_without_ext="${file_name%.*}"
        echo "Compiling $file_name"
        $CC $cflags -c -o "$cache_dir/$file_name_without_ext.o" $source_file

        OBJECTS+=("$cache_dir/$file_name_without_ext.o")
    done
}

link_sharedlib() {
    local name="$1"
    local location="$2"
    local lflags="$3"
    shift 3
    local objects=("$@")

    echo "Linking lib$name.so"
    $CC $lflags -shared -o "$location/lib$name.so" "${objects[@]}"
}

if [ ! -d $BUILD_DIR ]; then
    mkdir $BUILD_DIR
fi

if [ ! -d $BUILD_DIR/cache ]; then
    mkdir $BUILD_DIR/cache
fi

if [ ! -d $BUILD_DIR/cache/glfw ]; then
    mkdir $BUILD_DIR/cache/glfw
fi

echo "--- Compiling glfw ---"
compile_sources "$GLFW_CFLAGS" "$BUILD_DIR/cache/glfw" "${GLFW_SOURCES[@]}"

echo "--- Compiling NOE ---"
compile_sources "$NOE_CFLAGS" "$BUILD_DIR/cache" "${NOE_SOURCES[@]}"

echo "--- Linking ---"
link_sharedlib "$NAME" "$BUILD_DIR" "$LFLAGS" "${OBJECTS[@]}"

# Build test TEST
echo "--- Building test --- "
$CC -I./src/ -Wl,-rpath=$HOME/Programming/bagasjs/noe/build -o ./build/test ./test.c -Lbuild -lnoe

echo "DONE"
