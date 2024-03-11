CC := clang
COMMON_CFLAGS := -Wall -Wextra

VENDOR_DIR := ./src/vendors
VENDOR_SOURCES := $(VENDOR_DIR)/glad/src/glad.c

NOE_CFLAGS  := -I./src/vendors/glad/include/ -I./src/vendors/stb/
NOE_SOURCES := ./src/noe_core.c ./src/noe_draw.c ./src/noe_platform_linux.c ./src/noe_platform_linux_x11.c 
NOE_SOURCES += ./src/noe_text.c ./src/noe_image.c

TEST_CFLAGS := $(COMMON_CFLAGS) $(NOE_CFLAGS) -ggdb
TEST_LFLAGS := -lX11 -lGL -lm

test.exe: ./test.c $(NOE_SOURCES) $(VENDOR_SOURCES)
	$(CC) $(TEST_CFLAGS) -o $@ $^ $(TEST_LFLAGS)
