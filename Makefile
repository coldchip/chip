CC=gcc

chip:
	$(CC) src/*.c -o chip -g -std=c11 -lm 

run:
	./chip

clean:
	rm -rf chip