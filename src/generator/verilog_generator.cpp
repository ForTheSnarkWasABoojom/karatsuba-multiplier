#include "verilog_generator.h"
#include <string>
#include <sstream>
#include <set>

using namespace std;

// Глобальные переменные для хранения определений модулей
string module_definitions = "";           // Для хранения всех определений модулей
set<int> generated_modules;               // Множество уже сгенерированных модулей
set<int> adder_sizes;                     // Множество размеров сумматоров
set<int> subtractor_sizes;                // Множество размеров вычитателей

// Функция для генерации Verilog-модуля умножителя Карацубы
string generateVerilogModule(int n) {
    int module_count = 0; // Счетчик модулей для генерации уникальных имен
    module_definitions = ""; // Очистка определений модулей
    generated_modules.clear(); // Очистка множества сгенерированных модулей
    adder_sizes.clear();       // Очистка размеров сумматоров
    subtractor_sizes.clear();  // Очистка размеров вычитателей

    // Генерация верхнего модуля и его тело
    generateKaratsubaModule(n);

    // Генерация модулей сумматоров и вычитателей для всех необходимых размеров
    string adder_modules = "";
    for (int size : adder_sizes) {
        adder_modules += generateAdderModule(size) + "\n";
    }

    string subtractor_modules = "";
    for (int size : subtractor_sizes) {
        subtractor_modules += generateSubtractorModule(size) + "\n";
    }

    // Комбинирование всех модулей
    stringstream ss;
    ss << adder_modules;
    ss << subtractor_modules;
    ss << module_definitions; // Все модули уже добавлены в module_definitions

    return ss.str();
}

// Функция для генерации модуля Карацубы заданной разрядности n
string generateKaratsubaModule(int n) {
    string module_name = "karatsuba_mult_" + to_string(n);
    stringstream ss;

    // Проверка, был ли уже сгенерирован модуль для данного n
    if (generated_modules.find(n) != generated_modules.end()) {
        return ""; // Если модуль уже сгенерирован, возвращаем пустую строку
    }
    generated_modules.insert(n); // Добавляем n в множество сгенерированных модулей

    // Начало определения модуля
    ss << "module " << module_name << "(\n";
    ss << "    input [" << n - 1 << ":0] x,\n";
    ss << "    input [" << n - 1 << ":0] y,\n";
    ss << "    output [" << 2 * n - 1 << ":0] product\n";
    ss << ");\n\n";

    int module_count = 0;
    // Генерация тела модуля
    ss << generateKaratsubaModuleBody(n, module_count);

    // Конец определения модуля
    ss << "endmodule\n\n";

    // Добавляем определение модуля в глобальную переменную
    module_definitions = ss.str() + module_definitions; // Добавляем в начало, чтобы верхний модуль был последним

    return ""; // Возвращаем пустую строку, так как определение модуля уже сохранено
}

