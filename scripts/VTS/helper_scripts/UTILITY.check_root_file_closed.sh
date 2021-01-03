#!/bin/bash

#### checks if a root file has been properly closed.

if [ "$1" == "" ]; then
    echo "Usage: $0 root_file_to_check_if_closed.root"
fi

filename_to_check=$(readlink -f $1)
#echo $filename_to_check
root -b -l -q -e "$EVNDISPSYS/macros/VTS/checkCorrupted.C(\"$filename_to_check\")" >/dev/null 2>&1
if [ $? -ne 0 ]; then
    exit 1
fi
