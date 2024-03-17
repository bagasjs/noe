CC=clang

all: ./build/test.exe ./build/audiobox.exe

./build/test.exe: ./test.c
	$(CC) -I./src/ -o $@ $^ -Lbuild -lnoe -lm

./build/audiobox.exe: ./build/cache/audiobox.o ./build/cache/miniaudio.o ./build/cache/nogui.o
	$(CC) -o $@ $^ -L build -lnoe -lm 

./build/cache/nogui.o: ./audiobox/nogui.c
	$(CC) -I./src/ -c -o $@ $^ 

./build/cache/audiobox.o: ./audiobox/audiobox.c
	$(CC) -I./src/ -c -o $@ $^ 

./build/cache/miniaudio.o: ./audiobox/miniaudio.c
	$(CC) -I./src/ -c -o $@ $^


# $(CC) -I./src/ -Wl,-rpath=$HOME/Programming/bagasjs/noe/build -o ./build/test ./test.c -Lbuild -lnoe -lm
