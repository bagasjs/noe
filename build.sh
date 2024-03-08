#!/bin/sh

set -xe

cc="clang"
common_flags="-Wall -Wextra -I./src/vendors/glad/include/"

vendor_dir="./src/vendors"
vendor_sources="${vendor_dir}/glad/src/glad.c"

test_cflags="${common_flags} -ggdb -D_CRT_SECURE_NO_WARNINGS"
test_lflags="-lopengl32 -lgdi32 -luser32 -lkernel32"
test_sources="./src/noe_platform_win32.c ./src/noe_core.c ./src/noe_draw.c ./win32_test.c ${vendor_sources}"

$cc $test_cflags -o ./test.exe $test_sources $test_lflags
