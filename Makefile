CC := clang
COMMON_CFLAGS := -Wall -Wextra -I./src/vendors/glad/include/

VENDOR_DIR := ./src/vendors
VENDOR_SOURCES := $(VENDOR_DIR)/glad/src/glad.c

NOE_SOURCES := ./src/noe_core.c ./src/noe_draw.c ./src/noe_platform_linux.c ./src/noe_platform_linux_x11.c

TEST_CFLAGS := $(COMMON_CFLAGS) -ggdb
TEST_LFLAGS := -lX11 -lGL -lm
TEST_SOURCES := ./src/noe_platform_linux.c $(VENDOR_SOURCES) $(NOE_SOURCES)

test.exe: ./test.c $(TEST_SOURCES)
	$(CC) $(TEST_CFLAGS) -o $@ $^ $(TEST_LFLAGS)
