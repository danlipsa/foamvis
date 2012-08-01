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

// GSL headers
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multiroots.h>
#include <gsl/gsl_eigen.h>

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
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/tokenizer.hpp>
#include <boost/math/special_functions/fpclassify.hpp>


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

// VTK headers
#define VTK_EXCLUDE_STRSTREAM_HEADERS
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkCellDataToPointData.h>
#include <vtkColorTransferFunction.h>
#include <vtkCommand.h>
#include <vtkDataSetMapper.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkIndent.h>
#include <vtkInteractorStyle.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProbeFilter.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkSmartPointer.h>
#include <vtkShrinkFilter.h>
#include <vtkTetra.h>
#include <vtkTDxInteractorStyleCamera.h>
#include <vtkTDxInteractorStyleSettings.h>
#include <vtkThreshold.h>
#include <vtkTriangle.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLImageDataReader.h>
#include <QVTKWidget.h>
#include <QVTKApplication.h>

// OpenGL G3D headers
#include <GLG3D/getOpenGLState.h>

using namespace std;
namespace bl = boost::lambda;
namespace acc = boost::accumulators;
namespace po = boost::program_options;
namespace bm = boost::math;
#endif //__cplusplus
