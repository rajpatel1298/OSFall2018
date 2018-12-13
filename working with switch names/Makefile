COMPILER = gcc
FILESYSTEM_FILES = clientSNFS.c

build: $(FILESYSTEM_FILES)
	$(COMPILER) $(FILESYSTEM_FILES) -o clientSNFS `pkg-config fuse --cflags --libs`
	echo 'To Mount: ./clientSNFS -f /tmp/mounty'

clean:
	rm clientSNFS
