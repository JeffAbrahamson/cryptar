#!/bin/bash

make random

file_base=random_$(head -c $((500 + $RANDOM)) < /dev/urandom | sha1sum | head -c 40)
file_1=${file_base}_1
file_2=${file_base}_2
./random > $file_1
./random > $file_2
lines=$(cat $file_1 $file_2 | sort -u | wc -l)
rm $file_1 $file_2
if [ $lines != 100 ]; then
    echo "Some lines are identical, this shouldn't happen."
    exit 1
fi
echo "Test passed."
exit 0
