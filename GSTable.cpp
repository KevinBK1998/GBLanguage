#include <iostream>
#include <string.h>
#include "ASTree.h"
#include "GSTable.h"
using namespace std;

GSNode* head=nullptr;
uint16_t sp = 0xFFFE;

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
    node->bind=sp;
    sp-=size;
    node->next=head;
    head = node;
    return true;
}
