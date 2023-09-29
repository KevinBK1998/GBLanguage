#include <iostream>
#include <string.h>
#include "ASTree.h"
#include "GSTable.h"
using namespace std;

GSNode* head=nullptr;

GSNode* Lookup(char *name){
    GSNode* temp=head;
    while(temp && strcmp(temp->name,name))
        temp=temp->n;
    return temp;
}

void Install(char *name, DataType type, int size){
    GSNode* node = (GSNode*)malloc(sizeof(GSNode));
    node->name=name;
    node->dtype=type;
    node->size=size;
    node->bind=4096;
    node->n=head;
    head = node;
}

void DeclareList(ASNode* type, ASNode* list){
    if (list->nodeType == CONNECTOR){
        DeclareList(type, list->left);
        DeclareList(type, list->right);
    }
    else 
        Install(list->varName, type->dataType, 1);
}