AM_CPPFLAGS = -I$(top_srcdir)/deps/sleuthkit -Wall -D_FILE_OFFSET_BITS=64 @FUSE_CFLAGS@
LDADD = $(top_srcdir)/deps/sleuthkit/tsk3/libtsk3.la @FUSE_LIBS@
LDFLAGS += -static

bin_PROGRAMS = upartsfs fusexmp
fusexmp_SOURCES = fusexmp.c
upartsfs_SOURCES = upartsfs.c upartsfs.h

TESTS = \
  test_mount_fusexmp.sh \
  test_readdir_fusexmp.sh \
  test_mount_upartsfs.sh
#TODO  test_readdir_upartsfs.sh

check-valgrind:
	./test_mount_fusexmp.sh --with-valgrind    || (sleep 0.1s; ./_reset_test.sh ; sleep 0.5s; exit 1)
	./test_readdir_fusexmp.sh --with-valgrind  || (sleep 0.1s; ./_reset_test.sh ; sleep 0.5s; exit 1)
	./test_mount_upartsfs.sh --with-valgrind   || (sleep 0.1s; ./_reset_test.sh ; sleep 0.5s; exit 1)
#TODO	./test_readdir_upartsfs.sh --with-valgrind || (sleep 0.1s; ./_reset_test.sh ; sleep 0.5s; exit 1)
