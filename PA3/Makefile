all: test

test: serverSNFS.c
	gcc -Wall -Werror -fsanitize=address -lpthread serverSNFS.c -o serverSNFS

clean: serverSNFS
	rm -rf serverSNFS
