#!/bin/bash

set -e
set -x

. _reset_test.sh

#Default to using CFREDS Mac image
if [ -z "$IMAGEFILE" ]; then
  IMAGEFILE=macwd.e01
fi

mkdir test
valgrind --leak-check=full --show-reachable=yes ./upartsfs "$IMAGEFILE" test
ls test
ls -a test
ls -al test
test -d test/in_order
ls -al test/in_order

test -d test/by_offset
ls -al test/by_offset


sleep 3; fusermount -u test
rmdir test
