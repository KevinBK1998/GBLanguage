#include <cstdlib>
#include "exprtree.h"
struct tnode* makeLeafNode(int n)
{
    struct tnode *temp;
    temp = (struct tnode*)malloc(sizeof(struct tnode));
    temp->varName = NULL;
    temp->val = n;
    temp->nodeType = NUMERIC_LITERAL;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

struct tnode* makeLeafNode(char* id)
{
    struct tnode *temp;
    temp = (struct tnode*)malloc(sizeof(struct tnode));
    temp->varName = id;
    temp->nodeType = IDENTIFIER;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

struct tnode* makeOperatorNode(char c,struct tnode *l,struct tnode *r){
    struct tnode *temp;
    temp = (struct tnode*)malloc(sizeof(struct tnode));
    temp->varName = (char *)malloc(sizeof(char));
    *(temp->varName) = c;
    temp->nodeType = OPERATOR;
    temp->left = l;
    temp->right = r;
    return temp;
}

struct tnode* makeOperatorNode(char *c,struct tnode *child){
    struct tnode *temp;
    temp = (struct tnode*)malloc(sizeof(struct tnode));
    temp->varName = c;
    temp->nodeType = FUNCTION_CALL;
    temp->left = child;
    temp->right = NULL;
    return temp;
}

struct tnode* makeConnectorNode(struct tnode *l,struct tnode *r){
    struct tnode *temp;
    temp = (struct tnode*)malloc(sizeof(struct tnode));
    temp->varName = NULL;
    temp->nodeType = CONNECTOR;
    temp->left = l;
    temp->right = r;
    return temp;
}