CC=gcc

chip:
	$(CC) src/*.c -o chip -Ofast -std=c99 -lm

run:
	./chip

clean:
	rm -rf chip