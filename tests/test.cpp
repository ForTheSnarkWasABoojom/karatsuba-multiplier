#include "gtest/gtest.h"
#include "../src/generator/verilog_generator.h"
#include <fstream>
#include <cstdlib>
#include <cstdio>

using namespace std;
const string test_multiplier_filename = "test_karatsuba_multiplier.v";
const string test_testbench_filename = "test_tb_karatsuba_multiplier.v";
const string test_compiled_filename = "test_compiled.vpp";
const string test_log_filename = "test_log.log";

// Тесты для функции generateTestbench при малом значении N
TEST(UnitTest, GenerateTestbenchForN3) {
    string result = generateTestbench(3);
    
    // Проверяем, что тест-бенч содержит ключевые элементы
    EXPECT_NE(result.find("module tb_karatsuba_multiplier"), string::npos);
    EXPECT_NE(result.find("parameter N = 3"), string::npos);
    EXPECT_NE(result.find("initial begin"), string::npos);
    
    // Проверяем, что шаг тестирования для N=3 равен 1
    EXPECT_NE(result.find("for (i = 0; i < MAX; i = i + 1)"), string::npos);
    EXPECT_NE(result.find("for (j = 0; j < MAX; j = j + 1)"), string::npos);
}
// Тесты для функции generateTestbench при большом значении N
TEST(UnitTest, GenerateTestbenchForN10) {
    string result = generateTestbench(10);
    
    // Проверяем, что тест-бенч содержит ключевые элементы
    EXPECT_NE(result.find("module tb_karatsuba_multiplier"), string::npos);
    EXPECT_NE(result.find("parameter N = 10"), string::npos);
    EXPECT_NE(result.find("initial begin"), string::npos);
    
    // Проверяем, что шаг тестирования для N=10 увеличен
    EXPECT_NE(result.find("for (i = 0; i < MAX; i = i + 32)"), string::npos);
    EXPECT_NE(result.find("for (j = 0; j < MAX; j = j + 32)"), string::npos);
}

// Вспомогательная функция для проверки существования файла
bool fileExists(const string& filename) {
    ifstream file(filename);
    return file.good();
}

// Вспомогательная функция для чтения файла в строку
string readFileToString(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    return content;
}

// Вспомогательная функция для удаления файлов
void cleanupGeneratedFiles() {
    remove(test_multiplier_filename.c_str());
    remove(test_testbench_filename.c_str());
    remove(test_compiled_filename.c_str());
    remove(test_log_filename.c_str());
}

// Тест сценария: генерация, компиляция и проверка вывода
TEST(FunctionalTest, FullFlowForN10) {
    // 1. Генерация Verilog-модуля и тестбенча для N=10
    string moduleCode = generateVerilogModule(10);
    string testbenchCode = generateTestbench(10);

    // Записываем сгенерированный Verilog-модуль в файл
    ofstream moduleFile(test_multiplier_filename);
    ASSERT_TRUE(moduleFile.is_open());
    moduleFile << moduleCode;
    moduleFile.close();

    // Записываем сгенерированный тестбенч в файл
    ofstream testbenchFile(test_testbench_filename);
    ASSERT_TRUE(testbenchFile.is_open());
    testbenchFile << testbenchCode;
    testbenchFile.close();

    // 2. Компиляция с помощью iverilog
    string compileCommand = "iverilog -o "+test_compiled_filename+" " + test_multiplier_filename + " " + test_testbench_filename;
    int compileResult = system(compileCommand.c_str());
    ASSERT_EQ(compileResult, 0) << "Ошибка компиляции Verilog файлов";

    // Проверяем, что скомпилированный файл существует
    ASSERT_TRUE(fileExists(test_compiled_filename)) << "Файл "<<test_compiled_filename<<" не был создан";

    // 3. Запуск скомпилированного файла с помощью vvp
    string runCommand = "vvp " + test_compiled_filename + " > "+test_log_filename;
    int runResult = system(runCommand.c_str());
    ASSERT_EQ(runResult, 0) << "Ошибка выполнения скомпилированного Verilog кода";

    // Проверяем вывод программы
    string output = readFileToString(test_log_filename);
    ASSERT_NE(output.find("All tests passed."), string::npos) << "Тесты не прошли успешно";

    // 4. Удаление сгенерированных файлов
    cleanupGeneratedFiles();
}

// Тест сценария: генерация, компиляция и проверка вывода
TEST(FunctionalTest, FullFlowForN100) {
    // 1. Генерация Verilog-модуля и тестбенча для N=100
    string moduleCode = generateVerilogModule(100);
    string testbenchCode = generateTestbench(100);

    // Записываем сгенерированный Verilog-модуль в файл
    ofstream moduleFile(test_multiplier_filename);
    ASSERT_TRUE(moduleFile.is_open());
    moduleFile << moduleCode;
    moduleFile.close();

    // Записываем сгенерированный тестбенч в файл
    ofstream testbenchFile(test_testbench_filename);
    ASSERT_TRUE(testbenchFile.is_open());
    testbenchFile << testbenchCode;
    testbenchFile.close();

    // 2. Компиляция с помощью iverilog
    string compileCommand = "iverilog -o "+test_compiled_filename+" " + test_multiplier_filename + " " + test_testbench_filename;
    int compileResult = system(compileCommand.c_str());
    ASSERT_EQ(compileResult, 0) << "Ошибка компиляции Verilog файлов";

    // Проверяем, что скомпилированный файл существует
    ASSERT_TRUE(fileExists(test_compiled_filename)) << "Файл "<<test_compiled_filename<<" не был создан";

    // 3. Запуск скомпилированного файла с помощью vvp
    string runCommand = "vvp " + test_compiled_filename + " > "+test_log_filename;
    int runResult = system(runCommand.c_str());
    ASSERT_EQ(runResult, 0) << "Ошибка выполнения скомпилированного Verilog кода";

    // Проверяем вывод программы
    string output = readFileToString(test_log_filename);
    ASSERT_NE(output.find("All tests passed."), string::npos) << "Тесты не прошли успешно";

    // 4. Удаление сгенерированных файлов
    cleanupGeneratedFiles();
}

// Главная функция для запуска всех тестов
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
