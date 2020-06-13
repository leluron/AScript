VARS_OLD := $(.VARIABLES)

DEPSDIR = .deps
OBJDIR = .obj
SRCDIR = src
DISTDIR = dist
GRAMMARTESTDIR = grammar_tests
TESTDIR = tests

FLAGS = -g -std=c++17 -I/usr/include/antlr4-runtime/ -I$(SRCDIR)/include/

GRAMMARFILE = $(wildcard *.g4)
GRAMMAR = $(patsubst %.g4, %, ${GRAMMARFILE})

PARSERDIR = $(SRCDIR)/parser
PARSER = $(patsubst %, %Parser, ${GRAMMAR}) $(patsubst %, %Lexer, ${GRAMMAR})
PARSERH = $(patsubst %, $(PARSERDIR)/%.h, $(PARSER))
PARSERSRC = $(patsubst %, $(PARSERDIR)/%.cpp, $(PARSER))

SRC = $(patsubst $(SRCDIR)/%.cpp, %, $(wildcard $(SRCDIR)/*.cpp))
OBJPATH = $(patsubst %, $(OBJDIR)/%.o, $(PARSER) $(SRC))

LIBFILE = $(DISTDIR)/libascript.a

UNIT_TEST = unit_tests

lib: $(LIBFILE) $(UNIT_TEST)
	./unit_tests

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
	rm -rf $(UNIT_TEST)

.PHONY: clean

$(PARSERH) $(PARSERSRC): $(GRAMMARFILE) | $(PARSERDIR)
	antlr4 -Dlanguage=Cpp $< -o $(PARSERDIR) -visitor

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

TESTCLASSES = $(patsubst %, $(GRAMMARTESTDIR)/%Parser.class, ${GRAMMAR})

grammartest: $(TESTCLASSES)

%: $(TESTDIR)/%.cpp $(LIBFILE)
	g++ -o $@ $< $(FLAGS) -Ldist/ -lascript -Iinclude/ -lantlr4-runtime -lstdc++fs

vars:; $(foreach v, $(filter-out $(VARS_OLD) VARS_OLD,$(.VARIABLES)), $(info $(v) = $($(v)))) @#noop

