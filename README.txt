To compile foam on:

Fedora Core 8, G3D-7.00, qt-4.4.3-1, bison-2.3, flex-2.5.33, gcc-4.1.2
===================================
- remove the uint typedef in G3D/g3dmath.h at line 163
- remove the definition of glDrawRangeElements in GLG3D/glheaders.h at line 181
- after you run bison run ./afterBison.pl

