#ifndef ASTREE_H
#define ASTREE_H

enum NodeType{
    NUMERIC_LITERAL,
    IDENTIFIER,
    ASSIGNMENT,
    OPERATOR,
    FUNCTION_CALL,
    CONTROL,
    CONNECTOR,
};

enum DataType{
    BYTE_TYPE,
};

enum IO_TYPE{
    READ_CALL,
    WRITE_CALL,
    WRITE_NL_CALL,
    ASCII_LOAD,
};

typedef struct tnode{
    // value of the leaf node
    int val;
    // name of the identifier node
    char* varName;
    // type of node
    NodeType nodeType;
    DataType dataType;
    struct tnode *left,*right; //left and right branches
} tnode;

typedef struct LoopLabel{
    bool inLoop;
    char *continueLabel;
    char *breakLabel;
}LoopLabel;

// Make a leaf numeric literal node
struct tnode* makeLeafNode(int n);

// Make a leaf identifier node
struct tnode* makeLeafNode(char* id);

// Make a data type leaf node
struct tnode* makeDataTypeNode(DataType);

// Make an unary operator node
struct tnode* makeOperatorNode(char c, struct tnode *child);

// Make a binary operator node
struct tnode* makeOperatorNode(char c, struct tnode *l, struct tnode *r);

// Make a function node with single child
struct tnode* makeOperatorNode(char *c, struct tnode *child);

// Make a control leaf node
struct tnode* makeControlNode(char *c);

// Make a control node with with left and right branches
struct tnode* makeControlNode(char *c, struct tnode *l, struct tnode *r);

// Make an operator node with left and right branches
struct tnode* makeConnectorNode(struct tnode *l, struct tnode *r);

#endif