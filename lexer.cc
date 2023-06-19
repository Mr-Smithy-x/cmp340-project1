/*
 * Copyright (C) Rida Bazzi, 2016
 *
 * Do not share this file with anyone
 */
#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>

#include "types.h"
#include "lexer.h"
#include "debug.h"


// you should unget tokens in the reverse order in which they
// are obtained. If you execute
//
//    t1 = lexer.GetToken();
//    t2 = lexer.GetToken();
//    t3 = lexer.GetToken();
//
// in this order, you should execute
//
//    lexer.UngetToken(t3);
//    lexer.UngetToken(t2);
//    lexer.UngetToken(t1);
//
// if you want to unget all three tokens. Note that it does not
// make sense to unget t1 without first ungetting t2 and t3
//

using namespace std;

string reserved[] = {"END_OF_FILE",
                     "PUBLIC", "PRIVATE",
                     "EQUAL", "COLON",
                     "COMMA", "SEMICOLON",
                     "LBRACE", "RBRACE", "ID",
                     "ERROR",
};

#define KEYWORDS_COUNT 2
string keyword[] = {"public", "private"};

LexicalAnalyzer lexer;
Token token_1, token_2, token_3;

LexicalAnalyzer::LexicalAnalyzer() {
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}

bool LexicalAnalyzer::SkipSpace() {
    char c;
    bool space_encountered = false;

    if(input.EndOfInput()) {
        return false;
    }
    do {
        input.GetChar(c);
        line_no += (c == '\n');
        if(c == '\n') {
            if(line_no == 18) {
                debugPrint("Something?");
            }
        }
        if(isspace(c)) {
            space_encountered = true;
        }
    } while (!input.EndOfInput() && c == ' ');

    if(c == '\n') {
        return true;
    }

    if (!input.EndOfInput()) {
        input.UngetChar(c);
    }
    return space_encountered;
}

bool LexicalAnalyzer::IsKeyword(string s) {
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return true;
        }
    }
    return false;
}

TokenType LexicalAnalyzer::FindKeywordIndex(string s) {
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return (TokenType) (i + 1);
        }
    }
    return ERROR;
}

