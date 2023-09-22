#include <stdio.h>
#include <string.h>
#include "exprtree.h"

char reg = 0;
char label[] = "A_0";
FILE* target_file;

char getReg(){
    int temp = reg;
    char freeReg = 'A';
    do{
        temp /= 2;
        freeReg++;
    }while (temp%2);
    reg |= (1 << (freeReg -'A'));
    if (freeReg>'H'){
        printf("OOM");
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
    char oldLabel[4];
    strcpy(oldLabel, label);
    label[0] = labelChar;
    label[2] = labelNum + '0';
    char* freeLabel = oldLabel;
    return freeLabel;
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

char codeGen(struct tnode *t){
    if(t->op == NULL)
    {
        char temp = getReg();
        fprintf(target_file, "LD %c, 0x%X\n", temp, t->val);
        return temp;
    }
    else{
        char l = codeGen(t->left);
        char r = codeGen(t->right);
        char* tempLabel;
        switch(*(t->op)){
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
                fprintf(target_file, "JR NZ, %s\n", tempLabel);
                loadFROMAccumulator(l);
                break;
            case '-':
                loadTOAccumulator(l);
                fprintf(target_file, "SUB A, %c\n", r);
                loadFROMAccumulator(l);
                break;
            case '/':
                loadTOAccumulator(l);
                clearRegister(l);
                tempLabel = getLabel();
                fprintf(target_file, "\n%s:\n", tempLabel);
                fprintf(target_file, "INC %c\n", l);
                fprintf(target_file, "SUB A, %c\n", r);
                fprintf(target_file, "JR NC, %s\n", tempLabel);
                fprintf(target_file, "DEC %c\n", l);
                break;
            default:
                printf("Op:%c",t->op);
                exit(1);
        }
        freeReg(r);
        return l;
    }
}