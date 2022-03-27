
#include "CLR1_parser.h"

// class Item
Item::Item(char rule_lhs,
           std::string rule_rhs,
           int dot_pos,
           const std::set<char> &new_lookaheads) :
        lhs(rule_lhs),
        rhs(rule_rhs),
        lookaheads(new_lookaheads) {
    rhs.insert(rhs.begin() + dot_pos, '.');
}

Item::Item(char rule_lhs,
           std::string rule_rhs_with_dot,
           const std::set<char> &new_lookaheads) :
        lhs(rule_lhs),
        rhs(rule_rhs_with_dot),
        lookaheads(new_lookaheads) {
}

bool Item::operator==(Item &second) {
    if (lhs != second.lhs) {
        return false;
    }
    if (rhs != second.rhs) {
        return false;
    }
    if (lookaheads != second.lookaheads) {
        return false;
    }
    return true;
}

bool Item::operator<(Item &second) {
    if (lhs < second.lhs) {
        return true;
    }
    if (lhs == second.lhs) {
        if (rhs < second.rhs) {
            return true;
        }
        if (rhs == second.rhs) {
            if (lookaheads < second.lookaheads) {
                return true;
            }
        }
    }
    return false;
}

bool Item::operator!=(Item &second) {
    return !operator==(second);
}

// class State

int State::counter_id = 0;
State::State(const std::vector<Item> &new_items) : items(new_items), personal_id(counter_id++) {
}

// class Algo

Algo::Algo(std::vector<std::string> &grammar) {
    Fit(grammar);
}

void Algo::Fit(std::vector<std::string> &grammar) {
    ProcessInputGrammar(grammar);
    for (auto &nonterminal: nonterminals) {
        if (nonterminal.first == kRealStart) {
            continue;
        }
        std::set<char> symbols_in_lhs;
        symbols_in_lhs.insert(nonterminal.first);
        CalculateFirstOfSymbol(nonterminal.first, symbols_in_lhs);
    }
    State::counter_id = 0;
    CalculateStates();
    MakeTable();
}

void Algo::ProcessInputGrammar(std::vector<std::string> &grammar) {
    std::string delimiter = "->";
    for (auto &rule: grammar) {
        if (rule.size() == 1) {
            break;
        }
        auto delimiter_pos = rule.find(delimiter);
        auto lhs_part_substr = rule.substr(0, delimiter_pos);
        if (lhs_part_substr.size() != 1 || !std::isupper(lhs_part_substr[0])) {
            throw GrammarException("The grammar contains incorrect LHS part.");
        }
        auto lhs_part = lhs_part_substr[0];
        auto rhs_part = rule.substr(delimiter_pos + 2);
        if (!production_rules.contains(lhs_part)) {
            production_rules[lhs_part] = std::vector<std::string>();
        }
        production_rules[lhs_part].push_back(rhs_part);

        if (!nonterminals.contains(lhs_part)) {
            nonterminals[lhs_part] = NonTerminal(lhs_part);
        }

        for (auto symbol: rhs_part) {
            if (symbol >= kNonTerminalAlphabetBeg && symbol <= kNonTerminalAlphabetEnd) {
                if (!nonterminals.contains(symbol)) {
                    nonterminals[symbol] = NonTerminal(symbol);
                }
            } else {
                if (!terminals.contains(symbol)) {
                    terminals.insert(symbol);
                }
            }
        }
    }
    nonterminals[kRealStart] = NonTerminal(kRealStart);
    production_rules[kRealStart].push_back(grammar[grammar.size() - 1]);
}

std::set<char> Algo::CalculateFirstOfSymbol(char curr_symbol, std::set<char> &symbols_in_lhs) {
    if (terminals.contains(curr_symbol)) {
        return {curr_symbol};
    }

    bool eps_as_rhs_term = false;

    for (auto &rule: production_rules[curr_symbol]) {
        if (terminals.contains(rule[0])) {
            nonterminals[curr_symbol].first.insert(rule[0]);
            if (rule[0] == kEpsilon) {
                eps_as_rhs_term = true;
            }
        }
    }

    for (auto &rule: production_rules[curr_symbol]) {
        for (auto symbol_in_rule: rule) {
            std::set<char> first_of_current;
            if (!symbols_in_lhs.contains(symbol_in_rule)) {
                if (nonterminals.contains(symbol_in_rule)) {
                    symbols_in_lhs.insert(symbol_in_rule);
                }
                first_of_current = CalculateFirstOfSymbol(symbol_in_rule, symbols_in_lhs);
            } else {
                first_of_current = nonterminals[symbol_in_rule].first;
            }
            nonterminals[curr_symbol].first.insert(first_of_current.begin(), first_of_current.end());
            if (!first_of_current.contains(kEpsilon)) {
                if (auto eps_in_first = nonterminals[curr_symbol].first.find(kEpsilon);
                        (!eps_as_rhs_term) && eps_in_first != nonterminals[curr_symbol].first.end()) {
                    nonterminals[curr_symbol].first.erase(eps_in_first);
                }
                break;
            }
        }
    }

    if (nonterminals[curr_symbol].first.empty()) {
        throw GrammarException("The grammar contains useless characters.");
    }

    return nonterminals[curr_symbol].first;
}

