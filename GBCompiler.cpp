#include <iostream>
#include <string.h>
#include "GBCompiler.h"
#include "ASTree.h"
using namespace std;

extern void CompileError(string);
char reg = 0x20;
char label[] = "A_0";
bool loadedAscii = false;
FILE* target_file;

void TypeCheckError(DataType expectedType, ASNode* exp){
    DataType actualType = exp->dataType;
    string message = "Expected type to be ";
    message += DataTypeString(expectedType);
    message += " but '";
    if (exp->nodeType==LITERAL)
        message += to_string(exp->val);
    else
        message += exp->varName;
    message += "' is ";
    message += DataTypeString(actualType);
    CompileError(message);
}

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
    // cout<<"AllocateReg:"<<freeReg<<endl;
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
    // cout<<"FreeReg:"<<regToBeFreed<<endl;
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

char handleIdentifierLVal(GSNode* var){
    // cout<<var<<endl;
    char temp = getReg();
    fprintf(target_file, "LD %c, 0x%X\n", temp, var->bind & 0xFF);
    return temp;
}

char handleArrayLVal(ASNode* var, ASNode* index){
    GSNode* symbol = var->symbol;
    char temp = GenerateCode(index);
    fprintf(target_file, "LD A, 0x%X\n", symbol->bind & 0xFF);
    fprintf(target_file, "SUB A, %c\n", temp);
    fprintf(target_file, "LD %c, A\n", temp);
    return temp;
}

void loadRegToReg(char from, char to){
    if(from!=to){
        fprintf(target_file, "LD %c, %c\n", to, from);
    }
}

void backup(){
    if(reg & 0b110)
        fprintf(target_file, "PUSH BC\n");
    if(reg & 0b11000)
        fprintf(target_file, "PUSH DE\n");
    if(reg & 0b11000000)
        fprintf(target_file, "PUSH HL\n");
}

void restore(){
    if(reg & 0b11000000)
        fprintf(target_file, "POP HL\n");
    if(reg & 0b11000)
        fprintf(target_file, "POP DE\n");
    if(reg & 0b110)
        fprintf(target_file, "POP BC\n");
}

char readFromMemory(char address){
    freeReg(address);
    backup();
    loadRegToReg(address, 'C');
    fprintf(target_file, "LD A, [HC]\n");
    restore();
    address=getReg();
    fprintf(target_file, "LD %c, A\n", address);
    return address;
}

char handleIdentifier(ASNode* var){
    char temp = handleIdentifierLVal(var->symbol);
    temp = readFromMemory(temp);
    return temp;
}

char handleArray(ASNode* var){
    char temp = handleArrayLVal(var->left, var->right);
    temp = readFromMemory(temp);
    return temp;
}

void writeToMemory(char address, char data){
    freeReg(address);
    freeReg(data);
    backup();
    if (data =='C'){
        loadRegToReg(data,'D');
        data = 'D';
    }
    loadRegToReg(address,'C');
    fprintf(target_file, "LD A, %c\n", data);
    fprintf(target_file, "LD [HC], A\n");
    restore();
}

void validateType(ASNode* left, ASNode* right){
    if (left->dataType != right->dataType)
        TypeCheckError(left->dataType, right);
}

void validateType(DataType dataType, ASNode* exp){
    if (dataType != exp->dataType)
        TypeCheckError(dataType, exp);
}

void handleAssignment(ASNode* left, ASNode* right){
    validateType(left, right);
    char r = GenerateCode(right);
    char l;
    if (left->nodeType != ARRAY_VARIABLE)
        l = handleIdentifierLVal(left->symbol);
    else
        l = handleArrayLVal(left->left, left->right);
    writeToMemory(l,r);
}

char* checkRegisterForFalse(char tempReg){
    clearAccumulator();
    fprintf(target_file, "ADD A, %c\n", tempReg);
    char* skipLabel=getLabel();
    fprintf(target_file, "JR Z, %s\n\n", skipLabel);
    return skipLabel;
}

void checkRegisterForTrue(char tempReg, char* jmpLabel){
    clearAccumulator();
    fprintf(target_file, "ADD A, %c\n", tempReg);
    fprintf(target_file, "JR NZ, %s\n\n", jmpLabel);
}

