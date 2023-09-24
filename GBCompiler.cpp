#include <iostream>
#include <string.h>
#include "exprtree.h"
using namespace std;

char reg = 0;
char label[] = "A_0";
bool loadedAscii = false;
FILE* target_file;

enum FunctionType{
    READ_CALL,
    WRITE_CALL,
    ASCII_LOAD,
};

char getReg(){
    int temp = reg;
    char freeReg = 'A';
    do{
        temp /= 2;
        freeReg++;
    }while (temp%2);
    reg |= (1 << (freeReg -'A'));
    if (freeReg>'H'){
        cout<<"OOM"<<endl;
        exit(1);
    }
    return freeReg == 'G' ? 'L' :freeReg;
}

char* getLabel(){
    char labelChar = label[0];
    int labelNum =  label[2]-'0';
    if (++labelNum>9){
        labelChar++;
        labelNum=0;
    }
    char* oldLabel = strdup(label);
    label[0] = labelChar;
    label[2] = labelNum + '0';
    return oldLabel;
}

int freeReg(char regToBeFreed){
    int temp = regToBeFreed == 'L' ? 6 : regToBeFreed - 'A';
    reg &= ~(1 << temp);
    return temp -'A';
}

int loadTOAccumulator(char load){
    fprintf(target_file, "LD A, %c\n", load);
    return 0;
}

int clearRegister(char tempReg){
    fprintf(target_file, "LD %c, 0x0\n", tempReg);
    return 0;
}

int clearAccumulator(){
    return clearRegister('A');
}

int loadFROMAccumulator(char load){
    fprintf(target_file, "LD %c, A\n", load);
    return 0;
}

char handleNumericLiteral(int literal){
    char temp = getReg();
    fprintf(target_file, "LD %c, 0x%X\n", temp, literal);
    return temp;
}

char handleIdentifierLVal(char *var){
    // cout<<var<<endl;
    char temp = getReg();
    fprintf(target_file, "LD %c, 0x%X\n", temp, var[0]+0x1F);
    return temp;
}

void readFromMemory(char address){
    if(address>'C' && reg&0b100)
        fprintf(target_file, "PUSH BC\n");
    fprintf(target_file, "LD C, %c\n", address);
    fprintf(target_file, "LD A, [HC]\n");
    if(address>'C' && reg&0b100)
        fprintf(target_file, "POP BC\n");
    fprintf(target_file, "LD %c, A\n", address);
}

char handleIdentifier(tnode* var){
    char temp = handleIdentifierLVal(var->varName);
    readFromMemory(temp);
    return temp;
}

void writeToMemory(char address, char data){
    if(address>'C' && reg&0b100)
        fprintf(target_file, "PUSH BC\n");
    fprintf(target_file, "LD A, %c\n", data);
    if (address != 'C')
        fprintf(target_file, "LD C, %c\n", address);
    fprintf(target_file, "LD [HC], A\n");
    if(address>'C' && reg&0b100)
        fprintf(target_file, "POP BC\n");
    freeReg(data);
    freeReg(address);
}

void handleAssignment(tnode* left, tnode* right){
    char r = codeGen(right);
    char l = handleIdentifierLVal(left->varName);
    writeToMemory(l,r);
}

char handleOperator(char* op, tnode* operand1, tnode* operand2){
    char l = codeGen(operand1);
    char r = codeGen(operand2);
    char* tempLabel;
    switch(*(op)){
    case '+':
        loadTOAccumulator(l);
        fprintf(target_file, "ADD A, %c\n", r);
        loadFROMAccumulator(l);
        break;
    case '*':
        clearAccumulator();
        tempLabel = getLabel();
        fprintf(target_file, "\n%s:\n", tempLabel);
        fprintf(target_file, "ADD A, %c\n", l);
        fprintf(target_file, "DEC %c\n", r);
        fprintf(target_file, "JR NZ, %s\n\n", tempLabel);
        loadFROMAccumulator(l);
        break;
    case '-':
        loadTOAccumulator(l);
        fprintf(target_file, "SUB A, %c\n", r);
        loadFROMAccumulator(l);
        break;
    case '/':
        loadTOAccumulator(l);
        fprintf(target_file, "LD %c, 0xFF\n", l);
        tempLabel = getLabel();
        fprintf(target_file, "\n%s:\n", tempLabel);
        fprintf(target_file, "INC %c\n", l);
        fprintf(target_file, "SUB A, %c\n", r);
        fprintf(target_file, "JR NC, %s\n\n", tempLabel);
        break;
    default:
        cout<<"Op:"<< op<< endl;
        exit(-1);
    }
    freeReg(r);
    return l;
}

void loadAsciiTable(){
    if (loadedAscii)
        return;
    fprintf(target_file, "//LOAD ASCII\n");
    fprintf(target_file, "LD A, 0x%X\n", ASCII_LOAD);
    fprintf(target_file, "CALL LIBRARY\n");
    loadedAscii = true;
}

void handleFunctionCalls(tnode* exp){
    loadAsciiTable();
    if (exp->varName == "write"){
        char temp = codeGen(exp->left);
        fprintf(target_file, "//WRITE\n");
        fprintf(target_file, "LD A, 0x%X\n", WRITE_CALL);
        fprintf(target_file, "LD B, %c\n", temp);
        fprintf(target_file, "CALL LIBRARY\n");
        freeReg(temp);
    }
    else{
        fprintf(target_file, "//READ\n");
        fprintf(target_file, "LD A, 0x%X\n", READ_CALL);
        fprintf(target_file, "CALL LIBRARY\n");
        char add = handleIdentifierLVal(exp->left->varName);
        char temp = getReg();
        fprintf(target_file, "LD %c, A\n", temp);
        writeToMemory(add, temp);
    }
}

char codeGen(struct tnode *t){
    // cout<< t->nodeType << endl;
    switch (t->nodeType)
    {
    case NUMERIC_LITERAL:
        return handleNumericLiteral(t->val);
    case IDENTIFIER:
        return handleIdentifier(t);
    case ASSIGNMENT:
        handleAssignment(t->left, t->right);
        break;
    case OPERATOR:
        return handleOperator(t->varName, t->left, t->right);
    case FUNCTION_CALL:
        handleFunctionCalls(t);
        break;
    case CONNECTOR:
        codeGen(t->left);
        codeGen(t->right);
        break;
    default:
    cout<<"Undefined:"<< t->varName<< endl;
        exit(-1);
        break;
    }
    return 0;
}