std::set<char> Algo::CalculateFirstOfChain(std::string chain) {
    std::set<char> first_of_chain;
    for (auto letter: chain) {
        if (nonterminals.contains(letter)) {
            first_of_chain.insert(nonterminals[letter].first.begin(), nonterminals[letter].first.end());
        } else {
            first_of_chain.insert(letter);
            if (auto eps_in_set = first_of_chain.find(kEpsilon); eps_in_set != first_of_chain.end()) {
                first_of_chain.erase(eps_in_set);
            }
            break;
        }
        if (!nonterminals[letter].first.contains(kEpsilon)) {
            if (auto eps_in_set = first_of_chain.find(kEpsilon); eps_in_set != first_of_chain.end()) {
                first_of_chain.erase(eps_in_set);
            }
            break;
        }

    }
    return first_of_chain;
}

bool Algo::ItemIsInSet(Item &curr_item, std::vector<Item> &items) {
    for (auto &item: items) {
        if (curr_item.lhs != item.lhs || curr_item.rhs != item.rhs) {
            continue;
        }

        std::set<char> lookaheads_union;
        std::set_union(curr_item.lookaheads.begin(), curr_item.lookaheads.end(), item.lookaheads.begin(),
                       item.lookaheads.end(), std::inserter(lookaheads_union, lookaheads_union.begin()));
        item.lookaheads = std::move(lookaheads_union);
        return true;
    }
    return false;
}

std::vector<Item> Algo::Closure(std::vector<Item> items) {
    std::vector<Item> new_items_set = items;
    for (size_t i = 0; i < new_items_set.size(); ++i) {
        auto &item = new_items_set[i];
        auto dot_pos = item.rhs.find('.');
        if (dot_pos == item.rhs.size() - 1) {
            continue;
        }
        auto after_dot = item.rhs.substr(dot_pos + 1);
        if (nonterminals.contains(after_dot[0])) {
            std::set<char> new_lookaheads;
            std::set<char> first_of_right;
            if (after_dot.size() > 1) {
                first_of_right = CalculateFirstOfChain(after_dot.substr(1));
                for (auto letter: first_of_right) {
                    if (letter == kEpsilon) {
                        for (auto lookahead: item.lookaheads) {
                            new_lookaheads.insert(lookahead);
                        }
                        continue;
                    }
                    new_lookaheads.insert(letter);
                }
            } else {
                for (auto lookahead: item.lookaheads) {
                    new_lookaheads.insert(lookahead);
                }
            }

            for (auto &rule: production_rules[after_dot[0]]) {
                Item new_item(after_dot[0], rule, 0, new_lookaheads);
                if (!ItemIsInSet(new_item, new_items_set)) {
                    new_items_set.push_back(new_item);
                }
            }
        }
    }
    return new_items_set;
}

std::vector<Item> Algo::Transition(State &state, char symbol) {
    std::vector<Item> new_items;
    for (auto &item: state.items) {
        auto dot_pos = item.rhs.find('.');
        if (dot_pos == item.rhs.size() - 1) {
            continue;
        }
        auto after_dot = item.rhs.substr(dot_pos + 1);
        if (after_dot[0] == symbol) {
            auto new_rhs = item.rhs;
            std::swap(new_rhs[dot_pos], new_rhs[dot_pos + 1]);
            new_items.emplace_back(item.lhs, new_rhs, item.lookaheads);
        }
    }
    return Closure(new_items);
}

int Algo::StateAlreadyExists(std::vector<Item> &curr_state) {
    std::sort(curr_state.begin(), curr_state.end());
    for (auto &state: states) {
        if (curr_state.size() != state.items.size()) {
            continue;
        }
        std::sort(state.items.begin(), state.items.end());
        bool there_is_not_equal_item = false;
        for (size_t i = 0; i < curr_state.size(); ++i) {
            if (curr_state[i] != state.items[i]) {
                there_is_not_equal_item = true;
            }
        }
        if (!there_is_not_equal_item) {
            return state.personal_id;
        }
    }
    return -1;
}