// Функция для генерации тела модуля Карацубы
string generateKaratsubaModuleBody(int n, int &module_count) {
    stringstream ss;

    if (n <= 2) {
        // Базовый случай для n <= 2: прямое умножение с использованием побитовых операций
        ss << generateMultiplicationLogic(n, "x", "y", "product");
    } else {
        // Разбиение на старшие и младшие части
        int m = n / 2;
        int n_minus_m = n - m;

        ss << "wire [" << n_minus_m - 1 << ":0] x0 = x[" << n_minus_m - 1 << ":0];\n";
        ss << "wire [" << m - 1 << ":0] x1 = x[" << n - 1 << ":" << n_minus_m << "];\n";
        ss << "wire [" << n_minus_m - 1 << ":0] y0 = y[" << n_minus_m - 1 << ":0];\n";
        ss << "wire [" << m - 1 << ":0] y1 = y[" << n - 1 << ":" << n_minus_m << "];\n\n";

        // Рекурсивные вызовы для z2, z0
        string z2 = "z2_" + to_string(module_count);
        string z0 = "z0_" + to_string(module_count);
        string z1 = "z1_" + to_string(module_count);
        module_count++;

        // z2 = x1 * y1
        ss << "wire [" << 2 * m - 1 << ":0] " << z2 << ";\n";
        ss << generateKaratsubaModuleCall(m, "x1", "y1", z2, module_count);

        // z0 = x0 * y0
        ss << "wire [" << 2 * n_minus_m - 1 << ":0] " << z0 << ";\n";
        ss << generateKaratsubaModuleCall(n_minus_m, "x0", "y0", z0, module_count);

        // s1 = x1 + x0
        int s_width = max(m, n_minus_m) + 1;
        adder_sizes.insert(s_width); // Добавляем размер сумматора в множество
        ss << "wire [" << s_width - 1 << ":0] s1;\n";
        ss << "adder_" << s_width << " adder_s1 (\n";
        ss << "    .a({{" << (s_width - m) << "{1'b0}}, x1}),\n";
        ss << "    .b({{" << (s_width - n_minus_m) << "{1'b0}}, x0}),\n";
        ss << "    .sum(s1)\n";
        ss << ");\n";

        // s2 = y1 + y0
        ss << "wire [" << s_width - 1 << ":0] s2;\n";
        ss << "adder_" << s_width << " adder_s2 (\n";
        ss << "    .a({{" << (s_width - m) << "{1'b0}}, y1}),\n";
        ss << "    .b({{" << (s_width - n_minus_m) << "{1'b0}}, y0}),\n";
        ss << "    .sum(s2)\n";
        ss << ");\n\n";

        // p = s1 * s2
        string p = "p_" + to_string(module_count);
        int p_width = 2 * s_width;
        ss << "wire [" << p_width - 1 << ":0] " << p << ";\n";

        // Проверка на возможность рекурсивного вызова
        if (s_width >= n) {
            // Прямое умножение для p
            ss << generateMultiplicationLogic(s_width, "s1", "s2", p);
        } else {
            // Рекурсивный вызов для p
            ss << generateKaratsubaModuleCall(s_width, "s1", "s2", p, module_count);
        }

        // z1 = p - z2 - z0
        subtractor_sizes.insert(p_width); // Добавляем размер вычитателя в множество
        ss << "wire [" << p_width - 1 << ":0] temp_sub1;\n";
        ss << "subtractor_" << p_width << " sub1 (\n";
        ss << "    .a(" << p << "),\n";
        ss << "    .b({{" << (p_width - 2 * m) << "{1'b0}}, " << z2 << "}),\n";
        ss << "    .diff(temp_sub1)\n";
        ss << ");\n";

        ss << "wire [" << p_width - 1 << ":0] " << z1 << ";\n";
        ss << "subtractor_" << p_width << " sub2 (\n";
        ss << "    .a(temp_sub1),\n";
        ss << "    .b({{" << (p_width - 2 * n_minus_m) << "{1'b0}}, " << z0 << "}),\n";
        ss << "    .diff(" << z1 << ")\n";
        ss << ");\n\n";

        // Сборка конечного произведения
        int product_width = 2 * n;
        adder_sizes.insert(product_width); // Добавляем размер сумматора в множество
        ss << "wire [" << product_width - 1 << ":0] z2_shift = {" << z2 << ", " << (2 * n_minus_m) << "'b0};\n";
        ss << "wire [" << product_width - 1 << ":0] z1_shift = {" << z1 << ", " << n_minus_m << "'b0};\n";
        ss << "wire [" << product_width - 1 << ":0] z0_ext = {{" << (product_width - (2 * n_minus_m)) << "{1'b0}}, " << z0 << "};\n";

        ss << "wire [" << product_width - 1 << ":0] temp_sum1;\n";
        ss << "adder_" << product_width << " adder1 (\n";
        ss << "    .a(z2_shift),\n";
        ss << "    .b(z1_shift),\n";
        ss << "    .sum(temp_sum1)\n";
        ss << ");\n";

        ss << "adder_" << product_width << " adder2 (\n";
        ss << "    .a(temp_sum1),\n";
        ss << "    .b(z0_ext),\n";
        ss << "    .sum(product)\n";
        ss << ");\n";
    }

    return ss.str();
}


// Функция для генерации вызова подмодуля Карацубы
string generateKaratsubaModuleCall(int n, const string &x, const string &y, const string &product, int &module_count) {
    stringstream ss;
    string module_name = "karatsuba_mult_" + to_string(n);

    // Генерация подмодуля, если он еще не был сгенерирован
    if (generated_modules.find(n) == generated_modules.end()) {
        generateKaratsubaModule(n);
    }

    ss << module_name << " mult_" << module_count << " (\n";
    ss << "    .x(" << x << "),\n";
    ss << "    .y(" << y << "),\n";
    ss << "    .product(" << product << ")\n";
    ss << ");\n\n";
    module_count++;

    return ss.str();
}

