CC := clang 
COMMON_CFLAGS := -Wall -Wextra -pedantic
CFLAGS := $(COMMON_CFLAGS)
# -O3 -I.

LFLAGS := -lgdi32 -luser32

.PHONY: all
all: build/game.exe build/paint.exe build/example_image_cropping.exe build/example_text_drawing.exe

build/example_text_drawing.exe: ./noe.c ./noe_ext.c ./examples/example_text_drawing.c
	$(CC) $(CFLAGS) -D_CRT_SECURE_NO_WARNINGS -o $@ $^ $(LFLAGS)

build/game.exe: ./noe.c ./examples/game.c
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

build/paint.exe: ./noe.c ./examples/paint.c
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

build/example_image_cropping.exe: ./noe.c ./examples/example_image_cropping.c
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

