CC=gcc

chip:
	$(CC) src/*.c -o chip -Ofast -std=c11 -lm -s

run:
	./chip

clean:
	rm -rf chip