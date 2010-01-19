#!/bin/sh


if [ $# -ne 1 ]; then
    echo "${FUNCNAME[0]} <coarse3d | periodicshear>";
    exit;
fi

rm -f movie/*.jpg
./foam ~/Documents/phd-swansea/WIVC/COARSE3D summary_30_0*.dmp

#rm -f movie/*.jpg
#./foam ~/Documents/phd-swansea/WIVC/PERIODICSHEAR shear_65_0*_2.dmp