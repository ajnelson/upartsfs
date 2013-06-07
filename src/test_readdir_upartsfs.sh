#!/bin/bash

set -e
set -x

mkdir test
./upartsfs test
ls test
sleep 3; fusermount -u test
rmdir test
