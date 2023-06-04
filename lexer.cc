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
#include "inputbuf.h"

using namespace std;

string reserved[] = {"END_OF_FILE",
                     "IF", "WHILE", "DO", "THEN", "PRINT",
                     "PLUS", "MINUS", "DIV", "MULT",
                     "EQUAL", "COLON", "COMMA", "SEMICOLON",
                     "LBRAC", "RBRAC", "LPAREN", "RPAREN",
                     "NOTEQUAL", "GREATER", "LESS", "LTEQ", "GTEQ",
                     "DOT", "NUM", "ID", "ERROR", "REALNUM", "BASE08NUM",
                     "BASE16NUM" // TODO: Add labels for new token types here (as string)
};

#define KEYWORDS_COUNT 5
string keyword[] = {"IF", "WHILE", "DO", "THEN", "PRINT"};

bool isHexadecimal(char c) {
    if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
        return true;
    }
    return false;
}

void Token::Print() {
    cout << "{" << this->lexeme << " , "
         << reserved[(int) this->token_type] << " , "
         << this->line_no << "}\n";
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

    input.GetChar(c);
    line_no += (c == '\n');

    while (!input.EndOfInput() && isspace(c)) {
        space_encountered = true;
        input.GetChar(c);
        line_no += (c == '\n');
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
Token LexicalAnalyzer::ScanNumber()
{
    char memo[100]; // memoization technique to remember past variables
    bool isFzero = false;
    char a,b,c,d;
    string cache1, cache2, cache3;
    int position = 0, position2 = 0, isbase8 = 0;
    input.GetChar(a);
    if (isdigit(a)) {
        if (a == '0') {
            tmp.lexeme = "0";
            isFzero = true;
        } else {
            tmp.lexeme = "";
            while ((!input.EndOfInput() && isdigit(a))) {
                tmp.lexeme += a;
                input.GetChar(a);
                if(a == '8' || a == '9')
                    isbase8 = 1;
            }
            if (!input.EndOfInput()) {
                input.UngetChar(a);
            }
        }
        input.GetChar(b);
        if(b == 'x')
        {
            input.GetChar(c);
            if (c != '0') { // Not Base 8
                if (c == '1') { // Looking Based 16
                    input.GetChar(d);
                    if (d == '6') { // BASE16
                        tmp.lexeme = tmp.lexeme + b + c + d;
                        tmp.token_type = BASE16NUM;
                        tmp.line_no = line_no;
                        return tmp;
                    } else {
                        input.UngetChar(d);
                    }
                    input.UngetChar(c);
                }
            } else {
                input.GetChar(d);
                if (d == '8' && isbase8 == 0) { //Is likely Base8
                    tmp.lexeme = tmp.lexeme + b + c + d;
                    tmp.token_type = BASE08NUM;
                    tmp.line_no = line_no;
                    return tmp;
                } else {
                    input.UngetChar(d);
                }
                input.UngetChar(c);
            }
            input.UngetChar(b);
        }
        else if(b == '.') { // Check if Decimal
            input.GetChar(c);
            if(isdigit(c))
            {
                while (isdigit(c) && !input.EndOfInput())
                {
                    position++; // count digits
                    if(c == '0')
                        position2++; // count zeros only
                    cache2 += c; // store value into cache2
                    input.GetChar(c);
                }
                if (!input.EndOfInput())
                {
                    input.UngetChar(c);
                }
                if (position != position2 || !isFzero) {
                    input.UngetString(cache2);
                    input.GetChar(c);
                    tmp.lexeme += '.';
                    while (!input.EndOfInput() && (isdigit(c))) {
                        tmp.lexeme += c;
                        input.GetChar(c);
                    }
                    if (!input.EndOfInput()) {
                        input.UngetChar(c);
                    }
                    tmp.token_type = REALNUM;
                    tmp.line_no = line_no;
                    return tmp;
                } else {
                    input.UngetString(cache2);
                }
            }
            else
            {
                input.UngetChar(c);
            }
            input.UngetChar(b);
        }
        else if(isHexadecimal(a)) { // Check A-F
            input.UngetChar(b);
            input.GetChar(memo[position]);
            while((isHexadecimal(memo[position]) && !input.EndOfInput()) || isdigit(memo[position]))
            {
                position++;
                position2++;
                input.GetChar(memo[position]);
            }
            if (memo[position] == 'x') {
                input.GetChar(memo[position + 1]);
                if (memo[position + 1] != '1') {
                    input.UngetChar(memo[position + 1]);
                } else {
                    input.GetChar(memo[position + 2]);
                    if (memo[position + 2] != '6') {
                        input.UngetChar(memo[position + 2]);
                        input.UngetChar(memo[position + 1]);
                    } else {
                        while (position > -3) {
                            input.UngetChar(memo[position + 2]);
                            position--;
                        }
                        while (position2 > -3) {
                            position2--;
                            input.GetChar(a);
                            tmp.lexeme += a;
                        }
                        tmp.token_type = BASE16NUM;
                        tmp.line_no = line_no;
                        return tmp;
                    }
                }
            }
            while(position > 0)
            {
                input.UngetChar(memo[position]);
                position--;
            }
            input.UngetChar(b);
        }
        else
        {
            input.UngetChar(b);
        }
        tmp.token_type = NUM;
        tmp.line_no = line_no;
        return tmp;
    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(a);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
        tmp.line_no = line_no;
        return tmp;
    }
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
        if (IsKeyword(tmp.lexeme))
            tmp.token_type = FindKeywordIndex(tmp.lexeme);
        else
            tmp.token_type = ID;
    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}

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
TokenType LexicalAnalyzer::UngetToken(Token tok) {
    tokens.push_back(tok);;
    return tok.token_type;
}

Token LexicalAnalyzer::GetToken() {
    char c;

    // if there are tokens that were previously
    // stored due to UngetToken(), pop a token and
    // return it without reading from input
    if (!tokens.empty()) {
        tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }

    SkipSpace();
    tmp.lexeme = "";
    tmp.line_no = line_no;
    input.GetChar(c);
    switch (c) {
        case '.':
            tmp.token_type = DOT;
            return tmp;
        case '+':
            tmp.token_type = PLUS;
            return tmp;
        case '-':
            tmp.token_type = MINUS;
            return tmp;
        case '/':
            tmp.token_type = DIV;
            return tmp;
        case '*':
            tmp.token_type = MULT;
            return tmp;
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
        case '[':
            tmp.token_type = LBRAC;
            return tmp;
        case ']':
            tmp.token_type = RBRAC;
            return tmp;
        case '(':
            tmp.token_type = LPAREN;
            return tmp;
        case ')':
            tmp.token_type = RPAREN;
            return tmp;
        case '<':
            input.GetChar(c);
            if (c == '=') {
                tmp.token_type = LTEQ;
            } else if (c == '>') {
                tmp.token_type = NOTEQUAL;
            } else {
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = LESS;
            }
            return tmp;
        case '>':
            input.GetChar(c);
            if (c == '=') {
                tmp.token_type = GTEQ;
            } else {
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = GREATER;
            }
            return tmp;
        default:

            if (isdigit(c)) {
                input.UngetChar(c);
                return ScanNumber();
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

int main() {
    LexicalAnalyzer lexer;
    Token token;

    token = lexer.GetToken();
    token.Print();
    while (token.token_type != END_OF_FILE) {
        token = lexer.GetToken();
        token.Print();
    }
}