void Algo::CalculateStates() {
    auto start_rule = production_rules[kRealStart][0];
    std::vector<Item> zero_state_set;
    zero_state_set.push_back(Item(kRealStart, start_rule, 0, {kEndOfLine}));
    states.emplace_back(Closure(zero_state_set));
    if (states.back().items.size() == 1) {
        throw GrammarException("The grammar is incorrect. There are no reachable symbols.");
    }
    for (size_t i = 0; i < states.size(); ++i) {
        for (auto &terminal: terminals) {
            if (terminal == kEpsilon) {
                continue;
            }
            auto new_state = Transition(states[i], terminal);
            if (!new_state.empty()) {
                auto state_id = StateAlreadyExists(new_state);
                if (state_id == -1) {
                    states.emplace_back(new_state);
                    states[i].transitions[terminal] = states[states.size() - 1].personal_id;
                } else {
                    states[i].transitions[terminal] = state_id;
                }
            }
        }

        for (auto &nonterminal: nonterminals) {
            if (nonterminal.first == kRealStart) {
                continue;
            }
            auto new_state = Transition(states[i], nonterminal.first);
            if (!new_state.empty()) {
                auto state_id = StateAlreadyExists(new_state);
                if (state_id == -1) {
                    states.emplace_back(new_state);
                    states[i].transitions[nonterminal.first] = states[states.size() - 1].personal_id;
                } else {
                    states[i].transitions[nonterminal.first] = state_id;
                }

                if (states[states.size() - 1].items[0].lhs == kRealStart) {
                    accept_state_id = states[states.size() - 1].personal_id;
                }

            }
        }
    }
}

void Algo::MakeTable() {
    table = TableType(states.size());
    for (size_t i = 0; i < table.size(); ++i) {
        for (auto &transition: states[i].transitions) {
            if (table[i].contains(transition.first)) {
                throw GrammarException("Shift/Reduce conflict occurred. The grammar is no LR(1) type.");
            }
            table[i][transition.first] = std::to_string(transition.second);
        }
        for (auto &item: states[i].items) {
            if (item.rhs[item.rhs.size() - 1] == '.' || item.rhs[item.rhs.size() - 1] == kEpsilon) {
                for (auto lookahead: item.lookaheads) {
                    if (table[i].contains(lookahead)) {
                        std::string error;
                        if (std::isdigit(table[i][lookahead][0])) {
                            error = "Shift/Reduce conflict occurred. The grammar is no LR(1) type.";
                        } else {
                            error = "Reduce/Reduce conflict occurred. The grammar is no LR(1) type.";
                        }
                        throw GrammarException(error);
                    }
                    auto rule_without_dot = item.rhs;
                    auto dot_pos = rule_without_dot.find('.');
                    rule_without_dot.erase(dot_pos);
                    table[i][lookahead] = std::string{item.lhs} + "->" + rule_without_dot;
                }
            }
        }
    }
}

bool Algo::Predict(std::string input, std::vector<std::string> &derivation_rules) {
    input += kEndOfLine;
    if (input[0] == kEpsilon && input[1] == kEndOfLine) {
        input = input.substr(1);
    }
    std::stack<ParseType> parse_stack;
    std::string delimiter = "->";
    parse_stack.push({true, "0"});
    for (size_t i = 0; i < input.size(); ++i) {
        if (!terminals.contains(input[i]) && input[i] != kEndOfLine) {
            return false;
        }
        auto action = table[std::stoi(parse_stack.top().value)].find(input[i]);
        if (action != table[std::stoi(parse_stack.top().value)].end()) {
            if (std::isdigit(action->second[0])) {
                parse_stack.push({false, std::string{input[i]}});
                parse_stack.push({true, action->second});
            } else {
                if (!table[std::stoi(parse_stack.top().value)].contains(input[i])) {
                    return false;
                }
                while (!std::isdigit(action->second[0])) {
                    derivation_rules.push_back(action->second);
                    auto delimiter_pos = action->second.find(delimiter);
                    auto lhs_part = action->second.substr(0, delimiter_pos)[0];
                    auto rhs_part = action->second.substr(delimiter_pos + 2);
                    for (int j = rhs_part.size() - 1; j >= 0; --j) {
                        while (!parse_stack.empty() && !(parse_stack.top().value == std::string{rhs_part[j]} &&
                                                         !parse_stack.top().is_shift)) {
                            parse_stack.pop();
                        }
                        if (parse_stack.empty()) {
                            return false;
                        }
                        parse_stack.pop();
                    }

                    if (lhs_part == kRealStart && i == input.size() - 1) {
                        return true;
                    }

                    int last_number = std::stoi(parse_stack.top().value);
                    parse_stack.push({false, std::string{lhs_part}});
                    action = table[last_number].find(lhs_part);

                    if (action != table[last_number].end()) {
                        parse_stack.push({true, action->second});
                    } else {
                        return false;
                    }

                    action = table[std::stoi(parse_stack.top().value)].find(input[i]);
                    if (action == table[std::stoi(parse_stack.top().value)].end()) {
                        return false;
                    }

                }
                parse_stack.push({false, std::string{action->first}});
                parse_stack.push({true, action->second});
            }
        } else {
            return false;
        }
    }
    return false;
}

