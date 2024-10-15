#ifndef VERILOG_GENERATOR_H
#define VERILOG_GENERATOR_H

#include <string>
using namespace std;

string generateVerilogModule(int N);
string generateTestbench(int N);

string generateKaratsubaModule(int n);
string generateKaratsubaModuleBody(int n, int &module_count);
string generateKaratsubaModuleCall(int n, const string &x, const string &y, const string &product, int &module_count);
string generateMultiplicationLogic(int n, const string &a, const string &b, const string &product);
string generateAdderModule(int n);
string generateSubtractorModule(int n);

#endif