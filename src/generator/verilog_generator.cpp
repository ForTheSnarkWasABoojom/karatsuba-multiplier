#include "verilog_generator.h"
#include <cmath>
#include <string>

using namespace std;

// Функция для генерации Verilog-модуля умножения по Карацубе
string generateVerilogModule(int n) {
    if (n < 1) 
    {
        return "";
    }
    // Определяем половину разрядности
    int m = ceil(n / 2.0);

    // Определяем необходимую ширину для z1
    // z0 и z2 требуют 2*m бит
    // z1 требует 2*m +1 бит для предотвращения переполнения
    int z1_width = 2 * m + 1;

    // Шаблон модуля
    string module = 
"module karatsuba_multiplier(\n"
"    input  [" + to_string(n-1) + ":0] a,\n"
"    input  [" + to_string(n-1) + ":0] b,\n"
"    output [" + to_string(2*n-1) + ":0] product\n"
");\n\n"

"    // Разделение чисел на половины\n"
"    wire [" + to_string(m-1) + ":0] a_low = a[" + to_string(m-1) + ":0];\n"
"    wire [" + to_string(n-1) + ":" + to_string(m) + "] a_high = a[" + to_string(n-1) + ":" + to_string(m) + "];\n"
"    wire [" + to_string(m-1) + ":0] b_low = b[" + to_string(m-1) + ":0];\n"
"    wire [" + to_string(n-1) + ":" + to_string(m) + "] b_high = b[" + to_string(n-1) + ":" + to_string(m) + "];\n\n"

"    // Рекурсивное умножение низких и высоких частей\n"
"    wire [" + to_string(2*m-1) + ":0] z0;\n"
"    wire [" + to_string(z1_width-1) + ":0] z1;\n" // Увеличенная ширина для z1
"    wire [" + to_string(2*m-1) + ":0] z2;\n\n"

"    assign z0 = a_low * b_low;\n"
"    assign z2 = a_high * b_high;\n"
"    assign z1 = (a_low + a_high) * (b_low + b_high) - z0 - z2;\n\n"

"    // Сдвиг и суммирование результатов\n"
"    assign product = (z2 << " + to_string(2*m) + ") + (z1 << " + to_string(m) + ") + z0;\n\n"
"endmodule\n";

    return module;
}