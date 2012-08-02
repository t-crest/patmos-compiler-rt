#!/bin/bash

# this is a temporary helper script to extract the defined symbols from an archive
# and write declarations into a file.


ARCHIVE=${1:-../../build/lib/libll.a}

echo Reading $ARCHIVE
SYMBOLS=$(llvm-nm -defined-only $ARCHIVE | sed -n "s/^[\t ]*T \(.*\)$/@\1/p")

llvm-ar x $ARCHIVE
for f in *.bc
do
  llvm-dis $f
  LLFILE=${f%.bc}.ll
  echo "; $LLFILE"
  grep "$SYMBOLS" $LLFILE | grep 'define' | grep -v 'define internal' | sed -e "s/define/declare/" -e "s/ {$//"
  rm $f $LLFILE
done > decls.ll
