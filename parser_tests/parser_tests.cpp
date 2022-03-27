#include <gtest/gtest.h>
#include "CLR1_parser.h"

TEST(Predict, CorrectBracketSequences) {
    std::vector<std::string> grammar = {"S->(S)S",
                                        "S->[S]S",
                                        "S->{S}S",
                                        "S->~", // '~' stands for epsilon
                                        "S"};
    Algo parser(grammar);

    // Input: (()())
    {
        std::vector<std::string> derivation_rules;
        std::string derivation;
        ASSERT_EQ(parser.Predict("(()())", derivation_rules), true);
        derivation = CalculateDerivation(derivation_rules);
        ASSERT_EQ(derivation, "@->S->(S)S->(S)->((S)S)->((S)(S)S)->((S)(S))->((S)())->(()())");
    }

    // Input: ())
    {
        std::vector<std::string> derivation_rules;
        ASSERT_EQ(parser.Predict("())", derivation_rules), false);
    }

    // Input: [({})]
    {
        std::vector<std::string> derivation_rules;
        std::string derivation;
        ASSERT_EQ(parser.Predict("[({})]", derivation_rules), true);
        derivation = CalculateDerivation(derivation_rules);
        ASSERT_EQ(derivation, "@->S->[S]S->[S]->[(S)S]->[(S)]->[({S}S)]->[({S})]->[({})]");
    }

    // Input: ([)]
    {
        std::vector<std::string> derivation_rules;
        ASSERT_EQ(parser.Predict("([)]", derivation_rules), false);
    }

    // Input: [{}()]
    {
        std::vector<std::string> derivation_rules;
        std::string derivation;
        ASSERT_EQ(parser.Predict("[{}()]", derivation_rules), true);
        derivation = CalculateDerivation(derivation_rules);
        ASSERT_EQ(derivation, "@->S->[S]S->[S]->[{S}S]->[{S}(S)S]->[{S}(S)]->[{S}()]->[{}()]");
    }

}

TEST(Predict, ArithmeticExpressions) {
    std::vector<std::string> grammar = {"E->E+T",
                                        "E->T",
                                        "T->T*F",
                                        "T->F",
                                        "F->(E)",
                                        "F->1",
                                        "F->2",
                                        "F->3",
                                        "E"};

    Algo parser(grammar);

    // Input: (1+1)*2
    {
        std::vector<std::string> derivation_rules;
        std::string derivation;
        ASSERT_EQ(parser.Predict("(1+1)*2", derivation_rules), true);
        derivation = CalculateDerivation(derivation_rules);
        ASSERT_EQ(derivation, "@->E->T->T*F->T*2->F*2->(E)*2->(E+T)*2->(E+F)*2->(E+1)*2->(T+1)*2->(F+1)*2->(1+1)*2");
    }

    // Input: 1++
    {
        std::vector<std::string> derivation_rules;
        ASSERT_EQ(parser.Predict("1++", derivation_rules), false);
    }

    // Input: (1*2*3)
    {
        std::vector<std::string> derivation_rules;
        std::string derivation;
        ASSERT_EQ(parser.Predict("(1*2*3)", derivation_rules), true);
        derivation = CalculateDerivation(derivation_rules);
        ASSERT_EQ(derivation, "@->E->T->F->(E)->(T)->(T*F)->(T*3)->(T*F*3)->(T*2*3)->(F*2*3)->(1*2*3)");
    }

}

TEST(Predict, EmptyString) {
    std::vector<std::string> grammar = {"S->AB",
                                        "A->CD",
                                        "C->d",
                                        "C->~",
                                        "D->~",
                                        "B->a",
                                        "B->~",
                                        "S"};

    Algo parser(grammar);

    // Input: ~
    {
        std::vector<std::string> derivation_rules;
        std::string derivation;
        ASSERT_EQ(parser.Predict("~", derivation_rules), true);
        derivation = CalculateDerivation(derivation_rules);
        ASSERT_EQ(derivation, "@->S->AB->A->CD->C->");
    }

    grammar = {"S->~", "S"};
    parser = Algo(grammar);

    // Input: ~
    {
        std::vector<std::string> derivation_rules;
        std::string derivation;
        ASSERT_EQ(parser.Predict("~", derivation_rules), true);
        derivation = CalculateDerivation(derivation_rules);
        ASSERT_EQ(derivation, "@->S->");
    }

}

