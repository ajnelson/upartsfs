#!/bin/bash

set -e
set -x

#Default to using CFREDS Mac image
if [ -z "$IMAGEFILE" ]; then
  IMAGEFILE=macwd.e01
fi

mkdir test
./upartsfs "$IMAGEFILE" test
ls test
sleep 3; fusermount -u test
rmdir test
