# @file:   Makefile
# @author: Dan R. Lipsa
# 
# Build commands for foam project. It includes several types of files:
# lex  (processed by  flex), yacc  (processed by  bison), C++,  QT C++
# (preprocessed by moc)

objects = Data.o ParsingData.o\
	Body.o Face.o OrientedFace.o Edge.o OrientedEdge.o Point.o main.o\
	ExpressionTree.o
prep_objects = foamYacc.o foamLex.o
qt_objects = glwidget.o window.o

# QT preprocessor options
MOC = /usr/lib/qt4/bin/moc
moc_objects = $(addprefix moc_, $(qt_objects))
all_objects = $(objects) $(prep_objects) $(qt_objects) $(moc_objects)


# C/C++ preprocessor options
CPPFLAGS = $(DEFINES) $(INCPATH)
DEFINES       = \
	-DQT_OPENGL_LIB \
	-DQT_GUI_LIB \
	-DQT_CORE_LIB
INCPATH = \
	-I/usr/lib/qt4/mkspecs/linux-g++ \
	-I/usr/include/QtCore \
	-I/usr/include/QtGui \
	-I/usr/include/QtOpenGL \
	-I/usr/include \
	-I/usr/X11R6/include \
	-I.

# C/C++ compiler options
CC = g++
CFLAGS = -Wall -g
CXXFLAGS = -Wall -g

# lex options
LEX = flex
LFLAGS = --debug

# yacc options
YACC = bison
YFLAGS = --debug -y

# C/C++ linker options
LIBS = \
	-L/usr/X11R6/lib \
	-lQtOpenGL \
	-lQtGui \
	-lQtCore \
	-lGLU \
	-lGL \
	-lpthread

foam:	$(all_objects)
	$(CC) -ofoam $(all_objects) $(LIBS)

clean:
	rm -f y.tab.h foamYacc.c foamLex.c *.o *.d* foam foam.output TAGS \
	moc_*.cpp

tags:
	etags *.h *.cpp *.y *.lex

doc:
	mkdir html;doxygen

moc_%.cpp: %.h
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@

# automatic dependency generation
%.d: %.cpp
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$


# sinclude behaves like include but it does not give an error for missing files
# after a clean, the dependency files are missing
sinclude $(all_objects:.o=.d)
