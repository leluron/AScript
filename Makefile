VARS_OLD := $(.VARIABLES)

DEPSDIR = .deps
OBJDIR = .obj
SRCDIR = src
DISTDIR = dist
GRAMMARTESTDIR = grammar_tests
TESTDIR = tests

FLAGS = -g -std=c++17 -I/usr/include/antlr4-runtime/ -I$(SRCDIR)/include/

GRAMMARS = AS
GRAMMARFILES = $(patsubst %, %.g4, ${GRAMMARS})

PARSERDIR = $(SRCDIR)/parser
PARSER = $(patsubst %, %Parser, ${GRAMMARS}) $(patsubst %, %Lexer, ${GRAMMARS})
PARSERH = $(patsubst %, $(PARSERDIR)/%.h, $(PARSER))
PARSERSRC = $(patsubst %, $(PARSERDIR)/%.cpp, $(PARSER))

SRC = $(patsubst $(SRCDIR)/%.cpp, %, $(wildcard $(SRCDIR)/*.cpp))
OBJPATH = $(patsubst %, $(OBJDIR)/%.o, $(PARSER) $(SRC))

LIBFILE = $(DISTDIR)/libascript.a
TESTS = $(patsubst $(TESTDIR)/%.cpp, %, $(wildcard $(TESTDIR)/*.cpp))

lib: $(LIBFILE)

install: $(LIBFILE)
	install -d /usr/local/lib
	install -m 644 $(LIBFILE) /usr/local/lib
	install -d /usr/local/include/ascript
	install -m 644 $(wildcard $(SRCDIR)/include/ascript/*.h) /usr/local/include/ascript

$(LIBFILE): $(OBJPATH) | $(DISTDIR)
	ar r $(LIBFILE) $(OBJPATH)

clean:
	rm -rf $(DISTDIR)
	rm -rf $(OBJDIR)
	rm -rf $(DEPSDIR)
	rm -rf $(PARSERDIR)
	rm -rf $(GRAMMARTESTDIR)
	rm -rf $(TESTS)

.PHONY: clean

$(PARSERH) $(PARSERSRC): $(GRAMMARFILES) | $(PARSERDIR)
	antlr4 -Dlanguage=Cpp *.g4 -o $(PARSERDIR) -visitor

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPSDIR)/%.d $(PARSERH) | $(DEPSDIR) $(OBJDIR)
	g++ -MT $@ -MMD -MP -MF $(DEPSDIR)/$*.d -c -o $@ $< $(FLAGS)

$(OBJDIR)/%.o: $(PARSERDIR)/%.cpp $(PARSERH) | $(DEPSDIR) $(OBJDIR)
	g++ -c -o $@ $< $(FLAGS) -w

$(DEPSDIR): ; mkdir -p $@
$(OBJDIR): ; mkdir -p $@
$(PARSERDIR): ; mkdir -p $@
$(DISTDIR): ; mkdir -p $@
$(GRAMMARTESTDIR): ; mkdir -p $@

DEPSFILES := $(patsubst %,$(DEPSDIR)/%.d, $(SRC))
$(DEPSFILES):
include $(wildcard $(DEPSFILES))

$(GRAMMARTESTDIR)/%Parser.java: %.g4 | $(GRAMMARTESTDIR)
	antlr4 $< -o $(GRAMMARTESTDIR)

$(GRAMMARTESTDIR)/%Parser.class: $(GRAMMARTESTDIR)/%Parser.java
	javac $(GRAMMARTESTDIR)/$**.java

TESTCLASSES = $(patsubst %, $(GRAMMARTESTDIR)/%Parser.class, ${GRAMMARS})

grammartest: $(TESTCLASSES)

test: $(TESTS)

%: $(TESTDIR)/%.cpp $(LIBFILE)
	g++ -o $@ $< $(FLAGS) -Ldist/ -lascript -Iinclude/ -lantlr4-runtime

all: grammartest lib install test

vars:; $(foreach v, $(filter-out $(VARS_OLD) VARS_OLD,$(.VARIABLES)), $(info $(v) = $($(v)))) @#noop

