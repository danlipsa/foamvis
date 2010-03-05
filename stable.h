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
#define YY_NO_UNISTD_H
#endif //_MSC_VER

#ifdef __cplusplus

// Qt headers
#include <Qt>
#include <QtOpenGL>
#include <QApplication>

// G3D headers
#include <G3D/AABox.h>
#include <G3D/Vector3.h>
#include <G3D/Vector3int16.h>
#include <G3D/Vector2.h>
#include <G3D/Rect2D.h>
#include <G3D/Matrix2.h>
#include <G3D/Matrix3.h>
#include <G3D/Matrix4.h>
#include <G3D/CoordinateFrame.h>

// OpenGL G3D headers
#include <GLG3D/getOpenGLState.h>

// standard C headers
#include <cstring>
#include <ctime>
#include <cerrno>

// standard C++ headers
#include <limits>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

// boost TR1 headers
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/unordered_map.hpp>

// boost headers other than TR1
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/if.hpp>

using namespace std;
namespace bl = boost::lambda;

#endif //__cplusplus
