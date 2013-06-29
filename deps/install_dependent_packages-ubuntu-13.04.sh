#!/bin/bash

set -e
set -x

#Optional packages:
# fuse-dbg

sudo apt-get install \
  autoconf \
  automake \
  clang \
  fuse-dbg \
  libtool \
  libfuse-dev \
  valgrind
