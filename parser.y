%{
    #include <iostream>
    #include <stdlib.h>
    #include <string.h>
    #include "ASTree.h"
    #include "ASTree.h"
    #include "ASTree.cpp"
    #include "GBCompiler.h"
    #include "GBCompiler.cpp"
    #include "GSTable.h"
    #include "GSTable.cpp"
    using namespace std;
    int yylex(void);
    void yyerror(char const *s);
    ASNode* safeLinkSymbol(ASNode*);
    extern FILE* yyin;
%}

%union{
    struct ASNode *node;
}

%type <node> Program ListStatement BlockStatement Statement InputStatement OutputStatement ControlStatement AssignmentStatement
%type <node> Expression BooleanExpression VariableList Type Variable
%token BLOCK_OPEN BLOCK_CLOSE P_OPEN P_CLOSE IF ELSE DO WHILE BREAK CONTINUE READ WRITE WRITE_LN BYTE
%token LT GT LE GE EQ NE PLUS MINUS MUL DIV
%token <node> ID NUM
%left PLUS MINUS
%left MUL DIV
%nonassoc LT GT LE GE
%nonassoc EQ NE
%%

Program : BLOCK_OPEN ListStatement BLOCK_CLOSE  {GenerateCode($2);}
        | BLOCK_OPEN BLOCK_CLOSE                { $$ = NULL; }
        ;

BlockStatement  : BLOCK_OPEN ListStatement BLOCK_CLOSE  {$$ = $2;}
                | Statement                             {$$ = $1;}
                ;

ListStatement   : ListStatement BlockStatement  {$$ = makeConnectorNode($1,$2);}
                | BlockStatement                {$$ = $1;}
                ;

Statement   : DeclareStatement      {$$=NULL;}
            | InputStatement        {$$ = $1;}
            | OutputStatement       {$$ = $1;}
            | ControlStatement      {$$ = $1;}
            | AssignmentStatement   {$$ = $1;}
            | BREAK ';'             {$$ = makeControlNode("break");}
            | CONTINUE ';'          {$$ = makeControlNode("continue");}
            ;

DeclareStatement    : Type VariableList ';' {DeclareList($1, $2);}
                    ;

Type    : BYTE  {$$=makeDataTypeNode(BYTE_TYPE);}
        ;

VariableList    : VariableList ',' ID   {$$=makeConnectorNode($1,$3);}
                | ID                    {$$ = $1;}
                ;

InputStatement  : READ P_OPEN Variable P_CLOSE ';'    {$$ = makeOperatorNode("read",$3);}
                ;

OutputStatement : WRITE P_OPEN Expression P_CLOSE ';'       {$$ = makeOperatorNode("write",$3);}
                | WRITE_LN P_OPEN Expression P_CLOSE ';'    {$$ = makeOperatorNode("writeln",$3);}
                ;

ControlStatement    : IF P_OPEN BooleanExpression P_CLOSE BlockStatement                        {$$ = makeControlNode("if", $3, $5);}
                    | IF P_OPEN BooleanExpression P_CLOSE BlockStatement ELSE BlockStatement    {$$ = makeControlNode("if", $3, makeControlNode("else", $5, $7));}
                    | WHILE P_OPEN BooleanExpression P_CLOSE BlockStatement                     {$$ = makeControlNode("while", $3, $5);}
                    | DO BlockStatement WHILE P_OPEN BooleanExpression P_CLOSE ';'              {$$ = makeControlNode("do-while", $2, $5);}
                    ;

AssignmentStatement : Variable '=' Expression ';' {$$ = makeOperatorNode('=',$1,$3);}
                    ;

BooleanExpression   : Expression EQ Expression  {$$ = makeOperatorNode('E',$1,$3);}
                    | Expression NE Expression  {$$ = makeOperatorNode('N',$1,$3);}
                    | Expression LE Expression  {$$ = makeOperatorNode('L',$1,$3);}
                    | Expression GE Expression  {$$ = makeOperatorNode('G',$1,$3);}
                    | Expression LT Expression  {$$ = makeOperatorNode('<',$1,$3);}
                    | Expression GT Expression  {$$ = makeOperatorNode('>',$1,$3);}
                    ;

Expression  : Expression PLUS Expression    {$$ = makeOperatorNode('+',$1,$3);}
            | Expression MINUS Expression   {$$ = makeOperatorNode('-',$1,$3);}
            | Expression MUL Expression     {$$ = makeOperatorNode('*',$1,$3);}
            | Expression DIV Expression     {$$ = makeOperatorNode('/',$1,$3);}
            | P_OPEN Expression P_CLOSE     {$$ = $2;}
            | Variable                      {$$ = $1;}
            | NUM                           {$$ = $1;}
            ;
Variable    : ID    {$$=safeLinkSymbol($1);}

%%

void yyerror(char const *s)
{
    printf("\nERROR during parse : %s, Deleting temp file status: %d\n\n", s, remove("TEMP.gsm"));
    exit(1);
}

ASNode* safeLinkSymbol(ASNode* node){
    char* name = node->varName;
    node = linkSymbol(node);
    if (node==NULL)
        yyerror(strcat(name, " is not declared"));
    return node;
}

int main(int argc, char* argv[]) {
    char *name = "TEMP";
    if(argc > 1)
    {
        FILE *fp = fopen(argv[1], "r");
        if(fp){
            yyin = fp;
            name = argv[1];
            name = strchr(name, '/') + 1;
        }
    }
    target_file = fopen("TEMP.gsm","w+");
    int checksum = 0;
    int rom_reg_address = 0x134;

    // Store name of the program upto 11 char
    int shortName = 0;
    for (int i=0; i<11; i++,rom_reg_address++){
        if (!shortName && name[i]=='\0')
            shortName = 1;
        fprintf(target_file, "DATA 0x%X\n", (shortName ? 0 : name[i]) );
        checksum+=(shortName ? 0 : name[i]);
    }
    for (;rom_reg_address<0x14D;rom_reg_address++){
        fprintf(target_file, "DATA 0x%X\n", 0);
        checksum+=0;
    }
    checksum+=0x19;
    checksum=0-checksum;
    fprintf(target_file, "DATA 0x%X\n", checksum&0xFF);
    fprintf(target_file, "DATA 0x%X\n", 0);
    fprintf(target_file, "DATA 0x%X\n", 0);

    // Start of user program
    fprintf(target_file, "\nSTART:\n");
    yyparse();
    fprintf(target_file, "HALT");
    fclose(target_file);
    return 0;
}