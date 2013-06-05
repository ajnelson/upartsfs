#!/bin/bash

set -e
set -x

mkdir test
./fusexmp test
ls test
sleep 1; fusermount -u test
rmdir test
