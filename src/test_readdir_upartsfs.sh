#!/bin/bash

set -e
set -x

source _reset_test.sh

#Default to using CFREDS Mac image
if [ -z "$IMAGEFILE" ]; then
  IMAGEFILE=macwd.e01
fi

mkdir test
./upartsfs "$IMAGEFILE" test
ls test
ls -a test
ls -al test
ls -al test/by_index
sleep 3; fusermount -u test
rmdir test
