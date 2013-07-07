#!/bin/bash

#One-liner c/o http://stackoverflow.com/a/246128/1207160
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ -d ${SCRIPTDIR}/test ]; then
  if [ $(mount | grep "${SCRIPTDIR}/test" | wc -l) -eq 1 ]; then
    fusermount -u ${SCRIPTDIR}/test
  fi
  rm -rf ${SCRIPTDIR}/test
fi
