all: test

test: testTruncate.c
	gcc -D_GNU_SOURCE -std=c99 -Wall -Werror -fsanitize=address testTruncate.c -o testTruncate
clean: testWrite
	rm -rf testTruncate
