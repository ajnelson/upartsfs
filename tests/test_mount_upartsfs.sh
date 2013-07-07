#!/bin/bash

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
sleep 3; fusermount -u test
rmdir test
