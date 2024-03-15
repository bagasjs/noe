#!/bin/sh
set -xe
CC=clang

# $CC -I./src/ -Wl,-rpath=$HOME/Programming/bagasjs/noe/build -o ./build/test ./test.c -Lbuild -lnoe -lm
$CC -I./src/ -o ./build/test ./test.c -Lbuild -lnoe -lm
$CC -I./src/ -o ./build/audiobox ./audiobox/audiobox.c ./audiobox/miniaudio.c -L build -lnoe -lm
