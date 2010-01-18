#!/bin/sh


if [ $# -ne 1 ]; then
    echo "${FUNCNAME[0]} <coarse3d | periodicshear>";
    exit;
fi

rm -f movie/*.jpg
./foam ~/Documents/phd-swansea/WIVC/COARSE3D summary_30_0001*.dmp
