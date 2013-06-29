#!/bin/bash

set -e
set -x

source _reset_test.sh
source _test_lib.sh

#Default to using CFREDS Mac image
if [ -z "$IMAGEFILE" ]; then
  IMAGEFILE=macwd.e01
fi

mkdir test
${VALGRIND_COMMAND} ./upartsfs test "$IMAGEFILE"
sleep 3; fusermount -u test
rmdir test
