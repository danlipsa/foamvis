Compiling
=========
Linux
-----
Install using your system installation utility (yum, yast2, ...)
libqt4-devel, boost-devel, bison, flex 

Copy
http://csgalati.swansea.ac.uk/foam/libs/G3D-20100524.tgz to /usr/local
cd /usr/local
tar xzf G3D-20100524.tgz
mv G3D G3D-20100524
cd G3D-20100524
./buildg3d


Copy http://csgalati.swansea.ac.uk/foam/libs/qwt-5.2.1.tgz to ~
cd ~
qmake
make
make install

Copy http://csgalati.swansea.ac.uk/foam/libs/foam-0.1422.tgz to ~
cd ~
tar xzf foam-0.1422.tgz
cd foam
qmake
make

Running
=======
./foam ~/Documents/swansea-phd/foam/ctrctn dump_0.1520_0.2400_8.0000_0*.dmp

Generating a movie
==================
Check 'Save movie' check box.
Interact with the program
./movie.sh
This will generate a file called 'foamMovie.mp4'


Prerequisites
=============
openSUSe 11.2 (x86_64): G3D-8.00 (cvs version 20100524), 
qt-4.5.3-2.4.2.x86_64, bison-2.3, flex-2.5.35, boost-1.39, gcc-4.4.1
----------------------------------------------------------------------
qmake; make

Windows XP, G3D-7.00, qt-4.5.2, bison-2.3, flex-2.5.35, MSVC 2005 (v8),
Qt4 Add in 1.0.2
----------------------------------------------------------------------
- after you convert  the foam.pro using Qt > Open  Qt Project file run
  ./afterQmake.pl
- after you run bison run ./afterBison.pl

OSX Leopard, G3D-7.01, qt-4.5.2, bison-2.3, flex-2.5.33, gcc-4.2.1
----------------------------------------------------------------------
- chmod u+x buildg3d bin/icompile
- dos2unix buildg3d bin/icompile
- "qmake -spec macx-g++";make

OSX Snow Leopard: G3D-8.00 (cvs version 20100524), 
qt-4.6.2, bison-2.3, flex-2.5.35, boost-1.39, gcc-4.2.1
----------------------------------------------------------------------
- comment out 
- "qmake -spec macx-g++";make
