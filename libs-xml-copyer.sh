#!/bin/bash
# Script that periodically checks if libs.xml has been changed and
# overwrites it with our custom libs.xml .
# Kind of a ugly fix, but this has to work for now.
# Maybe later I have time to investigate a possibility to write
# a patch for necessitas qt-creator to include my custom shared libraries
# in libs.xml bundledlibraries .

SOURCE="sshmusicplay/android/res/values/libs.xml.base"
TARGET="sshmusicplay/android/res/values/libs.xml"

echo "Watcing for changes in $TARGET"

while true; do
    diffcount=`diff $SOURCE $TARGET | wc -l`;
    if test 0 -ne $diffcount; then
        # Copy file if it has been changed
        cp -v $SOURCE $TARGET
    fi;
    sleep 1;
done
