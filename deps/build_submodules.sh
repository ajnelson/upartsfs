#!/bin/bash

set -e
set -x


pushd sleuthkit
./bootstrap
./configure --prefix=$PWD/build
make -j
make install
popd

#Build FUSE - Not actually necessary for upartsfs, and the automake-derived Make rules for fusexmp weren't terribly helpful in the module's examples/ directory.  The build also fails without sudo.
exit 0    #Bail on building FUSE
pushd ./fuse
./makeconf.sh
./configure --prefix=$PWD/build
make
make check
make distcheck #This step happens to fail; $(DESTDIR) in the util/ Automake rules defaults to /sbin, and this test is purposefully run without sudo.
make install 
popd