TEST(Predict, ArbitaryGrammars) {
    std::vector<std::string> grammar = {"S->CC",
                                        "C->cC",
                                        "C->d",
                                        "S"};

    Algo parser(grammar);

    // Input: ccccdd
    {
        std::vector<std::string> derivation_rules;
        std::string derivation;
        ASSERT_EQ(parser.Predict("ccccdd", derivation_rules), true);
        derivation = CalculateDerivation(derivation_rules);
        ASSERT_EQ(derivation, "@->S->CC->Cd->cCd->ccCd->cccCd->ccccCd->ccccdd");
    }

    // Input: cccc
    {
        std::vector<std::string> derivation_rules;
        ASSERT_EQ(parser.Predict("cccc", derivation_rules), false);
    }

    grammar = {"S->S+A",
               "S->~",
               "A->A*B",
               "A->B",
               "B->C^B",
               "B->C",
               "C->a",
               "S"};

    parser = Algo(grammar);

    // Input: +a^a*a
    {
        std::vector<std::string> derivation_rules;
        std::string derivation;
        ASSERT_EQ(parser.Predict("+a^a*a", derivation_rules), true);
        derivation = CalculateDerivation(derivation_rules);
        ASSERT_EQ(derivation, "@->S->S+A->S+A*B->S+A*C->S+A*a->S+B*a->S+C^B*a->S+C^C*a->S+C^a*a->S+a^a*a->+a^a*a");
    }

    // Input: ^a+
    {
        std::vector<std::string> derivation_rules;
        ASSERT_EQ(parser.Predict("^a+", derivation_rules), false);
    }

}

TEST(Exceptions, ShiftReduceConflict) {

    std::vector<std::string> grammar = {"S->E",
                                        "E->T",
                                        "E->(E)",
                                        "T->n",
                                        "T->+T",
                                        "T->T+n",
                                        "S"};
    try {
        Algo parser(grammar);
    } catch (std::runtime_error &error) {
        ASSERT_EQ(std::string(error.what()), "Shift/Reduce conflict occurred. The grammar is no LR(1) type.");
    }

    grammar = {"S->RS",
               "S->R",
               "R->abT",
               "T->aT",
               "T->c",
               "T->~",
               "S"};
    try {
        Algo parser(grammar);
    } catch (std::runtime_error &error) {
        ASSERT_EQ(std::string(error.what()), "Shift/Reduce conflict occurred. The grammar is no LR(1) type.");
    }

}


TEST(Exceptions, ReduceReduceConflict) {
    std::vector<std::string> grammar = {"S->AB",
                                        "A->~",
                                        "B->c",
                                        "B->d",
                                        "B->D",
                                        "D->n",
                                        "D->BA",
                                        "S"};
    try {
        Algo parser(grammar);
    } catch (std::runtime_error &error) {
        ASSERT_EQ(std::string(error.what()), "Reduce/Reduce conflict occurred. The grammar is no LR(1) type.");
    }

    grammar = {"S->X",
               "X->Y",
               "X->1",
               "Y->1"
               "S"};

    try {
        Algo parser(grammar);
    } catch (std::runtime_error &error) {
        ASSERT_EQ(std::string(error.what()), "Reduce/Reduce conflict occurred. The grammar is no LR(1) type.");
    }
}

TEST(Exceptions, UselessCharacters) {
    std::vector<std::string> grammar = {"S->S+A",
                                        "A->A*B",
                                        "A->B",
                                        "B->C^B",
                                        "B->C",
                                        "C->a",
                                        "S"};
    try {
        Algo parser(grammar);
    } catch (std::runtime_error &error) {
        ASSERT_EQ(std::string(error.what()), "The grammar contains useless characters.");
    }
}

TEST(Exceptions, RulesAbsence) {
    std::vector<std::string> grammar = {"S"};
    try {
        Algo parser(grammar);
    } catch (std::runtime_error &error) {
        ASSERT_EQ(std::string(error.what()), "The grammar is incorrect. There are no reachable symbols.");
    }
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
