
#include "lox.h"
int main (int argc, char* argv[]) {
    Lox lox;
    if (argc > 1) {

    } else if (argc == 1) {
        lox.runFile(argv[0]);
    } else {
        lox.runPrompt();
    }
}