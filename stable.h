/**
 * @file stable.h
 * @author Dan R. Lipsa
 * 
 * Precompiled header
 */
// Add C includes here
#ifdef _MSC_VER
// isatty is defined here
#include <io.h>
#endif //_MSC_VER

#include <limits.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#ifdef __cplusplus

// Add C++ includes here

#if defined(_MSC_VER) || defined(__APPLE__)
// Windows and Mac OS X wants G3D header before everything else
#include <G3D/G3DAll.h>
#endif //_MSC_VER || __APPLE__

#include <Qt>
#include <QtOpenGL>
#include <QApplication>

#include <G3D/G3DAll.h>

#include <algorithm>
#include <stdexcept>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

#endif //__cplusplus
