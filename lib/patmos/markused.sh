#!/bin/bash -e

# This script adds a llvm.used metainfo for all declarations in an .ll file

LL_FILE=$1
if [ ! -f $LL_FILE ]; then
  error_exit 1 "Bitcode file $LL_FILE does not exist!"
fi

OUT_FILE=$2
if [ "$2" == "" ]; then
    OUT_FILE=$LL_FILE
fi

function error_exit {
  local errno=$1; shift
  echo "$(basename $0): ERROR: $*" >&2
  exit $errno
}


echo "Adding llvm.used to $OUT_FILE"

cnt=0
LLVM_USED=

IFS=$'\n'
for fn in $(grep "declare" $LL_FILE); do
    if [ "x$LLVM_USED" != "x" ]; then
	LLVM_USED="$LLVM_USED,\n"
    fi
    # Extract signature, remove variable names of arguments
    SIG=$(echo $fn | sed "s/declare \([a-z0-9*]*\) *\(@[a-zA-Z0-9_]*\)\((.*)\).*/\1 \3* \2/" | sed "s/noalias sret //g" | sed "s/\( nocapture\)* %[a-zA-Z][a-zA-Z0-9_\.]*//g")
    LLVM_USED="$LLVM_USED  i8* bitcast ($SIG to i8*)"
    cnt=$((cnt+1))
done

if [ "$OUT_FILE" != "$LL_FILE" ]; then
    cat $LL_FILE > $OUT_FILE
fi
if [ $cnt != 0 ]; then
  echo >> $OUT_FILE
  echo "@llvm.used = appending global [$cnt x i8*] [" >> $OUT_FILE
  echo -e "$LLVM_USED" >> $OUT_FILE
  echo '], section "llvm.metadata"' >> $OUT_FILE
fi

exit 0
