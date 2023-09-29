#include <iostream>
#include <cstdlib>
#include "ASTree.h"
#include "GSTable.h"
using namespace std;

ASNode* makeLeafNode(int n)
{
    ASNode* temp = (ASNode*)malloc(sizeof(ASNode));
    temp->val = n;
    temp->varName = nullptr;
    temp->symbol = nullptr;
    temp->nodeType = NUMERIC_LITERAL;
    temp->dataType = BYTE_TYPE;
    temp->left = nullptr;
    temp->right = nullptr;
    return temp;
}

ASNode* makeLeafNode(char* id)
{
    ASNode* temp = (ASNode*)malloc(sizeof(ASNode));
    temp->val = 0;
    temp->varName = id;
    temp->symbol = nullptr;
    temp->nodeType = IDENTIFIER;
    temp->dataType = INVALID_TYPE;
    temp->left = nullptr;
    temp->right = nullptr;
    return temp;
}

ASNode* linkSymbol(ASNode* node)
{
    GSNode* symbol = Lookup(node->varName);
    if(symbol==NULL)
        return NULL;
    node->dataType = symbol->dtype;
    node->symbol = symbol;
    return node;
}

ASNode* makeDataTypeNode(DataType dt)
{
    ASNode* temp = (ASNode*)malloc(sizeof(ASNode));
    temp->val = 0;
    temp->varName = nullptr;
    temp->symbol = nullptr;
    temp->nodeType = DATA_TYPE;
    temp->dataType = dt;
    temp->left = nullptr;
    temp->right = nullptr;
    return temp;
}

struct ASNode* makeOperatorNode(char c,struct ASNode *l,struct ASNode *r){
    struct ASNode *temp;
    temp = (struct ASNode*)malloc(sizeof(struct ASNode));
    temp->varName = (char *)malloc(sizeof(char));
    *(temp->varName) = c;
    temp->nodeType = OPERATOR;
    if(c=='=')
        temp->nodeType = ASSIGNMENT;
    temp->left = l;
    temp->right = r;
    return temp;
}

struct ASNode* makeOperatorNode(char *c,struct ASNode *child){
    struct ASNode *temp;
    temp = (struct ASNode*)malloc(sizeof(struct ASNode));
    temp->varName = c;
    temp->nodeType = FUNCTION_CALL;
    temp->left = child;
    temp->right = NULL;
    return temp;
}

struct ASNode* makeControlNode(char *c){
    struct ASNode *temp;
    temp = (struct ASNode*)malloc(sizeof(struct ASNode));
    temp->varName = c;
    temp->nodeType = CONTROL;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

struct ASNode* makeControlNode(char *c, struct ASNode *l, struct ASNode *r){
    struct ASNode *temp;
    temp = (struct ASNode*)malloc(sizeof(struct ASNode));
    temp->varName = c;
    temp->nodeType = CONTROL;
    temp->left = l;
    temp->right = r;
    return temp;
}

struct ASNode* makeConnectorNode(struct ASNode *l,struct ASNode *r){
    struct ASNode *temp;
    temp = (struct ASNode*)malloc(sizeof(struct ASNode));
    temp->varName = NULL;
    temp->nodeType = CONNECTOR;
    temp->left = l;
    temp->right = r;
    return temp;
}