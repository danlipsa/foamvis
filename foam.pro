HEADERS += Body.h Data.h Edge.h Element.h ExpressionTree.h \
	Face.h GLWidget.h lexYacc.h OrientedEdge.h OrientedFace.h \
	ParsingData.h Point.h SemanticError.h Window.h
SOURCES += Body.cpp Data.cpp Edge.cpp ExpressionTree.cpp Face.cpp \
	GLWidget.cpp main.cpp OrientedEdge.cpp OrientedFace.cpp \
	ParsingData.cpp Point.cpp Window.cpp
FORMS += Window.ui
LEXSOURCES += foam.l
YACCSOURCES += foam.y
TARGET = foam
QT           += opengl
CONFIG       += qt debug
QMAKE_YACC = bison -y --debug
