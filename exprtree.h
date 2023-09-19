#ifndef EXPR_TREE_H
#define EXPR_TREE_H

typedef struct tnode{
    int val; //value of the expression tree
    char *op; //indicates the opertor
    struct tnode *left,*right; //left and right branches
} tnode;

/*Make a leaf tnode and set the value of val field*/
struct tnode* makeLeafNode(int n);

/*Make a tnode with opertor, left and right branches set*/
struct tnode* makeOperatorNode(char c, struct tnode *l, struct tnode *r);

#endif