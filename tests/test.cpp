#include "gtest/gtest.h"
#include "../src/generator/verilog_generator.h"
#include <fstream>
#include <cstdlib>
#include <cstdio>

using namespace std;
const string test_multiplier_filename = "test_karatsuba_multiplier.v";
const string test_testbench_filename = "test_tb_karatsua_multiplier.v";
const string test_compiled_filename = "test_compiled.vpp";
const string test_log_filename = "test_log.log";

// Функция для генерации тест-бенча
string generateTestbench(int n) {
    if (n < 1) 
    {
        return "";
    }
    
    // Определение шага тестирования
    int step = (n > 8) ? (1 << (n / 2)) : 1;  // Если N > 8, увеличиваем шаг экспоненциально
    
    string tb = 
"`timescale 1ns / 1ps\n\n"
"module tb_karatsuba_multiplier;\n\n"

"    // Параметры\n"
"    parameter N = " + to_string(n) + ";\n"
"    parameter MAX = 2**N;\n\n"

"    // Входные сигналы\n"
"    reg [N-1:0] a;\n"
"    reg [N-1:0] b;\n\n"

"    // Выходной сигнал\n"
"    wire [2*N-1:0] product;\n\n"

"    // Инстанцирование модуля умножителя\n"
"    karatsuba_multiplier uut (\n"
"        .a(a),\n"
"        .b(b),\n"
"        .product(product)\n"
"    );\n\n"

"    // Процедура тестирования\n"
"    integer i, j, errors;\n"
"    reg [2*N-1:0] expected;\n\n"

"    initial begin\n"
"        // Инициализация\n"
"        a = 0;\n"
"        b = 0;\n"
"        errors = 0;\n\n"

"        // Временная задержка\n"
"        #10;\n\n"

"        // Тестирование с шагом " + to_string(step) + "\n"
"        for (i = 0; i < MAX; i = i + " + to_string(step) + ") begin\n"
"            for (j = 0; j < MAX; j = j + " + to_string(step) + ") begin\n"
"                a = i;\n"
"                b = j;\n"
"                expected = i * j;\n"
"                #1;\n"
"                if (product !== expected) begin\n"
"                    $display(\"Mismatch! a=%d, b=%d, product=%d, expected=%d\", a, b, product, expected);\n"
"                    errors = errors + 1;\n"
"                end\n"
"            end\n"
"        end\n\n"

"        // Вывод результата\n"
"        if (errors == 0) begin\n"
"            $display(\"All tests passed.\");\n"
"        end else begin\n"
"            $display(\"Errors found: %d.\", errors);\n"
"        end\n"

"        $finish;\n"
"    end\n\n"

"endmodule\n";

    return tb;
}

// Тесты для функции generateVerilogModule
TEST(UnitTest, GenerateModuleForN3) {
    string expected = 
"module karatsuba_multiplier(\n"
"    input  [2:0] a,\n"
"    input  [2:0] b,\n"
"    output [5:0] product\n"
");\n\n"

"    // Разделение чисел на половины\n"
"    wire [1:0] a_low = a[1:0];\n"
"    wire [2:2] a_high = a[2:2];\n"
"    wire [1:0] b_low = b[1:0];\n"
"    wire [2:2] b_high = b[2:2];\n\n"

"    // Рекурсивное умножение низких и высоких частей\n"
"    wire [3:0] z0;\n"
"    wire [4:0] z1;\n"
"    wire [3:0] z2;\n\n"

"    assign z0 = a_low * b_low;\n"
"    assign z2 = a_high * b_high;\n"
"    assign z1 = (a_low + a_high) * (b_low + b_high) - z0 - z2;\n\n"

"    // Сдвиг и суммирование результатов\n"
"    assign product = (z2 << 4) + (z1 << 2) + z0;\n\n"
"endmodule\n";

    EXPECT_EQ(generateVerilogModule(3), expected);
}

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


// Главная функция для запуска всех тестов
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
