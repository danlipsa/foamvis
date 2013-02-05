Release log
===========
0.8.
        - bug fix: next step did not update the OpenGL window when both 
          VTK and OpenGL windows are displayed.
        - bug fix: when set average vis the program segfaults
        - bug fix: sometimes copy color bar creates a clamping that was 
          not in the source
        - added Copy / Selection to the VTK view        
        - adjust context alpha per view (before it was done for all views)
        - bug fix: copy selection crashes if the histogram was never open
0.8.2699
        - bug fix: transformations are slow
        - allow keywords inside SCALAR_INTEGRANT (for shearv_35)
        - implement simple form of view transform
        - read t1s from the DMP file using t1positions and num_pops_step
        - improved constraint processing speed by avoiding string lookups at 
          each access of a variable.
        - fix regression: deselected objects are not transparent
        - save cache files in ~/.foamvis instead of the original data folder.
        - warning message in automatically generaged fragment shaders
        - add color mapped streamlines (colored by velocity magnitude)
        - allow color map copy between T1s KDE with the same sigma 
          for different simulations.
        - added user option 'time interval' for bubble paths
        - Remove user options for T1sKDE: 
          'Texture Size', 'Interval / Pixel' and set them automatically 
          based on 'Sigma'. Sigma is expressed in terms of bubble size.
        - show streamlines and KDE together and seed based on KDE value
        - show  min, max and clamping values for the color map
        - extend the linked time option with multiple events
        - bug fix: scalar selection shown on the time bar
        - update to vtk-6.0-20121121.tgz (for pathlines).
0.8.2553 2012-09-07
        - bug fixes (dataset fluctuates because is centered at foam center 
          instead of simulation center)
0.8.2552 2012-09-06   
	- parse and process arrays (for 8balls_5)
	- allow bubbles to appear in the simulation (for allcrypt2) and 
	  allow bodies without any attributes (for allcrypt2)
	- user option: ability to remove tessellation edges for "face view".
	- user option: rotate around x,y,z axes using ctrl, shift, and ctrl+shift
	- default settings: no edges shown, turn on lighting for 3D simulations
	- fix transparency: transparent objects blend with opaque ones.
	- fix 3D selection: deselection works front-back, selection works 
	  back-front.
	- Added "Rotation center" user option to allow rotation around a body
	- Fixed regression bug where the torus domain is stored per
          simulation instead of storing it per foam.
	- update to vtk-5.10.0 libraries, built dynamically
	- fix "scanner overflow" error (for 1000.dmp) by reporting only 
	  overflows not underflows.
	- Add average/multiple views for 3D data (using VTK)
	- Improve interaction speed for 'Visualization / Time Step / Faces' 
	  using OpenGL display lists. 
	  (stokeslibre_22_04_17_0003_0001_bodies.dmp.dmp.dmp)
	- Added --resolution <r> to specify the resolution of the regular grid 
	  saved (.vti) for 3D averages, 0 means no regular grid saved.
	- Fix: translate pressure only if there are no free surfaces.
	- A histogram per view can be displayed.
0.7.2374 2012-03-09
	- fix bug: position of the two_discs for two views.
	- added arrows for forces
	- use highlight0 for standalone edges in kernel density estimate.
	- link the grid size (and position) for reflected half view.
	- reflect the ellipse view so that it falls in the same side as the 
	  two discs.
	- added sequential color maps based on colorbrewer blues 9, ...
	- reorganize UI
	- Add option for clamping marker for velocity vectors
	- clamping for velocity vector is done the same as for scalar attributes.
	- added Same Size and Color Mapped options for velocity vectors
	- sync "Colored by" for reflected half view.
	- read torque from DMP files
	- add diff option for forces overlay
	- activate T1s KDE around moving objects
0.7.2270 2011-11-30
	- fixed bug where deformation ellipses are too small for
          datasets containing constraint objects.
	- added an "overlay bar" that shows clamping for velocity overlay.
	- improved appearance for velocity arrows and show which arrows are 
	  clamped.
0.7.2257 2011-11-28
	- added focus rectangle for the current view
	- alow _ and . for labels inside the ini file
	- fix to allow reading forces on a constraint object
	- deformation and velocity are clamped when increasing size (instead of 
	  disapearing from view)
	- use an arrow for velocity vector.
0.7.2231 2011-11-15
	- fixed selection for timesteps other than 0.
	- added average velocity vector	overlay
	- added linked transform horizontal axis: opposite halves from each view 
	  are shown.
