all: test

test: mapreduce.c
	gcc -Wall -Werror -fsanitize=address -lpthread mapreduce.c -o mapred
clean: mapred
	rm -rf mapred
