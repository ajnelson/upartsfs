#!/bin/bash

set -e
set -x

source _reset_test.sh
source _test_lib.sh

mkdir test
${VALGRIND_COMMAND} $top_srcdir/src/fusexmp test

first_regular_file="$(find test -type f | head -n1)"
test -f "$first_regular_file"
test -r "$first_regular_file"

rc=0
./test_open r "$first_regular_file" || rc=$?
if [ $rc -ne 0 ]; then
  echo "Error: ./test_open exited status $rc" >&2
  exit $rc
fi

sleep 3; fusermount -u test
rmdir test
