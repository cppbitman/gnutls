#!/bin/sh
# Test select() on file descriptors opened for writing.

tmpfiles=""
trap 'rm -fr $tmpfiles' 1 2 3 15

tmpfiles="$tmpfiles t-select-out.out t-select-out.tmp"

# Regular files.

rm -f t-select-out.tmp
./test-select-fd${EXEEXT} w 1 t-select-out.tmp > t-select-out.out
test `cat t-select-out.tmp` = "1" || exit 1

# Pipes.

rm -f t-select-out.tmp
( { echo abc; ./test-select-fd${EXEEXT} w 1 t-select-out.tmp; } | { sleep 1; cat; } ) > /dev/null
test `cat t-select-out.tmp` = "0" || exit 1

rm -f t-select-out.tmp
( { sleep 1; echo abc; ./test-select-fd${EXEEXT} w 1 t-select-out.tmp; } | cat) > /dev/null
test `cat t-select-out.tmp` = "1" || exit 1

# Special files.

rm -f t-select-out.tmp
./test-select-fd${EXEEXT} w 1 t-select-out.tmp > /dev/null
test `cat t-select-out.tmp` = "1" || exit 1

rm -fr $tmpfiles

exit 0