Token LexicalAnalyzer::ScanIdOrKeyword() {
    char c;
    input.GetChar(c);

    if (isalpha(c)) {
        tmp.lexeme = "";
        while (!input.EndOfInput() && isalnum(c)) {
            tmp.lexeme += c;
            input.GetChar(c);
        }
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.line_no = line_no;
        if (IsKeyword(tmp.lexeme)) {
            tmp.token_type = FindKeywordIndex(tmp.lexeme);
        } else {
            tmp.token_type = ID;
        }
    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}

TokenType LexicalAnalyzer::UngetToken(Token tok) {
    tokens.push_back(tok);;
    return tok.token_type;
}

Token LexicalAnalyzer::GetToken() {
    char c;

    // if there are tokens that were previously
    // stored due to UngetToken(), pop a token_1 and
    // return it without reading from input
    if (!tokens.empty()) {
        tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }

    SkipSpace();
    SkipComment();
    SkipSpace();

    tmp.lexeme = "";
    tmp.line_no = line_no;
    input.GetChar(c);
    switch (c) {
        case '=':
            tmp.token_type = EQUAL;
            return tmp;
        case ':':
            tmp.token_type = COLON;
            return tmp;
        case ',':
            tmp.token_type = COMMA;
            return tmp;
        case ';':
            tmp.token_type = SEMICOLON;
            return tmp;
        case '{':
            tmp.token_type = LBRACE;
            return tmp;
        case '}':
            tmp.token_type = RBRACE;
            return tmp;
        default:
            if (isdigit(c)) {
                input.UngetChar(c);
                tmp.token_type = ERROR;
                return tmp;
            } else if (isalpha(c)) {
                input.UngetChar(c);
                return ScanIdOrKeyword();
            } else if (input.EndOfInput())
                tmp.token_type = END_OF_FILE;
            else
                tmp.token_type = ERROR;

            return tmp;
    }
}

bool LexicalAnalyzer::SkipComment() {
    char c;
    bool comment = false;
    input.GetChar(c);
    if (input.EndOfInput()) {
        input.UngetChar(c);
        return comment;
    }
    if (c == '/') {
        input.GetChar(c);
        if (c == '/') {
            comment = true;
            while (c != '\n') {
                input.GetChar(c);
            }
            line_no += 1;
            SkipComment();
        } else {
            // We know that comments have two '//'
            exit(1);
        }
    } else {
        input.UngetChar(c);
    }
    return comment;
}

int LexicalAnalyzer::parseGlobalVariables() {
    token_1 = lexer.GetToken();
    if (token_1.token_type == ID) {
        lexer.UngetToken(token_1);
        lexer.parseVariables();
        token_1 = lexer.GetToken();
        if (token_1.token_type != SEMICOLON) {
            Error();
        }
    } else {
        Error();
    }
    return 0;
}

int LexicalAnalyzer::parseStatements() {
    token_1 = lexer.GetToken();
    if (token_1.token_type == ID) {
        lexer.UngetToken(token_1);
        lexer.parseStatement();
        token_2 = lexer.GetToken();
        if (token_2.token_type == ID) {
            lexer.UngetToken(token_2);
            lexer.parseStatements();
            return 0;
        } else if (token_2.token_type == RBRACE) {
            // we are most likely exiting a scope...
            lexer.UngetToken(token_2);
            return 0;
        } else {
            Error();
        }
    } else {
        Error();
    }
    return 0;
}

int LexicalAnalyzer::parseStatement() {
    debugPrint("Parse Statement Reach");
    token_1 = lexer.GetToken();
    if (token_1.token_type == ID) { // token_1 ? ?
        token_2 = lexer.GetToken();
        if (token_2.token_type == EQUAL) { // lhs(token_1) =(token_2) ?
            token_3 = lexer.GetToken();
            // Next statement should be id, if not error
            if (token_3.token_type == ID) { // lhs(token_1) =(token_2)  (rhs)token_3
                createAssignment(token_1, token_3);
                token_1 = lexer.GetToken();
                // Statement should end in semicolon, if not error
                if (token_1.token_type == SEMICOLON) { // lhs(token_1) =(token_2)  (rhs)token_3;
                    debugPrint("Ends in Semi Colon");
                    return 0;
                } else {
                    Error();
                }
            } else {
                Error();
            }
        } else if (token_2.token_type == LBRACE) {
            // token_1 { -> meaning we will be entering a scope because of left brace
            currScope = const_cast<char *>((token_1.lexeme).c_str());
            lexer.UngetToken(token_2);
            lexer.UngetToken(token_1);
            lexer.parseScope();
        } else {
            // failure
            Error();
        }
    } else {
        Error();
    }
    return 0;
}

int LexicalAnalyzer::parseVariables() {
    token_1 = lexer.GetToken();

    int lexemeSize = sizeof(token_1.lexeme);
    int tokenSize = sizeof(token_1);

    char *variable = (char *) malloc(lexemeSize);
    memcpy(variable, token_1.lexeme.c_str(), tokenSize);

    add(variable);

    SymbolNode *temp1 = symbolNode;

    // Go down the node list
    while (temp1 != NULL) {
        temp1 = temp1->next;
    }

    if (token_1.token_type == ID) {
        token_1 = lexer.GetToken();
        // if next token is comma most likely there will be more variables
        if (token_1.token_type == COMMA) {
            // we then parse recursively
            lexer.parseVariables();
            return 0;
        }
        // if next token is semicon, we reach end of parsing variables
        else if (token_1.token_type == SEMICOLON) {
            UngetToken(token_1);
            return 0;
        } else {
            Error();
        }
    } else {
        Error();
    }
    return 0;
}

int LexicalAnalyzer::parseVariable(TokenType visibility) {
    token_1 = lexer.GetToken();
    if (token_1.token_type == visibility) {
        currLevel = visibility;
        token_1 = lexer.GetToken();
        // after visibility scope, ie public or private, next character should be a colon if not error
        if (token_1.token_type == COLON) {
            token_1 = lexer.GetToken();
            // we identify that there are possibily more variables to parse
            // if not then thre is an error because after colon should logically be an id
            if (token_1.token_type == ID) {
                lexer.UngetToken(token_1);
                lexer.parseVariables();
                token_1 = lexer.GetToken();
                // if it is not semicolon then we have an error
                if (token_1.token_type != SEMICOLON) {
                    Error();
                }
            } else {
                Error();
            }
        } else {
            Error();
        }
    } else if (token_1.token_type != visibility || token_1.token_type == ID) {
        // since we know that token_type should be the visibility that we're looking for
        // we know that if it is an ID then that is not a valid case, we should then
        // exit the scenario
        lexer.UngetToken(token_1);
    } else {
        Error();
    }
    return 0;
}

int LexicalAnalyzer::parseScope() {
    token_1 = lexer.GetToken();

    if (token_1.token_type == ID) {
        // entering scope: token_1.lexeme contains the scope we are entering
        string slexeme = token_1.lexeme;
        currScope = const_cast<char *>(slexeme.c_str());
        // next character should be a left bracket
        token_1 = lexer.GetToken();

        if (token_1.token_type == LBRACE) {
            debugPrint("entering scope: lbrace");
            lexer.parseVariable(PUBLIC);
            debugPrint("parsing public variables");
            lexer.parseVariable(PRIVATE);
            debugPrint("parsing private variables");
            lexer.parseStatements();
            debugPrint("parsing statement list");
            token_1 = lexer.GetToken();

            if (token_1.token_type == RBRACE) {

                debugPrint("exiting scope: rbrace");
                remove(currScope);
                debugPrint("deleting list");
                if (input.EndOfInput()) {

                    debugPrint("End Of Input");
                    return 0;
                } else {
                    debugPrint("Not End Of Input");
                }
                token_1 = lexer.GetToken();

                if (token_1.token_type == END_OF_FILE) {
                    remove(currScope);
                } else {
                    UngetToken(token_1);
                }
                return 0;
            } else {
                Error();
            }
        } else {
            Error();
        }
    } else {
        Error();
    }
    return 0;
}

int LexicalAnalyzer::handleGlobalVariablesAndScope() {
    token_2 = lexer.GetToken();
    debugPrint("Check if comma or semi colon, if it is, parse global variables");
    if (token_2.token_type == COMMA || token_2.token_type == SEMICOLON) {
        lexer.UngetToken(token_2);
        lexer.UngetToken(token_1);
        debugPrint("First parse global variables if there are any");
        lexer.parseGlobalVariables();
        debugPrint("Parse scope");
        lexer.parseScope();
        debugPrint("parsed reached");
    } else if (token_2.token_type == LBRACE) {
        debugPrint("There are no global variables so we parse scope");
        lexer.UngetToken(token_2);
        lexer.UngetToken(token_1);
        lexer.parseScope();
        debugPrint("Parsed scope");
    } else {
        Error();
    }
    return 0;
}

int LexicalAnalyzer::start() {
    token_1 = lexer.GetToken();
    if (token_1.token_type == ID) {
        handleGlobalVariablesAndScope();
    } else {
        cout << "Syntax Error" << endl;
    }
    debugPrint("END");
    results();
    return 0;
}

int main() {
    lexer.start();
}
