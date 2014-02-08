#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([upartsfs], [0.0.0], [ajnelson@acm.org])
AC_CONFIG_SRCDIR([src/upartsfs.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])
AM_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([m4])
LT_INIT

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lulockmgr':
AC_CHECK_LIB([ulockmgr], [main])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h string.h sys/time.h unistd.h])

#Update PKG_CONFIG_PATH
# OSXFUSE puts a couple pkgconfig files into /usr/local/lib/pkgconfig.  For some reason, that directory is not in the pkgconfig's search path.
# This SO answer rec'd updating the path: http://stackoverflow.com/a/10229811
# The sufficient export command is c/o SSHFS's configure.ac.
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
PKG_CHECK_MODULES(
  [FUSE],
  [fuse],
  ,
  AC_MSG_ERROR([fuse not found.])
)
AC_SUBST(FUSE_LIBS)
AC_SUBST(FUSE_CFLAGS)
#Check for GLib (generic, portable linked lists)
#Macro c/o: <http://stackoverflow.com/a/3538635/1207160>
PKG_CHECK_MODULES([DEPS], [glib-2.0 >= 2.24.1])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_CHECK_FUNCS([lchown memset mkdir mkfifo rmdir])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 tests/Makefile
                 tests/_test_lib.sh])
AC_OUTPUT
