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

void loadTOAccumulator(char load){
    fprintf(target_file, "LD A, %c\n", load);
}

void clearRegister(char tempReg){
    fprintf(target_file, "LD %c, 0x0\n", tempReg);
}

void clearAccumulator(){
    fprintf(target_file, "SUB A, A\n");
}

void loadFROMAccumulator(char load){
    fprintf(target_file, "LD %c, A\n", load);
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

void loadRegToReg(char from, char to){
    loadTOAccumulator(from);
    loadFROMAccumulator(to);
}

void readFromMemory(char address){
    fprintf(target_file, "PUSH BC\n");
    if(address!='C')
        loadRegToReg(address, 'C');
    fprintf(target_file, "LD A, [HC]\n");
    fprintf(target_file, "POP BC\n");
    fprintf(target_file, "LD %c, A\n", address);
}

char handleIdentifier(tnode* var){
    char temp = handleIdentifierLVal(var->varName);
    readFromMemory(temp);
    return temp;
}

void writeToMemory(char address, char data){
    fprintf(target_file, "PUSH BC\n");
    fprintf(target_file, "PUSH DE\n");
    freeReg(address);
    freeReg(data);
    if (address != 'C'){
        if (data =='C'){
            loadRegToReg(data,'D');
            data = 'D';
        }
        loadRegToReg(address,'C');
    }
    fprintf(target_file, "LD A, %c\n", data);
    fprintf(target_file, "LD [HC], A\n");
    fprintf(target_file, "POP DE\n");
    fprintf(target_file, "POP BC\n");
}

void handleAssignment(tnode* left, tnode* right){
    char r = codeGen(right);
    char l = handleIdentifierLVal(left->varName);
    writeToMemory(l,r);
}

char* checkRegisterForZero(char tempReg){
    clearAccumulator();
    fprintf(target_file, "ADD A, %c\n", tempReg);
    char* skipLabel=getLabel();
    fprintf(target_file, "JR Z, %s\n\n", skipLabel);
    return skipLabel;
}

char handleOperator(char* op, tnode* operand1, tnode* operand2){
    char l = codeGen(operand1);
    char r = codeGen(operand2);
    char* startLoopLabel;
    char* skipLoopLabel;
    switch(*(op)){
    case '+':
        loadTOAccumulator(l);
        fprintf(target_file, "ADD A, %c\n", r);
        loadFROMAccumulator(l);
        break;
    case '-':
        loadTOAccumulator(l);
        fprintf(target_file, "SUB A, %c\n", r);
        loadFROMAccumulator(l);
        break;
    case '*':
        skipLoopLabel=checkRegisterForZero(r);
        clearAccumulator();
        startLoopLabel=getLabel(); 
        fprintf(target_file, "\n%s:\n", startLoopLabel);
        fprintf(target_file, "ADD A, %c\n", l);
        fprintf(target_file, "DEC %c\n", r);
        fprintf(target_file, "JR NZ, %s\n", startLoopLabel);
        fprintf(target_file, "\n%s:\n", skipLoopLabel);
        loadFROMAccumulator(l);
        break;
    case '/':
        skipLoopLabel=checkRegisterForZero(r);
        loadTOAccumulator(l);
        fprintf(target_file, "LD %c, 0xFF\n", l);
        startLoopLabel = getLabel();
        fprintf(target_file, "\n%s:\n", startLoopLabel);
        fprintf(target_file, "INC %c\n", l);
        fprintf(target_file, "SUB A, %c\n", r);
        fprintf(target_file, "JR NC, %s\n", startLoopLabel);
        fprintf(target_file, "\n%s:\n", skipLoopLabel);
        break;
    case '>':
        loadTOAccumulator(l);
        fprintf(target_file, "CP %c\n", r);
        startLoopLabel = getLabel();
        fprintf(target_file, "JR NC, %s\n\n", startLoopLabel);
        fprintf(target_file, "LD %c, 0x0\n", l);
        skipLoopLabel = getLabel();
        fprintf(target_file, "JR %s\n", skipLoopLabel);
        fprintf(target_file, "\n%s:\n", startLoopLabel);
        fprintf(target_file, "LD %c, 0x1\n", l);
        fprintf(target_file, "\n%s:\n", skipLoopLabel);
        break;
    case '<':
        loadTOAccumulator(l);
        fprintf(target_file, "CP %c\n", r);
        startLoopLabel = getLabel();
        fprintf(target_file, "JR C, %s\n\n", startLoopLabel);
        fprintf(target_file, "LD %c, 0x0\n", l);
        skipLoopLabel = getLabel();
        fprintf(target_file, "JR %s\n", skipLoopLabel);
        fprintf(target_file, "\n%s:\n", startLoopLabel);
        fprintf(target_file, "LD %c, 0x1\n", l);
        fprintf(target_file, "\n%s:\n", skipLoopLabel);
        break;
    case 'G':
        loadTOAccumulator(l);
        fprintf(target_file, "CP %c\n", r);
        startLoopLabel = getLabel();
        fprintf(target_file, "JR NC, %s\n\n", startLoopLabel);
        fprintf(target_file, "JR Z, %s\n\n", startLoopLabel);
        fprintf(target_file, "LD %c, 0x0\n", l);
        skipLoopLabel = getLabel();
        fprintf(target_file, "JR %s\n", skipLoopLabel);
        fprintf(target_file, "\n%s:\n", startLoopLabel);
        fprintf(target_file, "LD %c, 0x1\n", l);
        fprintf(target_file, "\n%s:\n", skipLoopLabel);
        break;
    case 'L':
        loadTOAccumulator(l);
        fprintf(target_file, "CP %c\n", r);
        startLoopLabel = getLabel();
        fprintf(target_file, "JR C, %s\n\n", startLoopLabel);
        fprintf(target_file, "JR Z, %s\n\n", startLoopLabel);
        fprintf(target_file, "LD %c, 0x0\n", l);
        skipLoopLabel = getLabel();
        fprintf(target_file, "JR %s\n", skipLoopLabel);
        fprintf(target_file, "\n%s:\n", startLoopLabel);
        fprintf(target_file, "LD %c, 0x1\n", l);
        fprintf(target_file, "\n%s:\n", skipLoopLabel);
        break;
    case 'N':
        loadTOAccumulator(l);
        fprintf(target_file, "CP %c\n", r);
        startLoopLabel = getLabel();
        fprintf(target_file, "JR NZ, %s\n\n", startLoopLabel);
        fprintf(target_file, "LD %c, 0x0\n", l);
        skipLoopLabel = getLabel();
        fprintf(target_file, "JR %s\n", skipLoopLabel);
        fprintf(target_file, "\n%s:\n", startLoopLabel);
        fprintf(target_file, "LD %c, 0x1\n", l);
        fprintf(target_file, "\n%s:\n", skipLoopLabel);
        break;
    case 'E':
        loadTOAccumulator(l);
        fprintf(target_file, "CP %c\n", r);
        startLoopLabel = getLabel();
        fprintf(target_file, "JR Z, %s\n\n", startLoopLabel);
        fprintf(target_file, "LD %c, 0x0\n", l);
        skipLoopLabel = getLabel();
        fprintf(target_file, "JR %s\n", skipLoopLabel);
        fprintf(target_file, "\n%s:\n", startLoopLabel);
        fprintf(target_file, "LD %c, 0x1\n", l);
        fprintf(target_file, "\n%s:\n", skipLoopLabel);
        break;
    default:
        cout<<"Undefined Operator : "<< op << endl;
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
        loadRegToReg(temp,'B');
        fprintf(target_file, "LD A, 0x%X\n", WRITE_CALL);
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

void handleControlStatements(tnode* statement, char* elseLabel){
    if (statement->varName == "if"){
        char temp = codeGen(statement->left);
        char* skipBlockLabel = checkRegisterForZero(temp);
        if(statement->right->nodeType != CONTROL){
            fprintf(target_file, "BRKP:");
            codeGen(statement->right);
            fprintf(target_file, "\n%s:\n", skipBlockLabel);
        }
        else
            handleControlStatements(statement->right, skipBlockLabel);
    }
    else if(statement->varName == "else"){
        codeGen(statement->left);
        char* endIfLabel = getLabel();
        fprintf(target_file, "JR %s\n", endIfLabel);
        fprintf(target_file, "\n%s:\n", elseLabel);
        codeGen(statement->right);
        fprintf(target_file, "\n%s:\n", endIfLabel);
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
    case CONTROL:
        handleControlStatements(t, NULL);
        break;
    case CONNECTOR:
        codeGen(t->left);
        codeGen(t->right);
        break;
    default:
        cout<<"Undefined Node : "<< t->nodeType << " = '" <<t->varName << "'" << endl;
        exit(-1);
        break;
    }
    return 0;
}