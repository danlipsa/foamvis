qmake
make


To compile G3D-7.01 on:
openSUSE 11.2 (x86_64)
======================
- add #include <limits.h> to GLG3D.lib/include/GLG3D/Discovery2.h


To compile foam on:

openSUSe 11.2 (x86_64), G3D-7.01, qt-4.5.3-2.4.2.x86_64, bison-2.3, 
flex-2.5.35, gcc-4.4.1
===================================================================
- remove the definition of glDrawRangeElements in GLG3D/glheaders.h at line 181
- after you run bison run ./afterBison.pl

Fedora Core 8, G3D-7.00, qt-4.4.3-1, bison-2.3, flex-2.5.33, gcc-4.1.2
======================================================================
- remove the uint typedef in G3D/g3dmath.h at line 163
- remove the definition of glDrawRangeElements in GLG3D/glheaders.h at line 181
- after you run bison run ./afterBison.pl

Windows XP, G3D-7.00, qt-4.5.2, bison-2.3, flex-2.5.35, MSVC 2005 (v8),
Qt4 Add in 1.0.2
=============================================================================
- after you convert  the foam.pro using Qt > Open  Qt Project file run
  ./afterQmake.pl
- after you run bison run ./afterBison.pl

OSX Leopard, G3D-7.01, qt-4.5.2, bison-2.3, flex-2.5.33, gcc-4.2.1
==================================================================
- chmod u+x buildg3d bin/icompile
- dos2unix buildg3d bin/icompile
- "qmake -spec macx-g++";make
- after you run bison run ./afterBison.pl
