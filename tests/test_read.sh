#!/bin/bash

set -e
set -x

source _reset_test.sh
source _test_lib.sh

#Default to using CFREDS Mac image
if [ -z "$IMAGEFILE" ]; then
  IMAGEFILE=$top_srcdir/src/macwd.e01
fi

MMCAT=../deps/sleuthkit/build/bin/mmcat
test -x "$MMCAT"

MD5COMM="openssl dgst -md5"

mkdir test
${VALGRIND_COMMAND} $top_srcdir/src/upartsfs test "$IMAGEFILE"

for part_index_path in $(ls test/in_order); do
  part_index=$(basename "$part_index_path")
  from_uparts=$(cat "test/in_order/$part_index" | $MD5COMM)
  from_mmcat=$("$MMCAT" "$IMAGEFILE" "$part_index" | $MD5COMM)
  if [ "x$from_uparts" == "x$from_mmcat" ]; then
    echo MATCH for partition $part_index: $from_uparts
  else
    echo MISMATCH for partition $part_index: $from_uparts $from_mmcat
    exit 1
  fi
done

sleep 3; fusermount -u test
rmdir test
