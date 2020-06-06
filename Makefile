VARS_OLD := $(.VARIABLES)

DEPSDIR = .deps
OBJDIR = .obj
SRCDIR = src
INCLUDEDIR = $(SRCDIR)/include
DISTDIR = dist
TESTDIR = grammar_tests

DEPFLAGS=-MT $@ -MMD -MP -MF $(DEPSDIR)/$*.d
FLAGS=-I/usr/include/antlr4-runtime/ -g -std=c++17
LIBS=

GRAMMARS = AS
GRAMMARFILES = $(patsubst %, %.g4, ${GRAMMARS})

PARSERDIR = $(SRCDIR)/parser
PARSER = $(patsubst %, %Parser, ${GRAMMARS}) $(patsubst %, %Lexer, ${GRAMMARS})
PARSERH = $(patsubst %, $(PARSERDIR)/%.h, $(PARSER))
PARSERSRC = $(patsubst %, $(PARSERDIR)/%.cpp, $(PARSER))

SRC = script ASTGen
OBJPATH = $(patsubst %, $(OBJDIR)/%.o, $(PARSER) $(SRC))

LIBFILE = $(DISTDIR)/libascript.a

HEADERS = $(wildcard $(INCLUDEDIR)/*.h)

lib: $(LIBFILE)

install: $(LIBFILE)
	install -d /usr/local/lib
	install -m 644 $(LIBFILE) /usr/local/lib
	install -d /usr/local/include/ascript
	install -m 644 $(HEADERS) /usr/local/include/ascript

$(LIBFILE): $(OBJPATH) | $(DISTDIR)
	ar r $(LIBFILE) $(OBJPATH)

cleancompile:
	rm -rf $(DISTDIR)
	rm -rf $(OBJDIR)
	rm -rf $(DEPSDIR)

cleanparser:
	rm -rf $(PARSERDIR)

cleantest:
	rm -rf $(TESTDIR)

clean: cleancompile cleanparser cleantest

.PHONY: clean cleancompile cleanparser cleantest

$(PARSERH) $(PARSERSRC): $(GRAMMARFILES) | $(PARSERDIR)
	antlr4 -Dlanguage=Cpp *.g4 -o $(PARSERDIR) -visitor

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPSDIR)/%.d $(PARSERH) | $(DEPSDIR) $(OBJDIR)
	g++ $(DEPFLAGS) -c -o $@ $< $(FLAGS)

$(OBJDIR)/%.o: $(PARSERDIR)/%.cpp $(PARSERH) | $(DEPSDIR) $(OBJDIR)
	g++ -c -o $@ $< $(FLAGS) -w

$(DEPSDIR): ; mkdir -p $@
$(OBJDIR): ; mkdir -p $@
$(PARSERDIR): ; mkdir -p $@
$(DISTDIR): ; mkdir -p $@
$(TESTDIR): ; mkdir -p $@

DEPSFILES := $(patsubst %,$(DEPSDIR)/%.d, $(SRC))
$(DEPSFILES):
include $(wildcard $(DEPSFILES))

$(TESTDIR)/%Parser.java: %.g4 | $(TESTDIR)
	antlr4 $< -o $(TESTDIR)

$(TESTDIR)/%Parser.class: $(TESTDIR)/%Parser.java
	javac $(TESTDIR)/$**.java

TESTCLASSES = $(patsubst %, $(TESTDIR)/%Parser.class, ${GRAMMARS})

test: $(TESTCLASSES)

vars:; $(foreach v, $(filter-out $(VARS_OLD) VARS_OLD,$(.VARIABLES)), $(info $(v) = $($(v)))) @#noop

