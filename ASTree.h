#include "DS.h"
#ifndef ASTREE_H
#define ASTREE_H

// Make a leaf numeric literal node
ASNode* makeLeafNode(int);

// Make a leaf boolean literal node
ASNode* makeLeafNode(bool);

// Make a leaf char literal node
ASNode* makeLeafNode(char);

// Make a leaf identifier node
ASNode* makeVarNode(char*);

/*
Link symbol to the node
returns NULL if undeclared variable
*/
ASNode* linkSymbol(ASNode*);

// Make a leaf identifier node with size
ASNode* makeArrayNode(char* id, int size);

// Make a data type leaf node
struct ASNode* makeDataTypeNode(DataType);

// Make an unary operator node
struct ASNode* makeOperatorNode(char c, struct ASNode *child);

// Make a binary operator node
struct ASNode* makeOperatorNode(char c, struct ASNode *l, struct ASNode *r);

// Make a binary logical operator node
struct ASNode* makeLogicalOperatorNode(char c, struct ASNode *l, struct ASNode *r);

// Make a function node with single child
struct ASNode* makeFunctionNode(char *c, struct ASNode *child);

// Make a control leaf node
struct ASNode* makeControlNode(char *c);

// Make a control node with with left and right branches
struct ASNode* makeControlNode(char *c, struct ASNode *l, struct ASNode *r);

// Make an operator node with left and right branches
struct ASNode* makeConnectorNode(struct ASNode *l, struct ASNode *r);

#endif