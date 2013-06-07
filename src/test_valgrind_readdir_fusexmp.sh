#!/bin/bash

set -e
set -x

mkdir test
valgrind ./fusexmp test
ls test
sleep 3; fusermount -u test
rmdir test
