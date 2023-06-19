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

#include "lexer.h"


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



struct Variable {
    char* name; // name of variable
    char* scope; // what scope are we in
    int level; // 0 - global, 1 - public, 2 - private
};

struct SymbolNode {
    Variable* item;
    SymbolNode* prev;
    SymbolNode* next;
};

struct Assignment {
    std::string lhs;
    std::string rhs;
    std::string scope1;
    std::string scope2;
};


std::vector<struct Assignment> assignments;

struct SymbolNode* symbolNode;
struct SymbolNode *tempNode;
struct SymbolNode *newNode;

char *currScope = "::";
int currLevel = 0;

bool debug = false;

void debugPrint(std::string text) {
    if(debug) {
        std::cout << text << std::endl;
    }
}

void Error() {
    std::cout << "Syntax Error" << std::endl;
    exit(1);
}
void createSymbolTable(char* lexeme) {
    // Allocate space for new node and variable
    symbolNode = (SymbolNode *) malloc(sizeof(SymbolNode));
    struct Variable *newVariable = (Variable *) malloc(sizeof(Variable));
    symbolNode->item = newVariable;
    symbolNode->next = NULL;
    symbolNode->prev = NULL;

    //Create and allocate space for string to be copied to
    int len = strlen(lexeme);
    symbolNode->item->name = new char[len + 1];
    strcpy(symbolNode->item->name, lexeme);
    symbolNode->item->name[len] = '\0'; // set last char to be end of feed


    symbolNode->item->scope = currScope;
    symbolNode->item->level = currLevel;

    newNode = symbolNode;
    tempNode = symbolNode;
}

void appendToSymbolTable(char* lexeme) {
    // create another pointer to struct temp1 which will point to top of the symbolNode
    tempNode = symbolNode;

    // navigate down the chain of nodes
    while (tempNode->next != NULL) {
        tempNode = tempNode->next;
    }

    newNode = (SymbolNode *) malloc(sizeof(SymbolNode));
    struct Variable *newVariable = (Variable *) malloc(sizeof(Variable));
    newNode->item = newVariable;
    //set value of next new node to null
    newNode->next = NULL;

    // set value of prev node to last element of tempNode
    newNode->prev = tempNode;
    // set the second to last node to this new node (newNode will be last node in chain)
    tempNode->next = newNode; // now newNode is at top of the stack;

    //Create and allocate space for string to be copied to
    int len = strlen(lexeme);
    newNode->item->name = new char[len + 1];
    strcpy(newNode->item->name, lexeme);
    newNode->item->name[len] = '\0'; // set last char to be end of feed

    newNode->item->scope = currScope;
    newNode->item->level = currLevel;
}


void add(char *lexeme) {
    if (symbolNode == NULL) {
        createSymbolTable(lexeme);
    } else {
        appendToSymbolTable(lexeme);
    }
}

void remove(char *lexeme) {
    if (symbolNode == NULL) {
        return; // do nothing
    }
    while (newNode->item->scope == lexeme && isalpha(lexeme[0]) == 1) {
        tempNode->next = NULL;
        newNode->prev = NULL;
        if (tempNode->prev != NULL) {
            // take the preview node of tempNode
            // set the next Node of the previous temp node to the new Node
            tempNode = tempNode->prev;
            newNode = tempNode->next;
            newNode->next = NULL;
            return;
        } else if (tempNode == newNode) {
            tempNode = NULL;
            newNode = NULL;
            return;
        } else {
            tempNode->next = NULL;
            newNode->prev = NULL;
            newNode = tempNode;
            tempNode->next = NULL;
        }
    }
    // take the scope of newNode
    currScope = newNode->item->scope;
}


Assignment createAssignment(Token lhs_token, Token rhs_token) {
    struct SymbolNode *temp = newNode;
    struct SymbolNode *temp1 = newNode;
    char *scope1 = (char *) malloc(sizeof(char) * 20);
    char *scope2 = (char *) malloc(sizeof(char) * 20);
    while (temp != NULL) {
        // We keep going up the nodes until we find that lexeme and node name are equal
        if (temp->item->name == lhs_token.lexeme) {
            // when lhs_token.lexeme == symbol table item, we check if
            if (currScope != temp->item->scope && temp->item->level == PRIVATE) { // when level == PRIVATE SCOPE
                // Scopes are not the same we need to go up the table
                temp = temp->prev;
                continue;
            } else {
                // We know that the name of the item->name && lhs_token.lexeme are the same
                scope1 = temp->item->scope;
                // So we assign the scope of the item to scope1 and exit
                break;
            }
        }
        // Go back up the list
        temp = temp->prev;
    }
    while (temp1 != NULL) {
        if (temp1->item->name == rhs_token.lexeme) {
            if (temp1->item->level == 2 && currScope != temp1->item->scope) {
                temp1 = temp1->prev;
                continue;

            } else {
                scope2 = temp1->item->scope;
                // We found the scope
                break;
            }
        }
        // same like other token_1 we travel up the nodes
        temp1 = temp1->prev;

    }
    // if we know that the length of scope1 == 0 then we know that the scope is unknown
    if (strlen(scope1) == 0) {
        scope1 = new char[2];
        scope1[0] = '?';
        scope1[1] = '\0';
    }
    if (strlen(scope2) == 0) {
        scope2 = new char[2];
        scope2[0] = '?';
        scope2[1] = '\0';
    }
    // now we Aasign our scopes that we found from lhs (token) & rhs (token3) to lhs and rhs accordingly
    struct Assignment temp_assignment;
    temp_assignment.lhs = lhs_token.lexeme;
    temp_assignment.rhs = rhs_token.lexeme;
    temp_assignment.scope1 = scope1;
    temp_assignment.scope2 = scope2;
    assignments.push_back(temp_assignment);
    return temp_assignment;
}

void results() {
    for (int i = 0; i < assignments.size(); i++) {
        std::cout << assignments[i].scope1;

        if (::strcmp(assignments[i].scope1.c_str(), "::") != 0) {
            std::cout << ".";
        }

        std::cout << assignments[i].lhs << " = " << assignments[i].scope2;

        if (::strcmp(assignments[i].scope2.c_str(), "::") != 0) {
            std::cout << ".";
        }

        std::cout << assignments[i].rhs << std::endl;
    }
}


LexicalAnalyzer::LexicalAnalyzer() {
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}

bool LexicalAnalyzer::SkipSpace() {
    char c;
    bool space_encountered = false;

    if (input.EndOfInput()) {
        return false;
    }
    do {
        input.GetChar(c);
        line_no += (c == '\n');
        if (isspace(c)) {
            space_encountered = true;
        }
    } while (!input.EndOfInput() && isspace(c));

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
        Error();
    }
    debugPrint("END");
    results();
    return 0;
}

int main() {
    lexer.start();
}
