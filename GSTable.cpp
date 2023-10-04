#include <iostream>
#include <string.h>
#include "ASTree.h"
#include "GSTable.h"
using namespace std;

extern void CompileError(string);
GSNode* head=nullptr;
uint16_t sp = 0xFFFF;

void RedeclarationError(string name){
    CompileError(name + " is getting re-declared");
}

GSNode* Lookup(char *name){
    GSNode* temp=head;
    while(temp && strcmp(temp->name,name))
        temp=temp->next;
    return temp;
}

bool Install(char *name, DataType type, int size){
    if(Lookup(name)!=NULL)
        return false;
    GSNode* node = (GSNode*)malloc(sizeof(GSNode));
    node->name=name;
    node->dtype=type;
    node->size=size;
    sp-=size;
    node->bind=sp;
    node->next=head;
    head = node;
    return true;
}

void DeclareList(ASNode* type, ASNode* list){
    if (list->nodeType == CONNECTOR){
        DeclareList(type, list->left);
        DeclareList(type, list->right);
    }
    else {
        char* name = list->varName;
        int size = 1;
        if(type->dataType == STR_TYPE)
            size = 10;
        if (list->nodeType == ARRAY_VARIABLE){
            name = list->left->varName;
            size *= list->right->val;
        }
        if(!Install(name, type->dataType, size))
            RedeclarationError(name);
    }
}
