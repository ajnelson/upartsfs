CC = clang
AM_CPPFLAGS = -Wall @DEPS_CFLAGS@
LDADD = @DEPS_LIBS@

noinst_PROGRAMS = test_glib

TESTS = \
  test_glib \
  test_mount_fusexmp.sh \
  test_readdir_fusexmp.sh \
  test_mount_upartsfs.sh \
  test_readdir_upartsfs.sh

test_glib_SOURCES = test_glib.c
test_glib_CFLAGS = $(DEPS_CFLAGS)
test_glib_LIBS = $(DEPS_LIBS)

check-valgrind:
	./test_mount_fusexmp.sh --with-valgrind    || (sleep 0.1s; ./_reset_test.sh ; sleep 0.5s; exit 1)
	./test_readdir_fusexmp.sh --with-valgrind  || (sleep 0.1s; ./_reset_test.sh ; sleep 0.5s; exit 1)
	./test_mount_upartsfs.sh --with-valgrind   || (sleep 0.1s; ./_reset_test.sh ; sleep 0.5s; exit 1)
	./test_readdir_upartsfs.sh --with-valgrind || (sleep 0.1s; ./_reset_test.sh ; sleep 0.5s; exit 1)
