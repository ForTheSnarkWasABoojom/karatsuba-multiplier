#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include "generator/verilog_generator.h"

using namespace std;
const string DEFAULT_MULTIPLIER_FILENAME = "output/karatsuba_multiplier_"; // Основа файла для модуля по умолчанию
const string DEFAULT_TESTBENCH_FILENAME = "output/tb_karatsuba_multiplier_"; // Основа файла для тестбенча по умолчанию

// Функция для печатания ошибки
void printError(const string& message) {
    cerr << "Ошибка: " << message << endl;
}
// Функция проверки, является ли строка валидным целым числом
// Если строка валидна, то она возвращает true и заполняет переменную number
bool isValidNumber(const string& str, int& number) {
    // Проверка на пустую строку
    if (str.empty()) {
        return false;
    }

    // Проверка на символы
    for (char c : str) {
        if (!isdigit(c)) {
            return false;  // Строка содержит недопустимые символы
        }
    }

    try {
        // Попытка преобразования строки в число с проверкой на диапазон
        size_t idx;
        long long num = stoll(str, &idx);

        // Проверка, не выходит ли число за пределы допустимых значений для int
        if (num < numeric_limits<int>::min() || num > numeric_limits<int>::max()) {
            throw out_of_range("Число слишком большое для типа int");
        }

        // Если мы успешно преобразовали и дошли до конца строки, то это валидное число
        if (idx == str.size()) {
            number = static_cast<int>(num);  // Преобразуем и сохраняем результат в переменную number
            return true;
        }
        return false;
    } catch (const invalid_argument&) {
        // Ошибка при попытке преобразования строки в число (например, строка пустая или содержит недопустимые символы)
        return false;
    } catch (const out_of_range&) {
        // Число слишком большое или маленькое для типа int
        return false;
    }
}

int main(int argc, char* argv[]) {
    string output_filename, number_str, generated_text;
    bool create_test = false;  // Флаг создания тестбенча
    int n;

    // Обрабатываем аргументы командной строки
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];

        if (arg == "-output") {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                output_filename = string(argv[++i]);
            } else {
                printError("Необходимо передать имя файла после аргумента -output.");
                return 1;
            }
        }  else if (arg == "-test") {
            create_test = true; 
        } else {
            number_str = arg; // Считаем, что это число
        }
    }

    // Проверяем, что число было передано и корректно
    if (number_str.empty()) {
        printError("Необходимо передать число как аргумент.");
        return 1;
    }

    if (!isValidNumber(number_str, n)) {
        printError("N должен являться типом int. Некорректное значение: " + number_str);
        return 1;
    }

    if (n <= 0) {
        printError("N должно быть больше 0.");
        return 1;
    }

    // Если флаг true - создаем тестбенч, иначе - модуль
    if (create_test) {
        if (output_filename == "") {
            output_filename = DEFAULT_TESTBENCH_FILENAME+number_str+".v";
        }
        generated_text = generateTestbench(n);
    } else {
        if (output_filename == "") {
            output_filename = DEFAULT_MULTIPLIER_FILENAME+number_str+".v";
        }
        generated_text = generateVerilogModule(n);
    }
    // Проверяем и создаем папку "output"
    filesystem::create_directories("output");

    // Открытие файла для записи
    ofstream output_file(output_filename);
    if (!output_file) {
        printError("Не удалось открыть файл для записи: " + output_filename);
        return 1;
    }

    output_file << generated_text;
    output_file.close();
    
    cout << "Программа успешно сгенерирована в файле: " << output_filename << endl;
    return 0;
}
