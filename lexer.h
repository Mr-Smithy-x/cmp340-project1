/*
 * Copyright (C) Rida Bazzi, 2016
 *
 * Do not share this file with anyone
 */
#ifndef __LEXER__H__
#define __LEXER__H__

#include <vector>
#include <cstring>

#include "inputbuf.h"

// ------- token types -------------------

typedef enum {
    END_OF_FILE = 0,
    PUBLIC, PRIVATE,
    EQUAL, COLON, COMMA, SEMICOLON,
    LBRACE, RBRACE, ID, ERROR // TODO: Add labels for new token types here
} TokenType;

class Token {
public:

    std::string lexeme;
    TokenType token_type;
    int line_no;
};

class LexicalAnalyzer {
public:
    Token GetToken();
    TokenType UngetToken(Token);
    LexicalAnalyzer();
    int start();

private:
    std::vector<Token> tokens;
    int line_no;
    Token tmp;
    InputBuffer input;

    int parseStatements();
    int parseStatement();
    int parseGlobalVariables();
    int parseVariables();
    int parseVariable(TokenType visibility);
    int parseScope();
    int handleGlobalVariablesAndScope();

    bool SkipSpace();
    bool SkipComment();

    bool IsKeyword(std::string);

    TokenType FindKeywordIndex(std::string);

    Token ScanIdOrKeyword();

};

#endif  //__LEXER__H__
