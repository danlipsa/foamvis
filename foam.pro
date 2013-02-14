HEADERS += Application.h ApproximationEdge.h AdjacentOrientedFace.h \
	Attribute.h AttributeCreator.h AttributeInfo.h \
	AttributeHistogram.h Average.h AverageInterface.h\
	AverageShaders.h PipelineAverage3D.h \
	Base.h Body.h BrowseSimulations.h\
	BodyAlongTime.h AdjacentBody.h BodySelector.h \
	ConstraintEdge.h ColorBarModel.h Comparisons.h\
	Debug.h \
	DebugStream.h Disk.h ImageBasedAverage.h ForceAverage.h\
	DisplayBodyFunctors.h DisplayElement.h\
	DisplayFaceFunctors.h DisplayVertexFunctors.h \
	DisplayEdgeFunctors.h DisplayElement.h WidgetSave.h\
	EditColorMap.h Edge.h Element.h ExpressionTree.h \
	Enums.h Foam.h FoamvisInteractorStyle.h \
	DataProperties.h Face.h ForceOneObject.h\
	WidgetBase.h WidgetGl.h WidgetHistogram.h WidgetVtk.h \
	Hashes.h Histogram.h HistogramItem.h HistogramSettings.h\
	HistogramStatistics.h Labels.h ListViewChange.h\
	LineEditFocus.h MainWindow.h NameSemanticValue.h \
	OOBox.h Info.h ObjectPosition.h OpenGLUtils.h OrientedElement.h\
	OrientedEdge.h OrientedFace.h Options.h \
	ParsingData.h ParsingDriver.h \
	Settings.h SelectBodiesById.h ScalarAverage.h ShaderProgram.h\
	ParsingEnums.h PipelineBase.h \
	ProcessBodyTorus.h PropertySetter.h \
	QuadraticEdge.h RegularGridAverage.h\
	RestrictedRangeSlider.h Simulation.h\
	stable.h StripIterator.h SystemDifferences.h ScalarDisplay.h \
	T1sKDE.h TensorAverage.h TransferFunctionHistogram.h \
	TimeStepsSlider.h Utils.h VectorAverage.h \
	Vertex.h AttributeAverages.h \
	AttributeAverages2D.h AttributeAverages3D.h ViewSettings.h
SOURCES += Application.cpp ApproximationEdge.cpp\
	Attribute.cpp AttributeCreator.cpp AttributeInfo.cpp \
	AttributeHistogram.cpp Average.cpp AverageShaders.cpp \
	AdjacentBody.cpp PipelineAverage3D.cpp \
	Base.cpp Body.cpp BodyAlongTime.cpp \
	BodySelector.cpp BrowseSimulations.cpp \
	ConstraintEdge.cpp ColorBarModel.cpp Comparisons.cpp \
	DebugStream.cpp Disk.cpp DisplayBodyFunctors.cpp DisplayElement.cpp\
	ImageBasedAverage.cpp DisplayFaceFunctors.cpp \
	DisplayEdgeFunctors.cpp	\
	DisplayVertexFunctors.cpp Edge.cpp \
	HistogramStatistics.cpp\
	EditColorMap.cpp Element.cpp ExpressionTree.cpp \
	Enums.cpp Foam.cpp FoamvisInteractorStyle.cpp\
	Face.cpp DataProperties.cpp ForceOneObject.cpp\
	ForceAverage.cpp \
	WidgetBase.cpp WidgetGl.cpp WidgetHistogram.cpp \
	WidgetSave.cpp WidgetVtk.cpp \
	Hashes.cpp Histogram.cpp HistogramItem.cpp \
	HistogramSettings.cpp main.cpp MainWindow.cpp  \
	NameSemanticValue.cpp \
	OOBox.cpp OpenGLUtils.cpp OrientedElement.cpp Options.cpp\
	OrientedEdge.cpp OrientedFace.cpp AdjacentOrientedFace.cpp\
	ScalarAverage.cpp Settings.cpp SelectBodiesById.cpp \
	ParsingData.cpp PipelineBase.cpp ParsingDriver.cpp \
	ParsingEnums.cpp ProcessBodyTorus.cpp \
	PropertySetter.cpp ShaderProgram.cpp\
	QuadraticEdge.cpp RegularGridAverage.cpp\
	RestrictedRangeSlider.cpp Simulation.cpp\
	StripIterator.cpp ScalarDisplay.cpp T1sKDE.cpp TimeStepsSlider.cpp \
	TransferFunctionHistogram.cpp TensorAverage.cpp \
	Utils.cpp VectorAverage.cpp Vertex.cpp \
	AttributeAverages.cpp \
	AttributeAverages2D.cpp AttributeAverages3D.cpp ViewSettings.cpp
FORMS += BrowseSimulations.ui SelectBodiesById.ui EditColorMap.ui \
	 HistogramSettings.ui RestrictedRangeSlider.ui MainWindow.ui Info.ui
