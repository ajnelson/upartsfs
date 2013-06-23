#!/bin/bash
if [ -d test ]; then
  if [ $(mount | grep "$PWD/test" | wc -l) -eq 1 ]; then
    fusermount -u test
  fi
  rm -rf test
fi
