# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++20 

# Папки
SRC_DIR = src
TESTS_DIR = tests
OUTPUT_DIR = bin

# Файлы
MAIN_SRC = $(SRC_DIR)/main.cpp $(SRC_DIR)/generator/verilog_generator.cpp
TEST_SRC = $(TESTS_DIR)/test.cpp $(SRC_DIR)/generator/verilog_generator.cpp
MAIN_EXEC = $(OUTPUT_DIR)/karatsuba-gen
TEST_EXEC = $(OUTPUT_DIR)/karatsuba-test

# Цели
.PHONY: all build run build_test run_test clean

# Компиляция основного кода
build: $(MAIN_SRC)
	mkdir -p $(OUTPUT_DIR)
	$(CXX) $(CXXFLAGS) -o $(MAIN_EXEC) $(MAIN_SRC)

# Запуск основного кода
run: build
	./$(MAIN_EXEC) $(ARGS)

# Компиляция тестов
build_test: $(TEST_SRC)
	mkdir -p $(OUTPUT_DIR)
	$(CXX) $(CXXFLAGS) $(TEST_SRC) -lgtest -lgtest_main -pthread -o $(TEST_EXEC)

# Запуск тестов
run_test: build_test
	./$(TEST_EXEC)

# Выполнение всех шагов последовательно
all: build build_test run_test

# Очистка сгенерированных файлов
clean:
	rm -f $(OUTPUT_DIR)/karatsuba-gen $(OUTPUT_DIR)/karatsuba-test
