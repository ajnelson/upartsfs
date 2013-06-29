#!/bin/bash

set -e
set -x

source _reset_test.sh
source _test_lib.sh

mkdir test
${VALGRIND_COMMAND} ./fusexmp test
ls test
sleep 3; fusermount -u test
rmdir test
