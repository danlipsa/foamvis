HEADERS += Application.h ApproximationEdge.h \
	Attribute.h AttributeCreator.h AttributeInfo.h \
	AttributeHistogram.h Average.h AverageInterface.h\
	AverageShaders.h Body.h \
	BodyAlongTime.h AdjacentBody.h BodySelector.h ConstraintRotation.h \
	ConstraintEdge.h ColorBarModel.h Comparisons.h Debug.h \
	DebugStream.h Disk.h ImageBasedAverage.h ForceAverage.h\
	DisplayBodyFunctors.h DisplayFaceFunctors.h DisplayVertexFunctors.h \
	DisplayEdgeFunctors.h DisplayElement.h DisplayWidget.h\
	SelectBodiesById.h EditColorMap.h Edge.h Element.h ExpressionTree.h \
	Enums.h Foam.h FoamAlongTime.h Face.h Force.h\
	GLWidget.h Hashes.h Histogram.h HistogramItem.h HistogramHeight.h\
	HistogramStatistics.h \
	LineEditFocus.h MainWindow.h NameSemanticValue.h \
	OOBox.h Info.h OpenGLUtils.h OrientedElement.h\
	OrientedEdge.h OrientedFace.h AdjacentOrientedFace.h \
	ParsingData.h ParsingDriver.h ScalarAverage.h ShaderProgram.h\
	ParsingEnums.h ProcessBodyTorus.h PropertySetter.h \
	QuadraticEdge.h RestrictedRangeSlider.h \
	stable.h StripIterator.h SystemDifferences.h \
	TensorAverage.h TransferFunctionHistogram.h \
	TimeStepsSlider.h Utils.h \
	Vertex.h ViewSettings.h
SOURCES += Application.cpp ApproximationEdge.cpp\
	Attribute.cpp AttributeCreator.cpp AttributeInfo.cpp \
	AttributeHistogram.cpp Average.cpp AverageShaders.cpp \
	Body.cpp BodyAlongTime.cpp AdjacentBody.cpp BodySelector.cpp\
	ConstraintEdge.cpp ColorBarModel.cpp Comparisons.cpp \
	DebugStream.cpp Disk.cpp DisplayBodyFunctors.cpp \
	ImageBasedAverage.cpp DisplayFaceFunctors.cpp \
	DisplayEdgeFunctors.cpp	\
	DisplayWidget.cpp DisplayVertexFunctors.cpp Edge.cpp \
	HistogramStatistics.cpp\
	SelectBodiesById.cpp EditColorMap.cpp Element.cpp ExpressionTree.cpp \
	Enums.cpp Foam.cpp FoamAlongTime.cpp Face.cpp ForceAverage.cpp\
	GLWidget.cpp Hashes.cpp Histogram.cpp HistogramItem.cpp\
	HistogramHeight.cpp main.cpp MainWindow.cpp  \
	NameSemanticValue.cpp \
	OOBox.cpp OpenGLUtils.cpp OrientedElement.cpp\
	OrientedEdge.cpp OrientedFace.cpp AdjacentOrientedFace.cpp\
	ScalarAverage.cpp ParsingData.cpp ParsingDriver.cpp \
	ParsingEnums.cpp ProcessBodyTorus.cpp \
	PropertySetter.cpp ShaderProgram.cpp\
	QuadraticEdge.cpp RestrictedRangeSlider.cpp\
	StripIterator.cpp TimeStepsSlider.cpp \
	TransferFunctionHistogram.cpp TensorAverage.cpp \
	Utils.cpp Vertex.cpp ViewSettings.cpp
FORMS += SelectBodiesById.ui EditColorMap.ui HistogramHeight.ui \
	 RestrictedRangeSlider.ui MainWindow.ui Info.ui
LEXSOURCES        += EvolverData.l
YACCSOURCES       += EvolverData.y
RESOURCES          = foam.qrc
QMAKE_YACC         = .\\/bison.pl -y --report=state --debug
QMAKE_LEX          = .\\/flex.pl
TARGET             = foam
QT                += opengl
PRECOMPILED_HEADER = stable.h
CONFIG            += qt precompile_header no_keywords debug_and_release
CONFIG(debug, debug|release) {
	TARGET = $$join(TARGET,,,_debug)
}

win32 {
INCLUDEPATH += "C:\G3D-7.00-vc8\include"
LIBS += "C:\G3D-7.00-vc8\lib\zlib.lib" 
LIBS += "C:\G3D-7.00-vc8\lib\png.lib" 
LIBS += "C:\G3D-7.00-vc8\lib\jpeg.lib" 
LIBS += "C:\G3D-7.00-vc8\lib\zip.lib" 
LIBS += "C:\G3D-7.00-vc8\lib\G3Dd.lib" 
LIBS += "C:\G3D-7.00-vc8\lib\GLG3Dd.lib" 
 }
else:macx {
#QMAKE_CXXFLAGS += -arch i686 -msse3 -mfpmath=sse
#QMAKE_LFLAGS += -arch i686

INCLUDEPATH += /usr/local/include
INCLUDEPATH += /usr/local/qwt-5.2.1/include
INCLUDEPATH += /usr/local/include/vtk-5.6
INCLUDEPATH += /usr/local/Trolltech/Qt-4.7.0/include

LIBS += -L/usr/local/lib
LIBS += -L/usr/local/qwt-5.2.1/lib
LIBS += -L/usr/local/lib/vtk-5.6
LIBS += -L/usr/local/Trolltech/Qt-4.7.0/lib

LIBS += "-lGLG3D" 
LIBS += "-lG3D" 
LIBS += "-ljpeg"
LIBS += "-lz"
LIBS += "-lzip"
LIBS += "-lavformat"
LIBS += "-lavcodec"
LIBS += "-lavutil"
LIBS += "-framework Carbon"
LIBS += "-lqwtd"
LIBS += "-lvtkFiltering"
LIBS += "-lvtkCommon"
LIBS += "-lvtksys"
LIBS += "-lQt3D"
LIBS += "-lgslcblas"
LIBS += "-lgsl"
LIBS += "-lboost_program_options"
}
else:unix {
INCLUDEPATH += /usr/local/include
INCLUDEPATH += /usr/local/include/vtk-5.6
INCLUDEPATH += /usr/local/qwt-5.2.1/include
INCLUDEPATH += /usr/local/Trolltech/Qt-4.7.0/include

LIBS += -L/usr/local/lib
LIBS += -L/usr/local/qwt-5.2.1/lib
LIBS += -L/usr/local/lib/vtk-5.6
LIBS += -L/usr/local/Trolltech/Qt-4.7.0/lib

LIBS += "-lGLG3D"
LIBS += "-lG3D"
LIBS += "-lzip"
LIBS += "-lSDL"
LIBS += "-ljpeg"
LIBS += "-lavformat"
LIBS += "-lavcodec"
LIBS += "-lavutil"
LIBS += "-lqwtd"
LIBS += "-lz"
LIBS += "-lpng12"
LIBS += "-lX11"
LIBS += "-lvtkFiltering"
LIBS += "-lvtkCommon"
LIBS += "-lvtksys"
LIBS += "-ldl"
LIBS += "-lQt3D"
LIBS += "-lgslcblas"
LIBS += "-lgsl"
LIBS += "-lboost_program_options"
}

# Local Variables:
# mode: makefile
# End:
