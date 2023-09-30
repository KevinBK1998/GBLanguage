#include <stdint.h>
#ifndef DATASTRUCT_H
#define DATASTRUCT_H

enum NodeType {
    NUMERIC_LITERAL,
    DATA_TYPE,
    IDENTIFIER,
    ARRAY_VARIABLE,
    ASSIGNMENT,
    OPERATOR,
    LOGICAL_OPERATOR,
    FUNCTION_CALL,
    CONTROL,
    CONNECTOR,
};

enum IO_TYPE{
    READ_CALL,
    WRITE_CALL,
    WRITE_NL_CALL,
    ASCII_LOAD,
};

enum DataType{
    INVALID_TYPE,
    VOID_TYPE,
    BYTE_TYPE,
    BOOL_TYPE,
};

typedef struct LoopLabel{
    bool inLoop;
    char *continueLabel;
    char *breakLabel;
}LoopLabel;

typedef struct GSNode{
    char *name;
    DataType dtype;
    int size;
    uint16_t bind;
    struct GSNode *next;
}GSNode;

typedef struct ASNode{
    // value of the leaf node
    int val;
    // name of the identifier node
    char* varName;
    GSNode* symbol;
    // type of node
    NodeType nodeType;
    DataType dataType;
    struct ASNode *left,*right; //left and right branches
}ASNode;

#endif