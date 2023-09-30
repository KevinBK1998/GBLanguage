#include <string>
#include "DS.h"
#ifndef GBCOMPILER_H
#define GBCOMPILER_H
using namespace std;

// Generate Assembly language code with label details for loop
char GenerateCode(ASNode *t, LoopLabel loopLabelDetails);

// Generate Assembly language code
char GenerateCode(ASNode *t);

// returns string format for NodeType
string NodeTypeString(ASNode* t){
    switch (t->nodeType)
    {
    case NUMERIC_LITERAL:
        return to_string(t->val);
    case ARRAY_VARIABLE:
        return "ARRAY";
    case CONNECTOR:
        return "CONNECTOR";
    default:
        return t->varName;
    }
}

// returns string format for DataType
string DataTypeString(DataType dataType){
    switch (dataType)
    {
    case BYTE_TYPE:
        return "BYTE";
    case BOOL_TYPE:
        return "BOOL";
    default:
        return "INVALID";
    }
}

#endif