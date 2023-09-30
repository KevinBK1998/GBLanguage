#include "DS.h"
#ifndef GSTABLE_H
#define GSTABLE_H

// Lookup variable in symbol table
GSNode* Lookup(char*);
// install variable in symbol table
bool Install(char*, DataType, int);
// Declare a list of variables in symbol table
void DeclareList(ASNode*, ASNode*);

#endif