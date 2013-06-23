#!/bin/bash

set -e
set -x

source _reset_test.sh

mkdir test
valgrind ./fusexmp test
ls test
sleep 3; fusermount -u test
rmdir test
