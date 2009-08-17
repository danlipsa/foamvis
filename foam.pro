HEADERS += AttributeInfo.h Attribute.h NameSemanticValue.h ElementUtils.h \
	 Body.h AttributeType.h Color.h \
	Data.h Edge.h Element.h ExpressionTree.h \
	Face.h GLWidget.h lexYacc.h OrientedEdge.h OrientedFace.h \
	ParsingData.h Vertex.h SemanticError.h MainWindow.h \
        AttributeCreator.h stable.h SystemDifferences.h
SOURCES += AttributeInfo.cpp Attribute.cpp NameSemanticValue.cpp Element.cpp \
	 Body.cpp Color.cpp\
	Data.cpp Edge.cpp ExpressionTree.cpp Face.cpp \
	GLWidget.cpp main.cpp OrientedEdge.cpp OrientedFace.cpp \
	ParsingData.cpp Vertex.cpp MainWindow.cpp AttributeCreator.cpp
FORMS += Window.ui
LEXSOURCES += foam.l
YACCSOURCES += foam.y
QMAKE_YACC = bison -y --debug --report=state
TARGET = foam
QT           += opengl
CONFIG       += qt debug precompile_header
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

# Local Variables:
# mode: makefile
# End:
