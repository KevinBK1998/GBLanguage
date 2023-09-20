#include <stdio.h>
#include <string.h>
#include "exprtree.h"

char reg = 0;
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

int freeReg(char regToBeFreed){
    int temp = regToBeFreed == 'L' ? 6 : regToBeFreed - 'A';
    reg &= (1 << (temp -'A'));
    return temp -'A';
}

int loadTOAccumulator(char load){
    fprintf(target_file, "LD A, %c\n", load);
    return 0;
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
        loadTOAccumulator(l);
        switch(*(t->op)){
            case '+' :
                fprintf(target_file, "ADD A, %c\n", r);
                break;
            case '-' : 
                fprintf(target_file, "SUB A, %c\n", r);
                break;
        }
        freeReg(r);
        loadFROMAccumulator(l);
        return l;
    }
}