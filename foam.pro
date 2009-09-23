HEADERS += DebugStream.h ParsingDriver.h AttributeInfo.h Attribute.h \
	NameSemanticValue.h ElementUtils.h \
	 Body.h Color.h DefineAttributeType.h \
	Data.h Edge.h Element.h ExpressionTree.h \
	Face.h GLWidget.h OrientedEdge.h OrientedFace.h \
	ParsingData.h Vertex.h SemanticError.h MainWindow.h \
        AttributeCreator.h stable.h SystemDifferences.h
SOURCES += DebugStream.cpp ParsingDriver.cpp AttributeInfo.cpp Attribute.cpp \
	NameSemanticValue.cpp Element.cpp \
	Body.cpp Color.cpp\
	Data.cpp Edge.cpp ExpressionTree.cpp Face.cpp \
	GLWidget.cpp main.cpp OrientedEdge.cpp OrientedFace.cpp \
	ParsingData.cpp Vertex.cpp MainWindow.cpp AttributeCreator.cpp
FORMS += Window.ui
LEXSOURCES += EvolverData.l
YACCSOURCES += EvolverData.y
QMAKE_YACC = bison -y
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
QMAKE_CC=gcc-4.2
QMAKE_CXX=g++-4.2
INCLUDEPATH = /usr/local/G3D-7.01/build/osx-i386-g++4.2/include $$INCLUDEPATH
LIBS += -L/usr/local/G3D-7.01/build/osx-i386-g++4.2/lib
LIBS += "-lGLG3D" 
LIBS += "-lG3D" 
LIBS += "-lzip" 
LIBS += "-lpng" 
LIBS += "-ljpeg" 
}
else:unix {
INCLUDEPATH += /usr/local/G3D-7.00/build/linux-i686-g++-4.1/include
LIBS += -L/usr/local/G3D-7.00/build/linux-i686-g++-4.1/lib
LIBS += "-lGLG3D"
LIBS += "-lG3D"
LIBS += "-lzip"
LIBS += "-lSDL"
LIBS += "-ljpeg"
}

# Local Variables:
# mode: makefile
# End:
