AM_CPPFLAGS = -I$(top_srcdir)/deps/sleuthkit -Wall -D_FILE_OFFSET_BITS=64 @FUSE_CFLAGS@
LDADD = $(top_srcdir)/deps/sleuthkit/tsk3/libtsk3.la @FUSE_LIBS@
LDFLAGS += -static

bin_PROGRAMS = upartsfs fusexmp
fusexmp_SOURCES = fusexmp.c
upartsfs_SOURCES = upartsfs.c

TESTS = \
  test_mount_fusexmp.sh \
  test_readdir_fusexmp.sh
# test_mount_upartsfs.sh \
# test_readdir_upartsfs.sh
#TODO upartsfs tests

check-valgrind:
	./test_valgrind_mount_fusexmp.sh
	./test_valgrind_readdir_fusexmp.sh
