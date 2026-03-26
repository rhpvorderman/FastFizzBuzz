#!/usr/bin/env bash
set -eu -o pipefail

for CODE_FILE in *.c
do
echo $CODE_FILE
PROGRAM="${CODE_FILE}.out"
gcc -Wall -O3 -mlzcnt -o $PROGRAM $CODE_FILE
./$PROGRAM | xxh128sum
hyperfine -w0 -r 1 "./$PROGRAM > /dev/null"
done
