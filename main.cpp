#include <vector>
#include <iostream>
#include "CLR1_parser.h"

int main() {
    std::vector<std::string> grammar;
    std::cout << "(1) Enter the grammar rules line by line in the format \"S->AbCd\" without spaces.\n";
    std::cout << "(2) Terminal can be any character except uppercase English characters, '$', '.' and '@'.\n";
    std::cout << "(3) For an empty terminal, use '~'.\n";
    std::cout << "(4) After all the rules, specify the starting nonterminal.\n";
    std::cout << "(5) Enter the words to be checked line by line.\n";
    std::cout << "(6) After finishing working with the parser, write \"[STOP]\".\n\n";
    std::cout << "Write grammar rules:\n";
    std::string current_input;
    do {
        std::cin >> current_input;
        grammar.push_back(current_input);
    } while (current_input.size() != 1);

    Algo parser(grammar);
    PrintStates(parser);
    PrintTable(parser);
    std::cout << "Enter words:\n";
    std::string input;
    std::cin >> input;
    while (input != "[STOP]") {
        std::vector<std::string> derivation_rules;
        if (parser.Predict(input, derivation_rules)) {
            std::cout << input << " belongs to grammar" << '\n';
            auto derivation = CalculateDerivation(derivation_rules);
            std::cout << "Rightmost derivation: " << derivation << '\n';
        } else {
            std::cout << input << " doesn't belong to grammar" << '\n';
        }
        std::cout << '\n';
        std::cin >> input;
    }
    return 0;
}
