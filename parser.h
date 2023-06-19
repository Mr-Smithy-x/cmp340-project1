//
// Created by CJ on 6/18/23.
//

#ifndef CSE340_PROJECT1_PARSER_H
#define CSE340_PROJECT1_PARSER_H

#include <iostream>
#include "lexer.h"

struct Variable {
    char *name; // name of variable
    char *scope; // what scope are we in
    int level; // 0 - global, 1 - public, 2 - private
};

struct SymbolNode {
    Variable *item;
    SymbolNode *prev;
    SymbolNode *next;
};

struct Assignment {
    std::string lhs;
    std::string rhs;
    std::string scope1;
    std::string scope2;
};


class Parser {

private:
    bool debug = false;

public:

    std::vector<struct Assignment> assignments;

    struct SymbolNode *symbolNode;
    struct SymbolNode *tempNode;
    struct SymbolNode *newNode;
    char *currScope = "::";
    int currLevel = 0;

    void debugPrint(std::string text);

    void Error();

    void createSymbolTable(char *lexeme);

    void appendToSymbolTable(char *lexeme);


    void add(char *lexeme);

    void remove(char *lexeme);


    Assignment createAssignment(Token lhs_token, Token rhs_token);

    void results();
};

#endif //CSE340_PROJECT1_PARSER_H
