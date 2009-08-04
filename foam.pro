HEADERS += AttributeInfo.h Attribute.h NameSemanticValue.h ElementUtils.h \
	 Body.h \
	Data.h Edge.h Element.h ExpressionTree.h \
	Face.h GLWidget.h lexYacc.h OrientedEdge.h OrientedFace.h \
	ParsingData.h Vertex.h SemanticError.h MainWindow.h \
	AttributeCreator.h
SOURCES += AttributeInfo.cpp Attribute.cpp NameSemanticValue.cpp Element.cpp \
	 Body.cpp \
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

# Local Variables:
# mode: makefile
# End:
