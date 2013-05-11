#!/bin/bash

set -e
set -x

#Assure FUSE is checked out
git submodule init deps/fuse
git submodule update deps/fuse

#Assure TSK is built
if [ ! -r "deps/sleuthkit/build/include/libtsk.h" ]; then
  echo "Note: Building TSK..." >&2
  git submodule init deps/sleuthkit
  git submodule update deps/sleuthkit
  pushd deps
  ./build_submodules.sh
  popd
fi
