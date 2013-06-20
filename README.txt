Requirements
======================================================================
A graphics card with:
  OpenGL version >= 2.1
  Shading language >= 1.2
  GL_ARB_texture_float

On Linux, run:
'glxinfo' to check capabilities of the drivers and
'lspci | grep VGA' to see the what graphics card you have in your system

Compile
======================================================================
Linux
-----
Install using your system installation utility (yum, yast2, ...)
libzip-devel, cmake (flex, bison).

----------------------------------------------------------------------
Copy http://csgalati.swansea.ac.uk/foam/build/boost_1_45_0.tar.bz2 ~
cd ~
tar xjf boost_1_45_0.tar.bz2
cd boost_1_45_0
sh bootstrap.sh
su <password>
./bjam install --prefix=/usr/local

----------------------------------------------------------------------
Copy http://csgalati.swansea.ac.uk/foam/build/qt-everywhere-opensource-src-4.7.0.tar.gz to ~
cd ~
tar xzf qt-everywhere-opensource-src-4.7.0.tar.gz
cd qt-everywhere-opensource-src-4.7.0
./configure
make -j <number_of_processors>
su <password>
make install
## Needed by qt3d ##
cd /usr/local/Trolltech/Qt-4.7.0; mv include include_
ln -s ~/qt-everywhere-opensource-src-4.7.0/include include
Make sure you type the path correctly: ls include
should list a bunch of files. If it does not: 'rm include' and start again.

Add the following line at the end of ~/.bash_profile
PATH=/usr/local/Trolltech/Qt-4.7.0/bin:$PATH;export PATH
QTDIR=/usr/local/Trolltech/Qt-4.7.0;export QTDIR
Execute '. .bash_profile'.
Make sure the variables are set correctly: 'echo $PATH;echo $QTDIR'
Make sure that the newly compiled qmake is
     called by using 'which qmake' to see the path qmake is called from.

----------------------------------------------------------------------
Copy http://csgalati.swansea.ac.uk/foam/build/qt3d-20101008.tgz to ~
cd ~
tar xzf qt3d-20101008.tgz
cd qt3d
qmake
make -j <number_of_processors>
su <password>
make install

----------------------------------------------------------------------
Copy http://csgalati.swansea.ac.uk/foam/build/qwt-5.2.1.tgz to ~
cd ~
qmake
make
make install

----------------------------------------------------------------------
Copy
http://csgalati.swansea.ac.uk/foam/build/G3D-8.00-src.zip to ~
cd ~
unzip /G3D-8.00-src.zip
cd source
./buildg3d --install lib
rm -rf source
----------------------------------------------------------------------
## Needed by VTK ##
Copy http://csgalati.swansea.ac.uk/foam/build/cmake-2.8.10.2.tar.gz to ~
cd ~
tar xzf cmake-2.8.10.2.tar.gz
cd cmake-2.8.10.2
./bootstrap
make
make install
Add the following line at the end of ~/.bash_profile
PATH=/usr/local/bin:$PATH;export PATH
Execute '. .bash_profile'.
Make sure the path is set correctly: which cmake
----------------------------------------------------------------------
Copy http://csgalati.swansea.ac.uk/foam/build/vtk-6.0-20130125.tgz to ~
tar xzf vtk-6.0-20130125.tgz
mkdir VTK-build
cd VTK-build
ccmake ../VTK
Set the configuration as in the following figure

 BUILD_DOCUMENTATION              OFF
 BUILD_EXAMPLES                   ON
 BUILD_SHARED_LIBS                ON
 BUILD_TESTING                    ON
 CMAKE_BUILD_TYPE                 Debug
 CMAKE_INSTALL_PREFIX             /usr/local
 VTK_EXTRA_COMPILER_WARNINGS      OFF
 VTK_Group_Imaging                ON
 VTK_Group_MPI                    OFF
 VTK_Group_Qt                     ON
 VTK_Group_Rendering              ON
 VTK_Group_StandAlone             ON
 VTK_Group_Tk                     ON
 VTK_Group_Views                  ON
 VTK_USE_TK                       ON
 VTK_WRAP_JAVA                    OFF
 VTK_WRAP_PYTHON                  ON
 VTK_WRAP_TCL                     ON

press c twice to configure the build and then g to generate the makefile and exit
make -j <number_of_processors> (or just make if you have one processor)
make install

