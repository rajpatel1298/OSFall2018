all: test

test: testCreate.c
	gcc -Wall -Werror -fsanitize=address testCreate.c -o testCreate

clean: testCreate
	rm -rf testCreate
