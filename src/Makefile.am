CC = clang
AM_CPPFLAGS = -I$(top_srcdir)/deps/sleuthkit -Wall -D_FILE_OFFSET_BITS=64 @FUSE_CFLAGS@ @DEPS_CFLAGS@
LDADD = $(top_srcdir)/deps/sleuthkit/tsk/libtsk.la @FUSE_LIBS@ @DEPS_LIBS@
LDFLAGS += -static

noinst_PROGRAMS = fusexmp loggedfs
bin_PROGRAMS = upartsfs

fusexmp_SOURCES = loggedfs.c
fusexmp_SOURCES = fusexmp.c
upartsfs_SOURCES = upartsfs.c upartsfs.h unconfig.h
