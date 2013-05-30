#!/bin/bash

set -e
set -x

#Assure FUSE is checked out
if [ ! -r "deps/fuse/include/fuse.h" ]; then
  git submodule init deps/fuse
  git submodule update deps/fuse
fi

#Assure TSK is built
if [ ! -r "deps/sleuthkit/build/include/tsk3/libtsk.h" ]; then
  echo "Note: Building TSK..." >&2
  git submodule init deps/sleuthkit
  git submodule update deps/sleuthkit
  pushd deps
  ./build_submodules.sh
  popd
fi

libtoolize -c || glibtoolize -c
aclocal
automake --foreign --add-missing --copy
autoreconf -i
