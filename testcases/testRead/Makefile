all: test

test: testRead.c
	gcc -Wall -Werror -fsanitize=address testRead.c -o testRead
clean: testRead
	rm -rf testRead
