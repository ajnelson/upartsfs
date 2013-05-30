#!/bin/bash

set -e

./bootstrap.sh
./configure --prefix=$PWD/build
make
make check
make install