char handleOperator(char* op, ASNode* operand1, ASNode* operand2){
    char l = GenerateCode(operand1);
    char r = GenerateCode(operand2);
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
        skipLoopLabel=checkRegisterForFalse(r);
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
        skipLoopLabel=checkRegisterForFalse(r);
        loadTOAccumulator(l);
        fprintf(target_file, "LD %c, 0xFF\n", l);
        startLoopLabel = getLabel();
        fprintf(target_file, "\n%s:\n", startLoopLabel);
        fprintf(target_file, "INC %c\n", l);
        fprintf(target_file, "SUB A, %c\n", r);
        fprintf(target_file, "JR NC, %s\n", startLoopLabel);
        fprintf(target_file, "\n%s:\n", skipLoopLabel);
        break;
    default:
        cout<<"Undefined Operator : "<< op << endl;
        exit(-1);
    }
    freeReg(r);
    return l;
}

void storeBooleanInReg(char tempReg, char* skipLabel){
    fprintf(target_file, "LD %c, 0xFF\n", tempReg);
    fprintf(target_file, "\n%s:\n", skipLabel);
    fprintf(target_file, "INC %c\n", tempReg);
}

char handleLogicalOperator(char* op, ASNode* operand1, ASNode* operand2){
    char l = GenerateCode(operand1);
    char r = GenerateCode(operand2);
    char* skipLabel;
    loadTOAccumulator(l);
    clearRegister(l);
    skipLabel = getLabel();
    switch(*(op)){
    case '>':
        fprintf(target_file, "CP %c\n", r);
        fprintf(target_file, "JR NC, %s\n\n", skipLabel);
        break;
    case '<':
        fprintf(target_file, "CP %c\n", r);
        fprintf(target_file, "JR C, %s\n\n", skipLabel);
        break;
    case 'G':
        fprintf(target_file, "CP %c\n", r);
        fprintf(target_file, "JR NC, %s\n", skipLabel);
        fprintf(target_file, "JR Z, %s\n\n", skipLabel);
        break;
    case 'L':
        fprintf(target_file, "CP %c\n", r);
        fprintf(target_file, "JR C, %s\n", skipLabel);
        fprintf(target_file, "JR Z, %s\n\n", skipLabel);
        break;
    case 'N':
        fprintf(target_file, "CP %c\n", r);
        fprintf(target_file, "JR NZ, %s\n\n", skipLabel);
        break;
    case 'E':
        fprintf(target_file, "CP %c\n", r);
        skipLabel = getLabel();
        fprintf(target_file, "JR Z, %s\n\n", skipLabel);
        break;
    default:
        cout<<"Undefined Operator : "<< op << endl;
        exit(-1);
    }
    storeBooleanInReg(l, skipLabel);
    freeReg(r);
    return l;
}

void loadAsciiTable(){
    if (loadedAscii)
        return;
    backup();
    fprintf(target_file, "//LOAD ASCII\n");
    fprintf(target_file, "LD A, 0x%X\n", ASCII_LOAD);
    fprintf(target_file, "CALL LIBRARY\n");
    restore();
    loadedAscii = true;
}

void handleFunctionCalls(ASNode* fun){
    if (fun->varName == "write"||fun->varName == "writeln"){
        char temp = GenerateCode(fun->left);
        backup();
        fprintf(target_file, "//WRITE\n");
        loadRegToReg(temp,'B');
        fprintf(target_file, "LD A, 0x%X\n", (fun->varName == "writeln")?WRITE_NL_CALL:WRITE_CALL);
        fprintf(target_file, "LD C, 0x%X\n", fun->left->dataType);
        fprintf(target_file, "CALL LIBRARY\n");
        restore();
        freeReg(temp);
    }
    else if(fun->varName == "read"){
        ASNode* exp = fun->left;
        char add;
        if (exp->nodeType == ARRAY_VARIABLE){
            add = handleArrayLVal(exp->left, exp->right);
        }
        else
            add = handleIdentifierLVal(exp->symbol);
        backup();
        fprintf(target_file, "//READ\n");
        fprintf(target_file, "LD A, 0x%X\n", READ_CALL);
        fprintf(target_file, "CALL LIBRARY\n");
        restore();
        char temp = getReg();
        fprintf(target_file, "LD %c, A\n", temp);
        writeToMemory(add, temp);
    }
}

