all: test

test: testWrite.c
	gcc -Wall -Werror -fsanitize=address testWrite.c -o testWrite
clean: testWrite
	rm -rf testWrite
