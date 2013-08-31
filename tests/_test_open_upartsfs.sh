#!/bin/bash

if [ -z "$_TEST_OPEN_UPARTSFS_MODE" ]; then
  echo "Error: _test_open_upartsfs.sh: Must have variable \$_TEST_OPEN_UPARTSFS_MODE defined.  This script isn't meant to be called directly; it's meant to be called by a script that defines this variable." >&2
  exit 1
fi

set -e
set -x

source _reset_test.sh
source _test_lib.sh

#Default to using CFREDS Mac image
if [ -z "$IMAGEFILE" ]; then
  IMAGEFILE=$top_srcdir/src/macwd.e01
fi

mkdir test
${VALGRIND_COMMAND} $top_srcdir/src/upartsfs test "$IMAGEFILE"

first_regular_file=test/in_order/0
test -f "$first_regular_file"

rc=0
./test_open r "$first_regular_file" || rc=$?
if [ $rc -ne 0 ]; then
  echo "Error: ./test_open exited status $rc" >&2
  exit $rc
fi

sleep 3; fusermount -u test
rmdir test
