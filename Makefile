CC=gcc

eval:
	$(CC) src/*.c -o eval -Ofast -std=c99 -lm

run:
	./eval

clean:
	rm -rf eval