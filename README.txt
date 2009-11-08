To compile foam on:

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


To execute:
