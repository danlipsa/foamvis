#include "DebugStream.h"

#ifdef _MSC_VER

DebugStream cdbg;

#else //_MSC_VER
std::ostream& cdbg = std::cerr;

#endif //_MSC_VER
