# ============================================================
#  DBMSTeam 404 — Optimizer Build System
# ============================================================

CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
LEX      = flex
YACC     = bison

SRC_DIR  = src
INC_DIR  = include
BIN      = optimizer_test

# ============================================================
#  Build targets
# ============================================================

all: $(BIN)

# Step 1: Run Bison on parser.y  →  parser.tab.c + parser.tab.h
$(SRC_DIR)/parser.tab.c $(SRC_DIR)/parser.tab.h: $(SRC_DIR)/parser.y
	$(YACC) -d -o $(SRC_DIR)/parser.tab.c $(SRC_DIR)/parser.y

# Step 2: Run Flex on lexer.l  →  lex.yy.c
$(SRC_DIR)/lex.yy.c: $(SRC_DIR)/lexer.l $(SRC_DIR)/parser.tab.h
	$(LEX) -o $(SRC_DIR)/lex.yy.c $(SRC_DIR)/lexer.l

# Step 3: Compile everything together
$(BIN): $(SRC_DIR)/main.cpp $(SRC_DIR)/parser.tab.c $(SRC_DIR)/lex.yy.c
	$(CXX) $(CXXFLAGS) \
		$(SRC_DIR)/main.cpp \
		$(SRC_DIR)/parser.tab.c \
		$(SRC_DIR)/lex.yy.c \
		-o $(BIN)

clean:
	rm -f $(SRC_DIR)/parser.tab.c $(SRC_DIR)/parser.tab.h \
	       $(SRC_DIR)/lex.yy.c \
	       $(BIN)

rebuild: clean all

.PHONY: all clean rebuild
