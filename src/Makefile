CC := clang
CFLAGS := -Wall $(shell pkg-config fuse --cflags)
LDFLAGS := $(shell pkg-config fuse --libs)

targets = fusexmp

all: $(targets)

fusexmp: fusexmp.c
	$(CC) $(CFLAGS) $(LDFLAGS) -lulockmgr $< -o $@

clean:
	rm -f *.o
	rm -f $(targets)
