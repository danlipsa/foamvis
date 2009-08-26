#include "DebugStream.h"

#ifdef _MSC_VER

DebugStream cdbg;

#else //_MSC_VER
ostream& cdbg = cerr;

#endif //_MSC_VER
