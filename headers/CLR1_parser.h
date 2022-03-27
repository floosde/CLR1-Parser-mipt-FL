
#ifndef CLR1_PARSER_CLR1_PARSER_H
#define CLR1_PARSER_CLR1_PARSER_H

#include <iostream>
#include <cctype>
#include <set>
#include <map>
#include <vector>
#include <iomanip>
#include <stack>
#include <exception>

class GrammarException : public std::runtime_error {
public:
    explicit GrammarException(std::string message) : runtime_error(message) {
    }
};

class NonTerminal {
public:
    char symbol;
    std::set<char> first = std::set<char>();
    NonTerminal() = default;
    explicit NonTerminal(char input_symbol) : symbol(input_symbol) {
    }
};

class Item {
public:
    char lhs;
    std::string rhs;
    std::set<char> lookaheads;
    Item(char rule_lhs, std::string rule_rhs, int dot_pos, const std::set<char> &new_lookaheads);
    Item(char rule_lhs, std::string rule_rhs_with_dot, const std::set<char> &new_lookaheads);
    bool operator==(Item &second);
    bool operator<(Item &second);
    bool operator!=(Item &second);
};

class State {
public:
    static int counter_id;
    int personal_id;
    std::vector<Item> items;
    std::map<char, int> transitions;
    explicit State(const std::vector<Item> &new_items);
};

struct ParseType {
    bool is_shift;
    std::string value;
};

auto const kRealStart = '@';
auto const kEndOfLine = '$';
auto const kEpsilon = '~';
auto const kNonTerminalAlphabetBeg = 65;
auto const kNonTerminalAlphabetEnd = 90;

class Algo {
public:
    using ProductionRulesType = std::map<char, std::vector<std::string>>;
    using TerminalSetType = std::set<char>;
    using NonTerminalSetType = std::map<char, NonTerminal>;
    using AutomatonType = std::vector<State>;
    using TableType = std::vector<std::map<char, std::string>>;

    ProductionRulesType production_rules;
    TerminalSetType terminals;
    NonTerminalSetType nonterminals;
    AutomatonType states;
    TableType table;
    int accept_state_id;
    explicit Algo(std::vector<std::string> &grammar);
    void Fit(std::vector<std::string> &grammar);
    void ProcessInputGrammar(std::vector<std::string> &grammar);
    std::set<char> CalculateFirstOfSymbol(char curr_symbol, std::set<char> &symbols_in_lhs);
    std::set<char> CalculateFirstOfChain(std::string chain);
    bool ItemIsInSet(Item &curr_item, std::vector<Item> &items);
    std::vector<Item> Closure(std::vector<Item> items);
    std::vector<Item> Transition(State &state, char symbol);
    int StateAlreadyExists(std::vector<Item> &curr_state);
    void CalculateStates();
    void MakeTable();
    bool Predict(std::string input, std::vector<std::string> &derivation_rules);
};

std::string CalculateDerivation(const std::vector<std::string> &derivation_rules);
void PrintStates(const Algo &parser);
void PrintTable(Algo &parser);

#endif //CLR1_PARSER_CLR1_PARSER_H
