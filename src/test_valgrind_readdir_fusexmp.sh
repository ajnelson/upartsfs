#!/bin/bash

set -e
set -x

source _reset_test.sh

mkdir test
valgrind --leak-check=full --show-reachable=yes ./fusexmp test
ls test
sleep 3; fusermount -u test
rmdir test
