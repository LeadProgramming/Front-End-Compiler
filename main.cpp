/*
* Name: David Nguyen (891362089)
* Professor's Name: Professor Anthony Le
* Date: 12/16/19
* Assignment Name: Syntax Analysis
* Assignment #: 3
*/
#include "Compiler.h"

using namespace std;

int main() {
    string syntax_input = "syntax_input.txt";
    string syntax_output = "syntax_output.txt";
    try {
        Compiler c;
        c.readFile(syntax_input);
        c.writeFile(syntax_output);
    }
    catch(const runtime_error& e){
        cout << e.what();
    }
    return 0;
}