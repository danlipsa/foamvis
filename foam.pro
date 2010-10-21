HEADERS += Application.h Attribute.h AttributeCreator.h AttributeInfo.h \
	AttributeHistogram.h HistogramHeight.h Body.h \
	BodyAlongTime.h BodyIndex.h BodySelector.h BodySetStatistics.h \
	ColorBar.h ColorBarModel.h ColoredElement.h Comparisons.h \
	Debug.h DebugStream.h \
	DisplayBodyFunctors.h DisplayFaceFunctors.h DisplayVertexFunctors.h \
	DisplayEdgeFunctors.h DisplayElement.h DisplayWidget.h\
	EditColorMap.h Edge.h Element.h ExpressionTree.h Enums.h\
	Foam.h FoamAlongTime.h Face.h \
	GLWidget.h Hashes.h Histogram.h HistogramItem.h\
	LineEditFocus.h MainWindow.h NameSemanticValue.h \
	OOBox.h OpenGLInfo.h OpenGLUtils.h OrientedElement.h\
	OrientedEdge.h OrientedFace.h OrientedFaceIndex.h \
	ParsingData.h ParsingDriver.h ProcessBodyTorus.h \
	QuadraticEdge.h RestrictedRangeSlider.h \
	stable.h StripIterator.h SystemDifferences.h \
	TransferFunctionHistogram.h TimeStepsSlider.h Utils.h \
	Vertex.h
SOURCES += Application.cpp Attribute.cpp AttributeCreator.cpp AttributeInfo.cpp \
	AttributeHistogram.cpp HistogramHeight.cpp \
	Body.cpp BodyAlongTime.cpp BodyIndex.cpp \
	BodySetStatistics.cpp BodySelector.cpp\
	ColorBar.cpp ColorBarModel.cpp ColoredElement.cpp Comparisons.cpp \
	Debug.cpp DebugStream.cpp DisplayWidget.cpp Edge.cpp \
	EditColorMap.cpp Element.cpp ExpressionTree.cpp Enums.cpp\
	Foam.cpp FoamAlongTime.cpp Face.cpp \
	GLWidget.cpp Hashes.cpp Histogram.cpp HistogramItem.cpp\
	main.cpp MainWindow.cpp  \
	NameSemanticValue.cpp \
	OOBox.cpp OpenGLUtils.cpp OrientedElement.cpp\
	OrientedEdge.cpp OrientedFace.cpp OrientedFaceIndex.cpp\
	ParsingData.cpp ParsingDriver.cpp ProcessBodyTorus.cpp \
	QuadraticEdge.cpp RestrictedRangeSlider.cpp\
	StripIterator.cpp TimeStepsSlider.cpp \
	TransferFunctionHistogram.cpp Utils.cpp Vertex.cpp
FORMS += HistogramHeight.ui EditColorMap.ui \
	 RestrictedRangeSlider.ui MainWindow.ui OpenGLInfo.ui
LEXSOURCES += EvolverData.l
YACCSOURCES += EvolverData.y
QMAKE_YACC = .\\/bison.pl -y --report=state --debug
QMAKE_LEX = .\\/flex.pl
TARGET = foam
QT           += opengl
CONFIG       += qt debug precompile_header no_keywords
PRECOMPILED_HEADER = stable.h

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
INCLUDEPATH += /usr/local/boost_1_39_0

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
}
else:unix {
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
LIBS += "-lzip"
LIBS += "-lSDL"
LIBS += "-ljpeg"
LIBS += "-lavformat"
LIBS += "-lavcodec"
LIBS += "-lavutil"
LIBS += "-lqwt"
LIBS += "-lz"
LIBS += "-lpng12"
LIBS += "-lX11"
LIBS += "-lvtkFiltering"
LIBS += "-lvtkCommon"
LIBS += "-lvtksys"
LIBS += "-ldl"
LIBS += "-lQt3D"
}

# Local Variables:
# mode: makefile
# End:
