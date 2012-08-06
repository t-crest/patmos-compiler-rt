#!/bin/bash

# this is a temporary helper script to extract the defined symbols from an archive
# and write declarations into a file.


ARCHIVE=${1-../../build/lib/libllspt.a}
if [ ! -f $ARCHIVE ]; then
  echo File $ARCHIVE does not exist!
  exit
fi


#LIBNAME=$(sed 's/lib\([[:alnum:]]*\)\.a/\1/' <(basename $ARCHIVE))
OUT_BASE=${ARCHIVE%.a}syms
OUT_LST=${OUT_BASE}.lst
OUT_LL=${OUT_BASE}.ll

echo Reading $ARCHIVE
llvm-nm -defined-only -extern-only $ARCHIVE | sed -n "s/^[\t ]*T \(.*\)$/\1/p"  > $OUT_LST

# prepend each symbol with @
SYMBOLS=$(sed 's/^.*$/@&/' < $OUT_LST)

echo 'target triple = "patmos-unknown-elf"' > $OUT_LL
llvm-ar x $ARCHIVE
for f in *.bc
do
  llvm-dis $f
  LLFILE=${f%.bc}.ll
  echo "; $LLFILE"
  grep "$SYMBOLS" $LLFILE | grep 'define' | sed -e "s/define/declare/" -e "s/ {$//"
  rm $f $LLFILE
done >> $OUT_LL
