#!/bin/bash

set -e
set -x

./bootstrap.sh
./configure --prefix=$PWD/build
make
make check
make install
