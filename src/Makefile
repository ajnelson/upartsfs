CC := clang
CFLAGS := -Wall $(shell pkg-config fuse --cflags)
LDFLAGS := $(shell pkg-config fuse --libs)
topsrcdir := ..

targets = fusexmp upartsfs

all: $(targets)

fusexmp: fusexmp.c
	$(CC) $(CFLAGS) $(LDFLAGS) -lulockmgr $< -o $@

upartsfs: upartsfs.c
	$(CC) $(CFLAGS) $(LDFLAGS) -I $(topsrcdir)/deps/sleuthkit/build/include -lulockmgr $< -o $@

check: upartsfs
	mkdir test
	./upartsfs test
	ls test
	sleep 1; fusermount -u test
	rmdir test

clean:
	rm -f *.o
	rm -f $(targets)
