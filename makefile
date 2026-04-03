CXX = g++
CXXFLAGS = -std=c++14 -I./include -I.

# By default, "make" will build both executables
all: bin/test_parser bin/optimizer

# Rule to build the parser test
bin/test_parser: src/parser.y src/lexer.l src/parser_test.cpp
	@mkdir -p bin
	@echo "Generating Parser & Lexer..."
	bison -d src/parser.y
	flex src/lexer.l
	@echo "Compiling Parser Test..."
	$(CXX) $(CXXFLAGS) src/parser_test.cpp parser.tab.c lex.yy.c -o bin/test_parser

# Rule to build the main optimizer
bin/optimizer: src/parser.y src/lexer.l src/main.cpp
	@mkdir -p bin
	@echo "Generating Parser & Lexer..."
	bison -d src/parser.y
	flex src/lexer.l
	@echo "Compiling Optimizer..."
	$(CXX) $(CXXFLAGS) src/main.cpp parser.tab.c lex.yy.c -o bin/optimizer

# Cleans up everything
clean:
	rm -f parser.tab.c parser.tab.h lex.yy.c bin/test_parser bin/optimizer

# Runs the optimizer test
test: bin/optimizer
	@echo "\n--- Running Optimizer Engine ---"
	./bin/optimizer query.json