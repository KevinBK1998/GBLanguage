#include "DS.h"
#ifndef ASTREE_H
#define ASTREE_H

// Make a leaf numeric literal node
ASNode* makeLeafNode(int n);

// Make a leaf identifier node
ASNode* makeLeafNode(char* id);

// Make a data type leaf node
struct ASNode* makeDataTypeNode(DataType);

// Make an unary operator node
struct ASNode* makeOperatorNode(char c, struct ASNode *child);

// Make a binary operator node
struct ASNode* makeOperatorNode(char c, struct ASNode *l, struct ASNode *r);

// Make a function node with single child
struct ASNode* makeOperatorNode(char *c, struct ASNode *child);

// Make a control leaf node
struct ASNode* makeControlNode(char *c);

// Make a control node with with left and right branches
struct ASNode* makeControlNode(char *c, struct ASNode *l, struct ASNode *r);

// Make an operator node with left and right branches
struct ASNode* makeConnectorNode(struct ASNode *l, struct ASNode *r);

#endif