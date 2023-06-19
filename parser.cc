#include "parser.h"

void Parser::debugPrint(std::string text) {
    if (debug) {
        std::cout << text << std::endl;
    }
}

void Parser::Error() {
    std::cout << "Syntax Error" << std::endl;
    exit(1);
}

void Parser::createSymbolTable(char *lexeme) {
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

void Parser::appendToSymbolTable(char *lexeme) {
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


void Parser::add(char *lexeme) {
    if (symbolNode == NULL) {
        createSymbolTable(lexeme);
    } else {
        appendToSymbolTable(lexeme);
    }
}

void Parser::remove(char *lexeme) {
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


Assignment Parser::createAssignment(Token lhs_token, Token rhs_token) {
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
    temp_assignment.scope1 = static_cast<char *>(malloc(sizeof(scope1)));
    memcpy(temp_assignment.scope1, scope1, sizeof (scope1));
    temp_assignment.scope2 = static_cast<char *>(malloc(sizeof(scope2)));
    memcpy(temp_assignment.scope2, scope2, sizeof (scope2));
    //temp_assignment.scope2 = scope2;
    //temp_assignment.scope2 = scope2;
    assignments.push_back(temp_assignment);
    return temp_assignment;
}

void Parser::results() {
    for (int i = 0; i < assignments.size(); i++) {
        std::cout << assignments[i].scope1;

        if (::strcmp(assignments[i].scope1, "::") != 0) {
            std::cout << ".";
        }

        std::cout << assignments[i].lhs << " = " << assignments[i].scope2;

        if (::strcmp(assignments[i].scope2, "::") != 0) {
            std::cout << ".";
        }

        std::cout << assignments[i].rhs << std::endl;
    }
}