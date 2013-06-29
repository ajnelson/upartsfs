#!/bin/bash

VALGRIND_COMMAND=
while [ $# -ge 1 ]; do
  case "$1" in
    --with-valgrind )
      VALGRIND_COMMAND="valgrind --leak-check=full --show-reachable=yes"
    ;;
    * )
    break
    ;;
  esac
  shift
done