// function CalculateDerivation

std::string CalculateDerivation(const std::vector<std::string> &derivation_rules) {
    std::string derivation = std::string{kRealStart};
    std::string delimiter = "->";
    derivation += delimiter + derivation_rules[derivation_rules.size() - 1][3];
    for (int i = derivation_rules.size() - 2; i >= 0; --i) {
        auto delimiter_pos = derivation.rfind(delimiter);
        auto rule_to_replace = derivation.substr(delimiter_pos + 2);
        auto nonterm_to_replace = rule_to_replace.rfind(derivation_rules[i][0]);
        auto part_after_nonterm = rule_to_replace.substr(nonterm_to_replace + 1);
        rule_to_replace = rule_to_replace.substr(0, nonterm_to_replace);
        delimiter_pos = derivation_rules[i].find(delimiter);
        auto expansion = derivation_rules[i].substr(delimiter_pos + 2);
        rule_to_replace += (expansion + part_after_nonterm);
        derivation += delimiter + rule_to_replace;
    }
    return derivation;
}

void PrintStates(const Algo &parser) {
    std::cout << "Automaton states:" << '\n';
    for (const auto &state: parser.states) {
        if (state.personal_id == parser.accept_state_id) {
            std::cout << "Accept state " << state.personal_id << '\n';
        } else {
            std::cout << "State " << state.personal_id << '\n';
        }
        for (auto &item: state.items) {
            std::cout << "(" << item.lhs << "->" << item.rhs << "|";
            for (auto symbol = item.lookaheads.begin(); symbol != item.lookaheads.end(); ++symbol) {
                if (std::next(symbol) == item.lookaheads.end()) {
                    std::cout << *symbol;
                    break;
                }
                std::cout << *symbol << ", ";
            }
            std::cout << ")" << '\n';
        }
        std::cout << '\n';
        for (auto j: state.transitions) {
            if (parser.terminals.contains(j.first)) {
                std::cout << "By terminal " << j.first << " shift to state " << j.second << '\n';
            } else {
                std::cout << "By nonterminal " << j.first << " go to state " << j.second << '\n';
            }
        }
        std::cout << '\n';
    }
}

void PrintTable(Algo &parser) {
    std::cout << std::setw(25) << std::left << "States/Symbols";

    for (const auto terminal: parser.terminals) {
        if (terminal == kEpsilon) {
            continue;
        }
        std::cout << std::setw(15) << std::left << terminal;
    }

    std::cout << std::setw(15) << std::left << kEndOfLine;

    for (const auto &nonterminal: parser.nonterminals) {
        if (nonterminal.first == kRealStart) {
            continue;
        }
        std::cout << std::setw(15) << std::left << nonterminal.first;
    }
    std::cout << '\n';

    for (size_t i = 0; i < parser.table.size(); ++i) {
        std::cout << "State " << std::setw(19) << std::left << i;
        for (const auto terminal: parser.terminals) {
            if (terminal == kEpsilon) {
                continue;
            }
            std::cout << std::setw(15) << std::left
                      << (parser.table[i].contains(terminal) ? parser.table[i][terminal] : " ");
        }

        std::cout << std::setw(15) << std::left
                  << (parser.table[i].contains(kEndOfLine) ? parser.table[i][kEndOfLine] : " ");

        for (const auto &nonterminal: parser.nonterminals) {
            if (nonterminal.first == kRealStart) {
                continue;
            }
            std::cout << std::setw(15) << std::left
                      << (parser.table[i].contains(nonterminal.first) ? parser.table[i][nonterminal.first] : " ");
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}


