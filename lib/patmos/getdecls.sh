#!/bin/bash -e

# this is a temporary helper script to extract the defined symbols from an archive
# and write declarations into a file.

# passing commands as parameters
LLVM_AR=${2-llvm-ar}
LLVM_NM=${3-llvm-nm}
LLVM_DIS=${4-llvm-dis}

#ARCHIVE=${1-../../build/lib/librt.a}
ARCHIVE=$1
if [ ! -f $ARCHIVE ]; then
  error_exit 1 "Archive file $ARCHIVE does not exist!"
fi



trap "cleanup" EXIT

function error_exit {
  local errno=$1; shift
  echo "$(basename $0): ERROR: $*" >&2
  exit $errno
}

function cleanup {
  test -d $TMPDIR && rm -fr $TMPDIR #&& echo $TMPDIR removed
}



#LIBNAME=$(sed 's/lib\([[:alnum:]]*\)\.a/\1/' <(basename $ARCHIVE))
OUT_BASE=${ARCHIVE%.a}syms
OUT_LST=${OUT_BASE}.lst
OUT_LL=${OUT_BASE}.ll

echo Reading $ARCHIVE
$LLVM_NM -defined-only -extern-only $ARCHIVE | sed -n "s/^[\t ]*T \(.*\)$/\1/p"  > $OUT_LST

# prepend each symbol with @
SYMBOLS=$(sed 's/^.*$/@&/' < $OUT_LST)

TMPDIR=$(mktemp -d)
pushd $TMPDIR > /dev/null

$LLVM_AR x $ARCHIVE
# llvm-ar creates an archive instead of returning an exit code
ls *.bc > /dev/null 2>&1 || error_exit $? "Could not extract $ARCHIVE!"

echo 'Extracting declarations'
echo 'target triple = "patmos-unknown-elf"' > $OUT_LL
for f in *.bc
do
  $LLVM_DIS $f
  LLFILE=${f%.bc}.ll
  echo "; $LLFILE"
  grep "$SYMBOLS" $LLFILE | grep 'define' | sed -e "s/define/declare/" -e "s/ {$//"
  rm $f $LLFILE
done >> $OUT_LL

popd > /dev/null
echo Done.

exit 0
