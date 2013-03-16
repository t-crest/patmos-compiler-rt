#!/bin/bash -e

# This script adds a llvm.used metainfo for all declarations in an .ll file

LL_FILE=$1
if [ ! -f $LL_FILE ]; then
  error_exit 1 "Bitcode file $LL_FILE does not exist!"
fi


function error_exit {
  local errno=$1; shift
  echo "$(basename $0): ERROR: $*" >&2
  exit $errno
}


echo "Adding llvm.used to $LL_FILE"

cnt=0
LLVM_USED=

IFS=$'\n'
for fn in $(grep "declare" $LL_FILE); do
    if [ "x$LLVM_USED" != "x" ]; then
	LLVM_USED="$LLVM_USED,"
    fi
    # Extract signature, remove variable names of arguments
    SIG=$(echo $fn | sed "s/declare \([a-z0-9*]*\) \(@[a-zA-Z0-9_]*\)\((.*)\).*/\1 \3* \2/" | sed "s/ %[a-zA-Z_]*//g")
    LLVM_USED="$LLVM_USED i8* bitcast ($SIG to i8*)"
    cnt=$((cnt+1))
done

echo >> $LL_FILE
echo "@llvm.used = appending global [$cnt x i8*] [" >> $LL_FILE 
echo "  $LLVM_USED" >> $LL_FILE
echo '], section "llvm.metadata"' >> $LL_FILE

echo Done.

exit 0
