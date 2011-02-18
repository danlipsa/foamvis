Compile
=======
Linux
-----
Install using your system installation utility (yum, yast2, ...)
boost-devel, libzip-devel, cmake (flex, bison)

Copy http://csgalati.swansea.ac.uk/foam/build/qt-everywhere-opensource-src-4.7.0.tar.gz to ~
cd ~
tar xzf qt-everywhere-opensource-src-4.7.0.tar.gz
cd qt-everywhere-opensource-src-4.7.0
./configure
make -j <number_of_processors>
su <password>
make install
# this is needed for qt3d
cd /usr/local/Trolltech/Qt-4.7.0; mv include include_
ln -s ~/qt-everywhere-opensource-src-4.7.0/include include

Add the following line at the end of ~/.bash_profile
PATH=/usr/local/Trolltech/Qt-4.7.0/bin:$PATH;export PATH
Then execute '. .bash_profile'. Make sure the path is set correctly: 'echo $PATH'

Everytime you call qmake, make sure that the newly compiled qmake is
called by using 'which qmake' to see the path qmake is called from.


Copy http://csgalati.swansea.ac.uk/foam/build/qt3d-20101008.tgz to ~
cd ~
tar xzf qt3d-20101008.tgz
cd qt3d
qmake
make -j <number_of_processors>
su <password>
make install

Copy
http://csgalati.swansea.ac.uk/foam/build/G3D-8.00-src.zip to ~
cd ~
unzip /G3D-8.00-src.zip
cd source
./buildg3d --install lib
rm -rf source

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
(you can do 'qmake CONFIG+=debug' for a debug build)
make

Make sure you don't run 'make clean' or 'make distclean' as this will
delete bison/flex files and require additional dependencies on the
correct version of bison/flex. If you did that, you have to copy
'*.save' to the corresponding files without '.save' extension or untar
the archive and start again.

Add the following line at the end of ~/.bash_profile
LD_LIBRARY_PATH=/usr/local/qwt-5.2.1/lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
Then execute '. .bash_profile'. Make sure the path is set correctly: 
'echo $LD_LIBRARY_PATH'


Windows
-------
Install cygwin, making sure you select openssh, subversion, perl, bison, flex

Run
===
./foam ~/Documents/swansea-phd/foam/ctrctn/dump_0.1520_0.2400_8.0000_0*.dmp

To skip some files in the simulation (to save memory) use:
~/Documents/swansea-phd/foam/ctrctn/dump_0.1520_0.2400_8.0000_0??[012345678].dmp

loads files 0, 1, ..., 8 and skips 9. You can skip more files by
removing more digits between the square brackets.


Generate a movie
================
Check 'Save movie' check box.
Interact with the program
./movie.sh
This will generate a file called 'foamMovie.mp4'


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

Cleanup memory leaks, ... (valgrind)
====================================
valgrind --suppressions=valgrind-supressions.txt --leak-check=yes
./foam ~/Documents/swansea-phd/foam/coarse100/coarse_01_0100_4309.dmp

You can use the option '--gen-suppressions=yes' to print suppressions
to be added to 'valgrind-suppressions.txt'.



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
- in G3D/GLG3D.lib/source/CarbonWindow.cpp include mycarbon.h
- in G3D/G3D.lib/source/prompt.cpp include mycarbon.h
- in G3D/bin/icompile uncomment the second found line for x86_64 search string
  and comment 
- in G3D/buildg3d replace the two i686 with x86_64
- "qmake -spec macx-g++";make


Release
=======
./test.pl
Set the major version in release.pl
Write an entry in the release log to describe the new features.
doxygen
Commit everything. 
Make sure 'svnversion' reports one version
make distclean;release.pl;svn commit -m "";cd ..;tar czf foam-<version>.tgz foam



Release log
===========
0.4.1746 - use display lists for center paths resulting in significant speed 
	   improvement for interaction with 3D bubble paths.
0.4.1738 - stationary body + context selection in face and average view
	 - fixes to the context view
0.4.1728 - set missing pressure of last body of constrained foam to 0
         - added 2D context view
         - fixed lighting for time displacement view
         - compile with optimizations by default
	 - added selection by location in edge and face views
0.3.1689 - added time displacement view for 2D data
	 - added command line option: -o : show original pressure values
0.3.1682 - bug fix for coarse100/coarse_01_0100_0407.dmp
	 - pressures made > 0 and aligned medians between time steps
	 - added combination brushing: by id and by property value
0.3.1598 - bug fix for pull2_bi_204_00099.dmp, updated test.pl
	 - bug fix assert failure for ctnctr timesteps [89]??

0.3.1596 - bug fixes to the average calculation (fixes for pressure and 
	   velocity).
	 - two new datasets tested ellipse_in_flow and sedimenting_discs
	 - display the center path only if there is at least one segment in focus

0.3.1569 
	- per pixel average for bubble attributes values (2D)

0.2.1489
	- Fixes in the parser and new tests added to test.pl

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
