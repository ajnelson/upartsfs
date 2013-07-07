CC = clang
AM_CPPFLAGS = -I$(top_srcdir)/deps/sleuthkit -Wall -D_FILE_OFFSET_BITS=64 @FUSE_CFLAGS@ @DEPS_CFLAGS@
LDADD = $(top_srcdir)/deps/sleuthkit/tsk3/libtsk3.la @FUSE_LIBS@ @DEPS_LIBS@
LDFLAGS += -static

noinst_PROGRAMS = fusexmp
bin_PROGRAMS = upartsfs

fusexmp_SOURCES = fusexmp.c
upartsfs_SOURCES = upartsfs.c upartsfs.h unconfig.h
