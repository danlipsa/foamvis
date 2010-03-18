HEADERS += DebugStream.h ParsingDriver.h AttributeInfo.h Attribute.h \
	NameSemanticValue.h ElementUtils.h \
	 Body.h Color.h DefineAttributeType.h \
	Data.h Edge.h Element.h ExpressionTree.h \
	Face.h GLWidget.h OrientedEdge.h OrientedFace.h \
	ParsingData.h Vertex.h MainWindow.h  FaceEdgeIndex.h \
        AttributeCreator.h stable.h SystemDifferences.h DataFiles.h Debug.h \
	DisplayBodyFunctors.h DisplayFaceFunctors.h DisplayVertexFunctors.h \
	DisplayEdgeFunctors.h DisplayElement.h ProcessBodyTorus.h \
	TriangleFit.h EdgeNormalFit.h
SOURCES += DebugStream.cpp ParsingDriver.cpp AttributeInfo.cpp Attribute.cpp \
	NameSemanticValue.cpp Element.cpp ElementUtils.cpp\
	Body.cpp Color.cpp DataFiles.cpp Debug.cpp EdgeNormalFit.cpp\
	Data.cpp Edge.cpp ExpressionTree.cpp Face.cpp FaceEdgeIndex.cpp\
	GLWidget.cpp main.cpp OrientedEdge.cpp OrientedFace.cpp \
	ParsingData.cpp Vertex.cpp MainWindow.cpp AttributeCreator.cpp \
	TriangleFit.cpp
FORMS += Window.ui
LEXSOURCES += EvolverData.l
YACCSOURCES += EvolverData.y
QMAKE_YACC = .\/bison.pl -y --report=state
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
QMAKE_CXXFLAGS += -arch i686 -msse3 -mfpmath=sse
QMAKE_LFLAGS += -arch i686
INCLUDEPATH += /usr/local/G3D-8.00-b04/build/osx-i386-g++4.2/include
INCLUDEPATH += /usr/local/boost_1_39_0
LIBS += -L/usr/local/G3D-8.00-b04/build/osx-i386-g++4.2/lib
LIBS += "-lGLG3D" 
LIBS += "-lG3D" 
LIBS += "-ljpeg"
LIBS += "-lz"
LIBS += "-lzip"
LIBS += "-lavformat"
LIBS += "-lavcodec"
LIBS += "-lavutil"
LIBS += "-framework Carbon"
}
else:unix {
#INCLUDEPATH = /usr/local/G3D-7.01/build/linux-x86_64-g++4.4/include $$INCLUDEPATH
INCLUDEPATH =  /usr/local/G3D-8.00-b04/build/linux-x86_64-g++4.4/include $$INCLUDEPATH
#LIBS += -L/usr/local/G3D-7.01/build/linux-x86_64-g++4.4/lib
LIBS += -L/usr/local/G3D-8.00-b04/build/linux-x86_64-g++4.4/lib
LIBS += "-lGLG3D"
LIBS += "-lG3D"
LIBS += "-lzip"
LIBS += "-lSDL"
LIBS += "-ljpeg"
LIBS += "-lavformat"
LIBS += "-lavcodec"
LIBS += "-lavutil"
}

# Local Variables:
# mode: makefile
# End:
