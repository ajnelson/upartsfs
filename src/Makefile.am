AM_CPPFLAGS = -I$(top_srcdir)/deps/sleuthkit -Wall -D_FILE_OFFSET_BITS=64
LDADD = $(top_srcdir)/deps/sleuthkit/tsk3/libtsk3.la
LDFLAGS += -static

bin_PROGRAMS = upartsfs
upartsfs_SOURCES = upartsfs.c