LEXSOURCES        += EvolverData.l
YACCSOURCES       += EvolverData.y
RESOURCES          = foam.qrc
QMAKE_YACC         = .\\/bison.pl -y --report=state --debug
QMAKE_LEX          = .\\/flex.pl
TARGET             = foam
QT                += opengl
PRECOMPILED_HEADER = stable.h
CPP                = /usr/bin/cpp
SED                = /usr/bin/sed

GLSL_PREPROCESS_FILES = TensorDisplay.frag.in VectorDisplay.frag.in
glsl_preprocess.input = GLSL_PREPROCESS_FILES
glsl_preprocess.output = ${QMAKE_FILE_BASE}
glsl_preprocess.depend_command = $${CPP} -E -M ${QMAKE_FILE_IN} |\
	 $${SED} \'s,^.*: ,,\'
glsl_preprocess.commands = \
	cat ${QMAKE_FILE_IN} | cpp -C -P - - | \
	.\\/cleanupFrag.pl > ${QMAKE_FILE_BASE}
glsl_preprocess.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += glsl_preprocess
PRE_TARGETDEPS += VectorDisplay.frag TensorDisplay.frag
# create old empty files that will be updated by the build process
# to avoid warning from the qmake
system (touch -t 0001010000 $$PRE_TARGETDEPS)

CONFIG            += qt precompile_header no_keywords debug_and_release
CONFIG(debug, debug|release) {
	TARGET = $$join(TARGET,,,_debug)
# Profile
# QMAKE_CXXFLAGS += -pg
# QMAKE_LFLAGS += -pg
DEFINES += vtkRenderingCore_AUTOINIT=\"4(vtkInteractionStyle,vtkRenderingFreeType,vtkRenderingFreeTypeOpenGL,vtkRenderingOpenGL)\"
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
INCLUDEPATH += /usr/local/include
INCLUDEPATH += /usr/local/qwt-5.2.1/include
INCLUDEPATH += /usr/local/include/vtk-6.0
INCLUDEPATH += /usr/local/Trolltech/Qt-4.7.0/include

LIBS += -L/usr/local/lib
LIBS += -L/usr/local/qwt-5.2.1/lib

# G3D
LIBS += "-lGLG3D" 
LIBS += "-lG3D" 

LIBS += "-ljpeg"
LIBS += "-lz"
LIBS += "-lzip"
LIBS += "-lavformat"
LIBS += "-lavcodec"
LIBS += "-lavutil"
LIBS += "-framework Carbon"

# Qwt
LIBS += "-lqwtd"

# VTK
LIBS += "-lvtkGUISupportQt-6.0"
LIBS += "-lvtkIOXML-6.0"
LIBS += "-lvtkFiltersFlowPaths-6.0"
LIBS += "-lvtkInteractionWidgets-6.0"
LIBS += "-lvtkRenderingOpenGL-6.0"
LIBS += "-lvtkRenderingFreeTypeOpenGL-6.0"
LIBS += "-lvtkRenderingCore-6.0"
LIBS += "-lvtkCommonMath-6.0"
LIBS += "-lvtkCommonCore-6.0"
LIBS += "-lvtkCommonDataModel-6.0"
LIBS += "-lvtkCommonExecutionModel-6.0"
LIBS += "-lvtkInteractionStyle-6.0"
LIBS += "-lvtkFiltersCore-6.0"
LIBS += "-lvtkRenderingAnnotation-6.0"

# Qt3D
LIBS += "-lQt3D"

# Gsl
LIBS += "-lgslcblas"
LIBS += "-lgsl"

# Boost
LIBS += "-lboost_program_options"
}
else:unix {
INCLUDEPATH += /usr/local/include
INCLUDEPATH += /usr/local/include/vtk-6.0
INCLUDEPATH += /usr/local/qwt-5.2.1/include
INCLUDEPATH += /usr/local/Trolltech/Qt-4.7.0/include

LIBS += -L/usr/local/lib
LIBS += -L/usr/local/qwt-5.2.1/lib
LIBS += -L/usr/local/Trolltech/Qt-4.7.0/lib

# G3D
LIBS += "-lGLG3D"
LIBS += "-lG3D"

LIBS += "-lzip"
LIBS += "-lSDL"
LIBS += "-ljpeg"
LIBS += "-lavformat"
LIBS += "-lavcodec"
LIBS += "-lavutil"
LIBS += "-ldl"
LIBS += "-lz"
LIBS += "-lpng12"
LIBS += "-lX11"

# Qwt
LIBS += "-lqwtd"

# VTK
LIBS += "-lvtkGUISupportQt-6.0"
LIBS += "-lvtkIOXML-6.0"
LIBS += "-lvtkFiltersFlowPaths-6.0"
LIBS += "-lvtkInteractionWidgets-6.0"
LIBS += "-lvtkRenderingOpenGL-6.0"
LIBS += "-lvtkRenderingFreeTypeOpenGL-6.0"
LIBS += "-lvtkRenderingCore-6.0"


# Qt3D
LIBS += "-lQt3D"

# Gsl
LIBS += "-lgslcblas"
LIBS += "-lgsl"

# Boost
LIBS += "-lboost_program_options"
}

# Local Variables:
# mode: makefile
# End:
