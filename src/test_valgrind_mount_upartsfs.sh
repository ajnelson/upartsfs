#!/bin/bash

set -e
set -x

source _reset_test.sh

#Default to using CFREDS Mac image
if [ -z "$IMAGEFILE" ]; then
  IMAGEFILE=macwd.e01
fi

mkdir test
valgrind --leak-check=full ./upartsfs "$IMAGEFILE" test
sleep 3; fusermount -u test
rmdir test
