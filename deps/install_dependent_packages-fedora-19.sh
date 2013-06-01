#!/bin/bash

set -e
set -x

sudo yum install \
  autoconf \
  automake \
  clang \
  fuse-devel \
  gcc-c++ \
  libtool
