CXX = g++

CXXFLAGS = -std=c++11 -I./include -I.

TARGET = bin/test_parser
all: $(TARGET)

$(TARGET): src/parser.y src/lexer.l src/parser_test.cpp
	@mkdir -p bin
	@echo "Generating Parser..."
	bison -d src/parser.y
	@echo "Generating Lexer..."
	flex src/lexer.l
	@echo "Compiling Executable..."
	$(CXX) $(CXXFLAGS) src/parser_test.cpp parser.tab.c lex.yy.c -o $(TARGET)
	@echo "Build successful! Executable is at $(TARGET)"

clean:
	rm -f parser.tab.c parser.tab.h lex.yy.c $(TARGET)

test: $(TARGET)
	@echo "\n--- Running Test Query ---"
	echo "SELECT name, age FROM users JOIN roles ON users.role_id = roles.id WHERE age > 18 GROUP BY role_id;" | ./$(TARGET)