void handleIfControlStatements(ASNode* statement, LoopLabel loopLabelDetails, char* elseLabel){
    if (statement->varName == "if"){
        validateType(BOOL_TYPE, statement->left);
        char temp = GenerateCode(statement->left);
        char* skipBlockLabel = checkRegisterForFalse(temp);
        freeReg(temp);
        if(statement->right->nodeType == CONTROL && statement->right->varName == "else")
            handleIfControlStatements(statement->right, loopLabelDetails, skipBlockLabel);
        else {
            GenerateCode(statement->right, loopLabelDetails);
            fprintf(target_file, "\n%s:\n", skipBlockLabel);
        }
    }
    else if(statement->varName == "else"){
        GenerateCode(statement->left, loopLabelDetails);
        char* endIfLabel = getLabel();
        fprintf(target_file, "JR %s\n", endIfLabel);
        fprintf(target_file, "\n%s:\n", elseLabel);
        GenerateCode(statement->right, loopLabelDetails);
        fprintf(target_file, "\n%s:\n", endIfLabel);
    }
}

void handleControlStatements(ASNode* statement, LoopLabel loopLabelDetails){
    if (statement->varName == "if")
        handleIfControlStatements(statement, loopLabelDetails, NULL);
    else if(statement->varName == "while")
    {
        char* loopLabel = getLabel();
        fprintf(target_file, "\n%s:\n", loopLabel);
        validateType(BOOL_TYPE, statement->left);
        char temp = GenerateCode(statement->left);
        char* skipLoopLabel = checkRegisterForFalse(temp);
        freeReg(temp);
        loopLabelDetails = {true, loopLabel, skipLoopLabel};
        GenerateCode(statement->right, loopLabelDetails);
        fprintf(target_file, "JR %s\n", loopLabel);
        fprintf(target_file, "\n%s:\n", skipLoopLabel);
    }
    else if(statement->varName == "do-while")
    {
        char* loopLabel = getLabel();
        fprintf(target_file, "\n%s:\n", loopLabel);
        char* skipLoopLabel = getLabel();
        loopLabelDetails = {true, loopLabel, skipLoopLabel};
        GenerateCode(statement->left, loopLabelDetails);
        validateType(BOOL_TYPE, statement->right);
        char temp = GenerateCode(statement->right);
        checkRegisterForTrue(temp, loopLabel);
        freeReg(temp);
        fprintf(target_file, "\n%s:\n", skipLoopLabel);
    }
    else if (loopLabelDetails.inLoop){
        if(statement->varName == "break")
            fprintf(target_file, "JR %s\n", loopLabelDetails.breakLabel);
        else if(statement->varName == "continue")
            fprintf(target_file, "JR %s\n", loopLabelDetails.continueLabel);
    }
}

char handleCharLiteral(char literal){
    char temp = getReg();
    fprintf(target_file, "LD %c, 0x%X\n", temp, literal);
    return temp;
}

char handleLiteral(ASNode* t){
    switch (t->dataType)
    {
    case BYTE_TYPE:
    case BOOL_TYPE:
        return handleNumericLiteral(t->val);
    case CHAR_TYPE:
        return handleCharLiteral(*(t->varName));
    default:
        cout<<"Undefined Node : "<< t->dataType << " = '" <<t->varName << "'" << endl;
        exit(-1);
        break;
    }
    return 'a';
}

char GenerateCode(struct ASNode *t, LoopLabel loopLabelDetails){
    if (t!=NULL){
        // cout<<"Debug Node : "<< t->nodeType << " = '" << NodeTypeString(t)<< "'" << endl;
        switch (t->nodeType)
        {
        case LITERAL:
            return handleLiteral(t);
        case IDENTIFIER:
            return handleIdentifier(t);
        case ARRAY_VARIABLE:
            return handleArray(t);
        case ASSIGNMENT:
            handleAssignment(t->left, t->right);
            break;
        case OPERATOR:
            validateType(t->left, t->right);
            return handleOperator(t->varName, t->left, t->right);
        case LOGICAL_OPERATOR:
            return handleLogicalOperator(t->varName, t->left, t->right);
        case FUNCTION_CALL:
            handleFunctionCalls(t);
            break;
        case CONTROL:
            handleControlStatements(t, loopLabelDetails);
            break;
        case CONNECTOR:
            GenerateCode(t->left, loopLabelDetails);
            GenerateCode(t->right, loopLabelDetails);
            break;
        default:
            cout<<"Undefined Node : "<< t->nodeType << " = '" <<t->varName << "'" << endl;
            exit(-1);
            break;
        }
    }
    return 0;
}

char GenerateCode(ASNode *t){
    return GenerateCode(t, {false, NULL, NULL});
}
