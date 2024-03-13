#!/bin/sh
set -xe
CC=clang
$CC -I./src/ -Wl,-rpath=$HOME/Programming/bagasjs/noe/build -o ./build/test ./test.c -Lbuild -lnoe
