Compile
=======
Linux
-----
Install using your system installation utility (yum, yast2, ...)
libqt4-devel, boost-devel, bison, flex, libzip-devel, cmake

Copy
http://csgalati.swansea.ac.uk/foam/build/G3D-20100524.tgz to /usr/local
cd /usr/local
tar xzf G3D-20100524.tgz
mv G3D G3D-20100524
cd G3D-20100524
./buildg3d


Copy http://csgalati.swansea.ac.uk/foam/build/qwt-5.2.1.tgz to ~
cd ~
qmake
make
make install

Copy http://csgalati.swansea.ac.uk/foam/build/VTK-5.6.0.tgz to /usr/local
tar xzf VTK-5.6.0.tgz
mkdir VTK-5.6.0-build
cd VTK-5.6.0-build
ccmake ../VTK-5.6.0
press c twice to configure the build and then g to generate the makefile and exit
make -j <number_of_processors> (or just make if you have one processor)
make install


Copy http://csgalati.swansea.ac.uk/foam/build/foam-<version>.tgz to ~
cd ~
tar xzf foam-<version>.tgz
cd foam
qmake
make

Windows
-------
Install cygwin, making sure you select openssh, subversion, perl, bison, flex

Run
===
./foam ~/Documents/swansea-phd/foam/ctrctn dump_0.1520_0.2400_8.0000_0*.dmp

Generate a movie
================
Check 'Save movie' check box.
Interact with the program
./movie.sh
This will generate a file called 'foamMovie.mp4'


Release
=======
./test.pl
Set the major version in release.pl
Commit everything. 
Make sure 'svnversion' reports one version
make distclean;release.pl;cd ..;tar czf foam-<version>.tgz foam

Profile
=======
Install: gprof, gprof2dot, graphviz

Add the following options to foam.pro:
QMAKE_CXXFLAGS += -pg
QMAKE_LFLAGS += -pg

Recompile the program:
make distclean;qmake;make -j 4;

Run the program:
./foam ~/Documents/swansea-phd/foam/ctrctn dump_0.1520_0.2400_8.0000_00*.dmp

Run gprof on the generated file (gmon.out), eventually selecting some
files of interest (to profile only functions from those files):
gprof -pFoamAlongTime.cpp -qFoamAlongTime.cpp ./foam > gprof.txt

Generate a call graph image
cat gprof.txt | gprof2dot.py | dot -Tpng -o gprof.png


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


Release log
===========
0.2.1486
	- Per property color map with clamping
	- Added several color maps: rainbow, black body radiator, 
	  blue-red diverging

0.1.1476
	- Handle significant space between expressions correctly
	- Apply the VIEW_MATRIX rotation, so that foam and evolver produce
	  the same view
        - added a few test files to test.pl

0.1.1462
	- Added UI feedback for histogram bars that are taller than the 
          height of the histogram. (a red cap)
	- Histogram selection is reflected in time step selection,
	  scrolling is done through all time steps not only through
	  selected time steps (context)
	- Added logarithmic scale for histogram
	- Added color coded histograms
	- Histogram selection is reflected in body selection and
	  center path selection. Deselected bodies are displayed
	  transparent in both Face and Center Path views

0.1.1432
	- Added user option to adjust the height of a histogram.
	- Fixed histogram for center path

0.1.1427
	- parsing 2D and 3D dmp files
	- support for torus model
	- support for quadratic model
	- display edges, faces and center paths
	- body attributes supported: volume, pressure, 
	       velocity (magnitude, x,y,z)
	- color faces based on values of attributes
	- color center paths based on values of attributes
	- histogram of values of attributes
	- histogram selection resulting in selection of time steps in the 
	  face view
