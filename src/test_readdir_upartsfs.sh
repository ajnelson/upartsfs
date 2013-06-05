#!/bin/bash

set -e
set -x

mkdir test
./upartsfs test
ls test
sleep 1; fusermount -u test
rmdir test
