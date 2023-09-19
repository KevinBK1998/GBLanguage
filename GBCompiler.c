#include <stdio.h>
#include <string.h>
#include "exprtree.h"

int reg = 0;

int getReg(){
    return reg++;
}
int freeReg(){
    return reg--;
}

int evaluate(struct tnode *t){
    FILE* target_file = fopen("TEMP.gsm","w");
    if(t->op == NULL)
    {
        int n = getReg();
        fprintf(target_file, "MOV R%d, %d\n", n, t->val);
        return n;
    }
    else{
        int res;
        switch(*(t->op)){
            case '+' :
                
                res = evaluate(t->left) + evaluate(t->right);
                break;
            case '-' : 
                res = evaluate(t->left) - evaluate(t->right);
                break;
            case '*' : 
                res = evaluate(t->left) * evaluate(t->right);
                break;
            case '/' : 
                res = evaluate(t->left) / evaluate(t->right);
                break;
        }
        printf("%s\n", t ->op);
        return res;
    }
}