// Функция для генерации логики умножения для малых значений n
string generateMultiplicationLogic(int n, const string &a, const string &b, const string &product) {
    stringstream ss;

    ss << "// Прямое умножение для " << n << "-битных чисел\n";
    ss << "assign " << product << " = ";

    // Генерируем умножение с помощью побитовых операций и сложения
    bool first_term = true;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (first_term) {
                first_term = false;
            } else {
                ss << " + ";
            }
            ss << "((" << a << "[" << i << "] & " << b << "[" << j << "]) << " << (i + j) << ")";
        }
    }
    ss << ";\n";

    return ss.str();
}

// Функция для генерации модуля сумматора
string generateAdderModule(int n) {
    stringstream ss;
    ss << "module adder_" << n << "(\n";
    ss << "    input [" << n - 1 << ":0] a,\n";
    ss << "    input [" << n - 1 << ":0] b,\n";
    ss << "    output [" << n - 1 << ":0] sum\n";
    ss << ");\n";

    // Реализация сумматора через побитовые операции
    ss << "assign sum = a + b;\n"; // Можно заменить на побитовую реализацию при необходимости

    ss << "endmodule\n";
    return ss.str();
}

// Функция для генерации модуля вычитателя
string generateSubtractorModule(int n) {
    stringstream ss;
    ss << "module subtractor_" << n << "(\n";
    ss << "    input [" << n - 1 << ":0] a,\n";
    ss << "    input [" << n - 1 << ":0] b,\n";
    ss << "    output [" << n - 1 << ":0] diff\n";
    ss << ");\n";

    // Реализация вычитателя через побитовые операции
    ss << "assign diff = a - b;\n"; // Можно заменить на побитовую реализацию при необходимости

    ss << "endmodule\n";
    return ss.str();
}

// Функция для генерации тестбенча
string generateTestbench(int n) {
    if (n < 1) 
    {
        return "";
    }
    
    // Определение шага тестирования
    int step = (n > 8) ? (1 << (n / 2)) : 1;  // Если n > 8, увеличиваем шаг экспоненциально
    int MAX = 1 << n; // Максимальное значение для перебора

    stringstream ss;

    ss << "`timescale 1ns / 1ps\n\n";
    ss << "module tb_karatsuba_multiplier_" << n << ";\n\n";

    ss << "    // Параметры\n";
    ss << "    parameter N = " << n << ";\n";
    ss << "    parameter MAX = " << MAX << ";\n\n";

    ss << "    // Входные сигналы\n";
    ss << "    reg [N-1:0] a;\n";
    ss << "    reg [N-1:0] b;\n\n";

    ss << "    // Выходной сигнал\n";
    ss << "    wire [2*N-1:0] product;\n\n";

    ss << "    // Инстанцирование модуля умножителя\n";
    ss << "    karatsuba_mult_" << n << " uut (\n";
    ss << "        .x(a),\n";
    ss << "        .y(b),\n";
    ss << "        .product(product)\n";
    ss << "    );\n\n";

    ss << "    // Процедура тестирования\n";
    ss << "    integer i, j, errors;\n";
    ss << "    reg [2*N-1:0] expected;\n\n";

    ss << "    initial begin\n";
    ss << "        // Инициализация\n";
    ss << "        a = 0;\n";
    ss << "        b = 0;\n";
    ss << "        errors = 0;\n\n";

    ss << "        // Временная задержка\n";
    ss << "        #10;\n\n";

    ss << "        // Тестирование с шагом " << step << "\n";
    ss << "        for (i = 0; i < MAX; i = i + " << step << ") begin\n";
    ss << "            for (j = 0; j < MAX; j = j + " << step << ") begin\n";
    ss << "                a = i;\n";
    ss << "                b = j;\n";
    ss << "                expected = i * j;\n";
    ss << "                #1;\n";
    ss << "                if (product !== expected) begin\n";
    ss << "                    $display(\"Mismatch! a=%d, b=%d, product=%d, expected=%d\", a, b, product, expected);\n";
    ss << "                    errors = errors + 1;\n";
    ss << "                end\n";
    ss << "            end\n";
    ss << "        end\n\n";

    ss << "        // Вывод результата\n";
    ss << "        if (errors == 0) begin\n";
    ss << "            $display(\"All tests passed.\");\n";
    ss << "        end else begin\n";
    ss << "            $display(\"Errors found: %d.\", errors);\n";
    ss << "        end\n";

    ss << "        $finish;\n";
    ss << "    end\n\n";

    ss << "endmodule\n";

    return ss.str();
}