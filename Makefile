CC=gcc

eval:
	$(CC) src/*.c -o eval -Ofast -std=c99

run:
	./eval

clean:
	rm -rf eval