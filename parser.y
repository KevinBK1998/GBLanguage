%{
    #include <iostream>
    #include <stdlib.h>
    #include <string.h>
    #include "ASTree.h"
    #include "ASTree.cpp"
    #include "GBCompiler.h"
    #include "GBCompiler.cpp"
    #include "GSTable.h"
    #include "GSTable.cpp"
    using namespace std;
    int yylex(void);
    void yyerror(char const *s);
    void DeclareLine(ASNode*, ASNode*);
    extern FILE* yyin;
%}

%union{
    struct ASNode *node;
}

%type <node> Program ListStatement BlockStatement Statement InputStatement OutputStatement ControlStatement AssignmentStatement
%type <node> Expression VariableList Type Variable
%token B_OPEN B_CLOSE IF ELSE DO WHILE BREAK CONTINUE READ WRITE WRITE_LN TRUE FALSE
%token LT GT LE GE EQ NE PLUS MINUS MUL DIV BYTE BOOL CHAR STRING_T NULL_C
%token <node> ID NUM CHARACTER STRING
%left PLUS MINUS
%left MUL DIV
%nonassoc LT GT LE GE
%nonassoc EQ NE
%%

Program : ListStatement {GenerateCode($1);}
        ;

BlockStatement  : '{' ListStatement '}'  {$$ = $2;}
                | Statement                             {$$ = $1;}
                ;

ListStatement   : ListStatement BlockStatement  {$$ = makeConnectorNode($1,$2);}
                | BlockStatement                {$$ = $1;}
                ;

Statement   : DeclareStatement      {$$=NULL;}
            | InputStatement        {loadAsciiTable();$$ = $1;}
            | OutputStatement       {loadAsciiTable();$$ = $1;}
            | ControlStatement      {$$ = $1;}
            | AssignmentStatement   {$$ = $1;}
            | BREAK ';'             {$$ = makeControlNode("break");}
            | CONTINUE ';'          {$$ = makeControlNode("continue");}
            ;

DeclareStatement    : Type VariableList ';' {DeclareLine($1, $2);}
                    ;

Type    : BYTE      {$$=makeDataTypeNode(BYTE_TYPE);}
        | BOOL      {$$=makeDataTypeNode(BOOL_TYPE);}
        | CHAR      {$$=makeDataTypeNode(CHAR_TYPE);}
        | STRING_T  {$$=makeDataTypeNode(STR_TYPE);}
        ;

VariableList    : VariableList ',' ID B_OPEN NUM B_CLOSE    {$$=makeConnectorNode($1, makeArrayNode($3, $5));}
                | VariableList ',' ID                       {$$=makeConnectorNode($1,$3);}
                | ID B_OPEN NUM B_CLOSE                     {$$ = makeArrayNode($1, $3);}
                | ID                                        {$$ = $1;}
                ;

InputStatement  : READ '(' Variable ')' ';'                             {$$ = makeFunctionNode("read",$3);}
                | READ '(' Variable B_OPEN Expression B_CLOSE ')' ';'   {$$ = makeFunctionNode("read", makeArrayNode($3, $5));}
                ;

OutputStatement : WRITE '(' Expression ')' ';'      {$$ = makeFunctionNode("write",$3);}
                | WRITE_LN '(' Expression ')' ';'   {$$ = makeFunctionNode("writeln",$3);}
                ;

ControlStatement    : IF '(' Expression ')' BlockStatement                      {$$ = makeControlNode("if", $3, $5);}
                    | IF '(' Expression ')' BlockStatement ELSE BlockStatement  {$$ = makeControlNode("if", $3, makeControlNode("else", $5, $7));}
                    | WHILE '(' Expression ')' BlockStatement                   {$$ = makeControlNode("while", $3, $5);}
                    | DO BlockStatement WHILE '(' Expression ')' ';'            {$$ = makeControlNode("do-while", $2, $5);}
                    ;

AssignmentStatement : Variable '=' Expression ';'                           {$$ = makeOperatorNode('=',$1,$3);}
                    | Variable B_OPEN Expression B_CLOSE '=' Expression ';' {$$ = makeOperatorNode('=',makeArrayNode($1, $3),$6);}
                    ;

Expression  : Expression PLUS Expression            {$$ = makeOperatorNode('+',$1,$3);}
            | Expression MINUS Expression           {$$ = makeOperatorNode('-',$1,$3);}
            | Expression MUL Expression             {$$ = makeOperatorNode('*',$1,$3);}
            | Expression DIV Expression             {$$ = makeOperatorNode('/',$1,$3);}
            | Expression EQ Expression              {$$ = makeLogicalOperatorNode('E',$1,$3);}
            | Expression NE Expression              {$$ = makeLogicalOperatorNode('N',$1,$3);}
            | Expression LE Expression              {$$ = makeLogicalOperatorNode('L',$1,$3);}
            | Expression GE Expression              {$$ = makeLogicalOperatorNode('G',$1,$3);}
            | Expression LT Expression              {$$ = makeLogicalOperatorNode('<',$1,$3);}
            | Expression GT Expression              {$$ = makeLogicalOperatorNode('>',$1,$3);}
            | '(' Expression ')'                    {$$ = $2;}
            | Variable                              {$$ = $1;}
            | Variable B_OPEN Expression B_CLOSE    {$$ = makeArrayNode($1, $3);}
            | NUM                                   {$$ = $1;}
            | CHARACTER                             {$$ = $1;}
            | STRING                                {$$ = $1;}
            | TRUE                                  {$$ = makeLeafNode(true);}
            | FALSE                                 {$$ = makeLeafNode(false);}
            | NULL_C                                {$$ = makeLeafNode('\0');}
            ;
Variable    : ID    {$$=linkSymbol($1);}

%%

void yyerror(char const *s)
{
    printf("\nERROR during parse : %s, Deleting temp file status: %d\n\n", s, remove("TEMP.gsm"));
    exit(1);
}

void CompileError(string message){
    yyerror(message.c_str());
}

void DeclareLine(ASNode* type, ASNode* list){
    /* cout<<"Debug Declare : "<<type->dataType<<", "<< list << endl; */
    DeclareList(type, list);
    fprintf(target_file, "LD SP, 0x%X\n", sp);
}

bool typeCheck(ASNode* operand1, ASNode* operand2){

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