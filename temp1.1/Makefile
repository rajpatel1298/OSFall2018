COMPILER = gcc
FILESYSTEM_FILES = ssfs.c

build: $(FILESYSTEM_FILES)
	$(COMPILER) $(FILESYSTEM_FILES) -o ssfs `pkg-config fuse --cflags --libs`
	echo 'To Mount: ./ssfs -f /tmp/yo'

clean:
	rm ssfs
