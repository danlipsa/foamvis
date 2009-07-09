objects = foam.tab.o lex.yy.o Data.o
CFLAGS = -Wall
CC=g++

foam:	$(objects)
	$(CC) -ofoam $(objects)

lex.yy.c: foam.lex foam.tab.h lexYacc.h
	flex --debug foam.lex

foam.tab.c foam.tab.h: foam.y lexYacc.h
	bison --verbose --debug foam.y

clean:
	rm -f lex.yy.c foam.tab.c foam.tab.h *.o *.d foam foam.output

# automatic dependency generation
%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

# sinclude behaves like include but it does not give an error for missing files
# after a clean, the dependency files are missing
sinclude $(objects:.o=.d)
