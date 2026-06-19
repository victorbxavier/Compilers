CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
FLEX = flex

SRC_DIR = src
BUILD_DIR = build

TARGET = $(BUILD_DIR)/compiler

LEX_SRC = $(SRC_DIR)/lexer.l
LEX_OUT = $(BUILD_DIR)/lex.yy.c

HEADERS = $(SRC_DIR)/token.h $(SRC_DIR)/ast.h $(SRC_DIR)/parser.h \
          $(SRC_DIR)/symbol_table.h $(SRC_DIR)/semantic_analyzer.h

OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/lex.yy.o

.PHONY: all clean test

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(LEX_OUT): $(LEX_SRC) | $(BUILD_DIR)
	$(FLEX) -o $(LEX_OUT) $(LEX_SRC)

$(BUILD_DIR)/lex.yy.o: $(LEX_OUT) $(SRC_DIR)/token.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $(LEX_OUT) -o $@

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $(SRC_DIR)/main.cpp -o $@

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

test: $(TARGET)
	@echo "=== Teste: prog-factorial.ling ==="
	./$(TARGET) --ast --symbols assets/prog-factorial.ling
	@echo ""
	@echo "=== Teste: prog-bubblesort.ling ==="
	./$(TARGET) --ast --symbols assets/prog-bubblesort.ling

clean:
	rm -rf $(BUILD_DIR)
