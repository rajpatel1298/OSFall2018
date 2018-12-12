all: test

test: testOpen.c
	gcc -Wall -Werror -fsanitize=address testOpen.c -o testOpen

clean: testOpen
	rm -rf testOpen
