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
ls test
ls -a test
ls -al test
test -d test/in_order
ls -al test/in_order
test -e test/in_order/0  #Test file exists
test -f test/in_order/0  #Test file exists and is a regular file
test -r test/in_order/0  #Test file exists and is readable

test -d test/by_offset
ls -al test/by_offset


sleep 3; fusermount -u test
rmdir test