0.7.2201 2011-10-21
	- several simulations can be loaded to facilitate comparison. To do 
	  this, several --simulation switches or the browse 
	  simulations dialog can be used.
	- added two time linkage modes between multiple views: 
	  Linked and Independent. Can be accessed with in the 
	  'General > Show > View' panel.	
	- added --rotation-2d to specify a rotated dataset (Transform > 
	  Axes Order > '2D rotated right 90' and '2D rotated left 90'.
        - added slice_coeff keword to the parser
	- added edge_length and edge_tension Evolver methods to the parser.
	- added method_instance.value construct to the parser.
0.6.2166 2011-09-28
	 - added T1s probability density estimate
0.6.2148 2011-09-16
	 - added --ticks-for-timestep to deal with vfplug_137v_9.857973_2.
	 - Rearanged the views in 'Time step' and 'Time dependent' categories 
	   (instead of Edges, Faces and Bodies).
	 - Fixed Average Around for ellipse_sed_rotate simulation (enabled both 
	   rotation and translation)
	 - Replaced 'Context > Stationary foam' with 
	            'Average around > Allow rotation'.         
         - Solved bug that produced wrong deformation tensor for Average Around.
	 - Added Average around > Second body to compute average around two 
	   bodies.

0.6.2118 2011-08-26
	 - added instantaneous and time averaged deformation (texture) tensor.
	 - fixed 'Context > Stationary > Foam' to not cut the displayed image.
	 - added scalar value: deformation eigen (1 - l_min/l_max)
	 - added the release date to the version information. (when you execute
           foamvis without any parameters)
	 - change the 2D number of sides information to use the number 
	   of physical vertices (fixes vfplug_137v_9.857973_2) instead of
	   the number of physical vertices.
	 - allow a face made of two quadratic edges (fixes 
	   ctrctndumps_725v_0.1480_0.2400_9.0000_rupt_v1/
	   dump_0.1480_0.2400_9.0000_0171.dmp)
	 - reading command parameters from ini file + browsing simulations

0.5.2062 2011-07-11
	 - added axes labels (Show > Ohter > Axes)
	 - added right-click > Info > Face and Info > Edge
	 - added --constraint option, to specify an object described
           with a constraint in the dmp file (for sedimenting_ellipse_odd).
         -if a bubble edge is on a constraint, The bubble neighbor for that
           edge is determined through "reflection". See previous email from
           12/07/11.
	 - deduce an object specified with a constraint (for sedimenting-discs)
	 - added Show > Other > Body neighbors checkbox
	 - allow CR,LF as line termination (for two_discs_c1_n2200_dumps)

0.5.2023 2011-07-04
	 - fixed wetfoam_100_0002.dmp
	 - added Actual Volume to the list of body properties, 
	   Added 'General > Look > Volume shown' user option.
	 - Set selections 'per view'. Added 'Copy Selection' right-click menu
	 - Added 'Play Reverse' button

0.5.1974 2011-06-10
	 - read forces acting on sedimenting discs using --force command line

0.5.1960 2011-06-01
	 - use long command line options (boost program options)
	 - use --use-original for datasets that have to use ORIGINAL 
	   attribute (ellipse)
	 - added --constraint-rotation option to read rotation of the 
	   ellipse from the DMP file
	 - Added boost-1.45 in the list of libraries.
	 - body bounding box is displayed only if body is selected
	 - reverse works for statistics (it restores the statistics at 
	   previous timesteps)
	 - use the stencil buffer to display concave polygons (fixes display 
	   for some bubbles in the ellipse datase)
	 - fix sausage_problem
	 - added right-click > Context > Stationary > Foam

0.5.1928 2011-05-16
	 - bug for tmp_in.dmp (array on several lines)
	 - changed body ID to the one specified in ORIGINAL attribute 
	   (fixes ell0.700000_750v_5.000000_1.dmp) only for 2D. Otherwise
	   it breaks sphereall/sphere_0.711204_144_0001.dmp
	 - added missing body edges from constraints specification

0.4.1852 
	 - linked views
	 - edit highlight colors for colormaps
	 - separated body context from body stationary and displayed both
           regardless of focus
         - more precise center calculation (using all aproximation points for a
	   quadratic edge)
	 - user option: show missing pressure as 0

0.4.1794 
	 - added option to shift T1s lower

0.4.1789 
	 - new scalar variables Number of sides, Elongation
	 - added T1s overlay (-r option on the command line)
	 - added 2 highlight colors for each color map

0.4.1777 
	 - rolling average
	 - domain histogram (count of values that fall in a certain interval per
	   unit area). Note that histogram shown for 'Edit Color Map' is not 
	   correct for the 'Domain Histogram' color bar.
	 - fix the front-back faces
         - fix the concave polygons problem.

0.4.1746 
	 - use display lists for center paths resulting in significant speed 
	   improvement for interaction with 3D bubble paths.

0.4.1740 
	 - stationary body + context selection in face and average view
	 - fixes to the context view

0.4.1728 
	 - set missing pressure of last body of constrained foam to 0
         - added 2D context view
         - fixed lighting for time displacement view
         - compile with optimizations by default
	 - added selection by location in edge and face views

0.3.1690 
	 - added time displacement view for 2D data
	 - added command line option: -o : show original pressure values

0.3.1682 
	 - bug fix for coarse100/coarse_01_0100_0407.dmp
	 - pressures made > 0 and aligned medians between time steps
	 - added combination brushing: by id and by property value

0.3.1598 
	 - bug fix for pull2_bi_204_00099.dmp, updated test.pl
	 - bug fix assert failure for ctnctr timesteps [89]??

0.3.1596 
	 - bug fixes to the average calculation (fixes for pressure and 
	   velocity).
	 - two new datasets tested ellipse_in_flow and sedimenting_discs
	 - display the center path only if there is at least one segment in focus

0.3.1569 
	 - per pixel average for bubble attributes values (2D)

0.2.1489 
	 - Fixes in the parser and new tests added to test.pl

0.2.1487 
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

Requirements
============
A graphics card with:
  OpenGL version >= 2.1
  Shading language >= 1.2
  GL_ARB_texture_float
On Linux, run:
'glxinfo' to check capabilities of the drivers and
'lspci | grep VGA' to see the what graphics card you have in your system

Compile
=======
Linux
-----
Install using your system installation utility (yum, yast2, ...)
libzip-devel, cmake (flex, bison)

Copy http://csgalati.swansea.ac.uk/foam/build/boost_1_45_0.tar.bz2 ~
cd ~
tar xjf boost_1_45_0.tar.bz2
cd boost_1_45_0
sh bootstrap.sh
su <password>
./bjam install --prefix=/usr/local

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
QTDIR=/usr/local/Trolltech/Qt-4.7.0;export QTDIR
Execute '. .bash_profile'.
Make sure the variables are set correctly: 'echo $PATH;echo $QTDIR'
Make sure that the newly compiled qmake is
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


Copy http://csgalati.swansea.ac.uk/foam/build/gsl-1.15.tar.gz to ~
tar xzf gsl-1.15.tar.gz
cd gsl-1.15
./configure
make
su <password>
make install


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
'*.save' to the corresponding files without '.save' extension or untar
the archive and start again.

Add the following line at the end of ~/.bash_profile
LD_LIBRARY_PATH=/usr/local/qwt-5.2.1/lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
Then execute '. .bash_profile'. Make sure the path is set correctly: 
'echo $LD_LIBRARY_PATH'

Development only:
cp $VTK_BINARY_DIR/bin/libQVTKWidgetPlugin.so $QTDIR/plugins/designer


Mac OS X Snow Leopard
---------------------
Install XCode 3.2.6 (search for 'Xcode snow leopard')


Windows - not up to date.
-------
Install cygwin, making sure you select openssh, subversion, perl, bison, flex

Run
===
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
================
Check 'Save movie' check box.
Interact with the program
./movie.sh
This will generate a file called 'foamMovie.mp4'


Debug - Profile (gprof)
=======================
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
================================
cppcheck-gui will use the project file foamvis.cppcheck and 


Debug - Memory leaks, ... (valgrind)
====================================
valgrind --suppressions=valgrind-supressions.txt --leak-check=yes
./foam ~/Documents/swansea-phd/foam/coarse100/coarse_01_0100_4309.dmp

You can use the option '--gen-suppressions=yes' to print suppressions
to be added to 'valgrind-suppressions.txt'.

Debug - OpenGL (bugle, gldb-gui)
==============================
gldb-gui

Debug - Parser, Scanner
=======================
./foam --debug-parsing
./foam --debug-scanning
See Evolver.y, Evolver.l and EvolverData.output for information on the parser or
scanner

Debug - STL error messages
==========================
Copy the offending compilation command and replace 'g++' with 'gfilt'. 
It uses STLFilt tool that simplifies STL error messages.

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
Write an entry in the release log to describe the new features (version += 2)
Commit everything.
Make sure 'svnversion' reports one version
make distclean;release.pl;svn commit -m "";
doxygen
cd ..;tar czf foam-<version>.tgz foam
cd xxx-private/src/
tar czf xxx-ini-<version>.tgz xxx xxx.ini
