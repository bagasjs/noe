CC := clang -Wall -Wextra -pedantic -O3
LFLAGS := -lgdi32 -luser32

demo.exe: ./noe.c ./demo.c
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)