----------------------------------------------------------------------
Copy http://csgalati.swansea.ac.uk/foam/build/gsl-1.15.tar.gz to ~
tar xzf gsl-1.15.tar.gz
cd gsl-1.15
./configure
make
su <password>
make install

----------------------------------------------------------------------
Copy http://csgalati.swansea.ac.uk/foam/build/foam-<version>.tgz to ~
cd ~
tar xzf foam-<version>.tgz
cd foam
qmake
make
(you can do 'make debug' for a debug build)


Make sure you don't run 'make clean' or 'make distclean' as this will
delete bison/flex files and require additional dependencies on the
correct version of bison/flex. If you did that, you have to copy
'*.save' files to the corresponding files without '.save' extension or untar
the archive and start again.

Add the following line at the end of ~/.bash_profile
LD_LIBRARY_PATH=/usr/local/qwt-5.2.1/lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
Then execute '. .bash_profile'. Make sure the path is set correctly: 
'echo $LD_LIBRARY_PATH'

For development only:
cp $VTK_BINARY_DIR/bin/libQVTKWidgetPlugin.so $QTDIR/plugins/designer


Mac OS X Snow Leopard
---------------------
Install XCode 3.2.6 (search for 'Xcode snow leopard')


Windows - not up to date.
-------------------------
Install cygwin, making sure you select openssh, subversion, perl, bison, flex

Run
======================================================================
./foam ~/Documents/swansea-phd/foam/ctrctn/dump_0.1520_0.2400_8.0000_0*.dmp

To skip some files in the simulation (to save memory) use:
~/Documents/swansea-phd/foam/ctrctn/dump_0.1520_0.2400_8.0000_0??[012345678].dmp

loads files 0, 1, ..., 8 and skips 9. You can skip more files by
removing more digits between the square brackets.

Execute ./foam without any parameters to see a list of options.

Command lines for various datasets:
-----------------------------------
See simulations.ini.

Generate a movie
======================================================================
Check 'Save movie' check box.
Interact with the program
./movie.sh
This will generate a file called 'foamMovie.mp4'

Debug - Profile (gprof)
======================================================================
Install: gprof

Add the following options to foam.pro:
QMAKE_CXXFLAGS += -pg
QMAKE_LFLAGS += -pg

Recompile the program:
make distclean;qmake;make -j 4;

Run the program:
./foam ~/Documents/swansea-phd/foam/ctrctndumps_704v_0.1520_0.2400_8.0000_v1 dump_0.1520_0.2400_8.0000_00*.dmp

Run gprof on the generated file (gmon.out), eventually selecting some
files of interest (to profile only functions from those files):
gprof ./foam_debug > gprof.txt
or
gprof -pFoamAlongTime.cpp -qFoamAlongTime.cpp ./foam_debug > gprof.txt
-p generates a flat profile
-q generates a call graph
See gprof.info for information on gprof.txt.

Debug - static checks (cppcheck)
======================================================================
cppcheck-gui will use the project file foamvis.cppcheck and 


Debug - Memory leaks, ... (valgrind)
======================================================================
valgrind --suppressions=valgrind-supressions.txt --leak-check=yes
./foam ~/Documents/swansea-phd/foam/coarse100/coarse_01_0100_4309.dmp

You can use the option '--gen-suppressions=yes' to print suppressions
to be added to 'valgrind-suppressions.txt'.

Debug - OpenGL (bugle, gldb-gui)
======================================================================
gldb-gui

Debug - Parser, Scanner
======================================================================
./foam --debug-parsing 2>error.txt
./foam --debug-scanning 2>error.txt
See error.txt, Evolver.y, Evolver.l and EvolverData.output for 
information on the parser and scanner

Debug - STL error messages
======================================================================
Copy the offending compilation command and replace 'g++' with 'gfilt'. 
It uses STLFilt tool that simplifies STL error messages.

Prerequisites
======================================================================
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

Generate documentation
======================================================================
// To modify documentation generation settings run:
doxywizard Doxyfile

// To generate the documentation run:
doxygen




Release
======================================================================
./test.pl
Set the major version in release.pl
svn update
svnversion
Write an entry in the release log to describe the new features (version += 2)
Commit everything.
Make sure 'svnversion' reports one version
make distclean;release.pl;svn commit -m "";
doxygen
cd ..;tar czf foam-<version>.tgz foam
cd xxx-private/src/
   tar czf xxx-ini-<version>.tgz xxx xxx.ini
