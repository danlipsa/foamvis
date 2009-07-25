# @file:   Makefile
# @author: Dan R. Lipsa
# 
# Build commands for foam project. It includes several types of files:
# lex  (processed by  flex), yacc  (processed by  bison), C++,  QT C++
# (preprocessed by moc)

objects = Data.o ParsingData.o Body.o Face.o OrientedFace.o \
	Edge.o OrientedEdge.o Point.o main.o ExpressionTree.o
# sources created through other tools (flex and bison)
prep_objects = foamYacc.o foamLex.o
# QT classes that need preprocessing through moc
qt_objects = GLWidget.o Window.o
all_objects = $(prep_objects) $(moc_objects) $(objects) $(qt_objects)

# QT preprocessor options
MOC = /usr/lib/qt4/bin/moc
moc_objects = $(addprefix moc_, $(qt_objects))

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
YFLAGS = --debug

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

moc_%.cpp: %.h
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@

# Bison generates  y.tab.h as well,  and the authomatic rule  does not
# specify that.
foamYacc.c foamYacc.h: foamYacc.y
	$(YACC) $(YFLAGS) $<
	mv foamYacc.tab.c foamYacc.c
	mv foamYacc.tab.h foamYacc.h

# Prevent the deletion of intermediate files. See for more information
# info: make/Implicit Rules/Chained Rules
.SECONDARY: $(subst .o,.c,$(prep_objects)) $(subst .o,.cpp,$(moc_objects))

.PHONY : clean
clean:
	rm -f *.o *.d* foam*.c foam*.h foam foam.output TAGS moc_*.cpp

.PHONY : tags
tags:
	etags *.h *.cpp *.y *.lex

.PHONY : doc
doc:
	doxygen

# automatic dependency generation
%.d: %.c*
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

# include all generated dependencies in the make file, if the dependency files
# don't exist they are created. We don't include the moc_ files because they
# depend only on the .h file they are generated from, and that dependency is
# already specified.
# sinclude behaves like include but it does not give an error for missing files
# after a clean, the dependency files are missing
dep_objects = $(prep_objects) $(objects) $(qt_objects)
ifneq ($(MAKECMDGOALS),clean)
	sinclude $(dep_objects:.o=.d)
endif
