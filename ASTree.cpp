#include <iostream>
#include <cstdlib>
#include <string.h>
#include "ASTree.h"
#include "GSTable.h"
using namespace std;

extern void CompileError(string);

void UndeclaredError(string name){
    CompileError(name + " is not declared");
}

ASNode* makeLeafNode(int n)
{
    ASNode* temp = (ASNode*)malloc(sizeof(ASNode));
    temp->val = n;
    temp->varName = nullptr;
    temp->symbol = nullptr;
    temp->nodeType = LITERAL;
    temp->dataType = BYTE_TYPE;
    temp->left = nullptr;
    temp->right = nullptr;
    return temp;
}

ASNode* makeLeafNode(bool flag)
{
    ASNode* temp = (ASNode*)malloc(sizeof(ASNode));
    temp->val = flag;
    temp->varName = nullptr;
    temp->symbol = nullptr;
    temp->nodeType = LITERAL;
    temp->dataType = BOOL_TYPE;
    temp->left = nullptr;
    temp->right = nullptr;
    return temp;
}

ASNode* makeLeafNode(char c)
{
    ASNode* temp = (ASNode*)malloc(sizeof(ASNode));
    temp->val = 0;
    temp->varName = (char *)malloc(sizeof(char));
    *(temp->varName) = c;
    temp->symbol = nullptr;
    temp->nodeType = LITERAL;
    temp->dataType = CHAR_TYPE;
    temp->left = nullptr;
    temp->right = nullptr;
    return temp;
}

ASNode* makeVarNode(char* id)
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
        UndeclaredError(node->varName);
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

ASNode* makeArrayNode(ASNode* id, ASNode* size)
{
    ASNode* temp = (ASNode*)malloc(sizeof(ASNode));
    temp->val = 0;
    temp->varName = nullptr;
    temp->symbol = nullptr;
    temp->nodeType = ARRAY_VARIABLE;
    temp->dataType = id->dataType;
    temp->left = id;
    temp->right = size;
    return temp;
}

struct ASNode* makeOperatorNode(char c,struct ASNode *l,struct ASNode *r){
    struct ASNode *temp;
    temp = (struct ASNode*)malloc(sizeof(struct ASNode));
    temp->val = 0;
    temp->varName = (char *)malloc(sizeof(char));
    *(temp->varName) = c;
    temp->symbol = nullptr;
    temp->nodeType = OPERATOR;
    if(c=='=')
        temp->nodeType = ASSIGNMENT;
    temp->dataType = BYTE_TYPE;
    temp->left = l;
    temp->right = r;
    return temp;
}

struct ASNode* makeLogicalOperatorNode(char c,struct ASNode *l,struct ASNode *r){
    struct ASNode *temp;
    temp = (struct ASNode*)malloc(sizeof(struct ASNode));
    temp->val = 0;
    temp->varName = (char *)malloc(sizeof(char));
    *(temp->varName) = c;
    temp->symbol = nullptr;
    temp->nodeType = LOGICAL_OPERATOR;
    temp->dataType = BOOL_TYPE;
    temp->left = l;
    temp->right = r;
    return temp;
}

struct ASNode* makeFunctionNode(char *c,struct ASNode *child){
    struct ASNode *temp;
    temp = (struct ASNode*)malloc(sizeof(struct ASNode));
    temp->val = 0;
    temp->varName = c;
    temp->symbol = nullptr;
    temp->nodeType = FUNCTION_CALL;
    temp->dataType = VOID_TYPE;
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
