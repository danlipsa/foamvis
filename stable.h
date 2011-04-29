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

// standard C headers
#include <cstring>
#include <ctime>
#include <cerrno>
#include <unistd.h>

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
#include <queue>
#include <iomanip>
#include <bitset>

// boost TR1 headers
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/pointer_cast.hpp>

// other BOOST headers
#include <boost/weak_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/operators.hpp>
#include <boost/function.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/if.hpp>
#include <boost/make_shared.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/density.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/math/tools/roots.hpp>

// required by GLG3D
#include <GLG3D/GL/glew.h>

// Qt headers
#include <QApplication>
#include <QtOpenGL>
#include <QtConcurrentMap>
#include <QtDebug>

// Qt3D headers
#include <Qt3D/qglfunctions.h>
#include <Qt3D/qbox3d.h>
#include <Qt3D/qray3d.h>

// QWT headers
#include <qwt_color_map.h>
#include <qwt_interval_data.h>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_item.h> 
#include <qwt_plot_picker.h>
#include <qwt_painter.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_map.h>
#include <qwt_scale_widget.h>
#include <qwt_slider.h>


// G3D headers
#include <G3D/AABox.h>
#include <G3D/CoordinateFrame.h>
#include <G3D/Line.h>
#include <G3D/LineSegment.h>
#include <G3D/Matrix2.h>
#include <G3D/Matrix3.h>
#include <G3D/Matrix4.h>
#include <G3D/Plane.h>
#include <G3D/Rect2D.h>
#include <G3D/Vector2.h>
#include <G3D/Vector3.h>
#include <G3D/Vector3int16.h>

#include <vtkColorTransferFunction.h>
#include <vtkSmartPointer.h>


// OpenGL G3D headers
#include <GLG3D/getOpenGLState.h>

using namespace std;
namespace bl = boost::lambda;
namespace acc = boost::accumulators;
namespace mt = boost::math::tools;
#endif //__cplusplus
