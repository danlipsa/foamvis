HEADERS += Body.h Data.h Edge.h Element.h ExpressionTree.h \
	Face.h GLWidget.h lexYacc.h OrientedEdge.h OrientedFace.h \
	ParsingData.h Point.h SemanticError.h Window.h Attribute.h \
	AttributeCreator.h
SOURCES += Body.cpp Data.cpp Edge.cpp ExpressionTree.cpp Face.cpp \
	GLWidget.cpp main.cpp OrientedEdge.cpp OrientedFace.cpp \
	ParsingData.cpp Point.cpp Window.cpp AttributeCreator.cpp
FORMS += Window.ui
LEXSOURCES += foam.l
YACCSOURCES += foam.y
QMAKE_YACC = bison -y --debug
TARGET = foam
QT           += opengl
CONFIG       += qt debug

# Local Variables:
# mode: makefile
# End:
