%{
    #include <iostream>
    #include <stdlib.h>
    #include <string.h>
    #include "exprtree.h"
    #include "exprtree.cpp"
    #include "GBCompiler.h"
    #include "GBCompiler.cpp"
    using namespace std;
    int yylex(void);
    void yyerror(char const *s);
    extern FILE* yyin;
%}

%union{
    struct tnode *node;
}

%type <node> Program ListStatement Statement InputStatement OutputStatement ControlStatement AssignmentStatement Expression
%token BLOCK_OPEN BLOCK_CLOSE P_OPEN P_CLOSE LT GT LE GE EQ NE IF ELSE READ WRITE PLUS MINUS MUL DIV 
%token <node> ID NUM
%left PLUS MINUS
%left MUL DIV
%nonassoc LT GT LE GE
%nonassoc EQ NE
%%

Program : BLOCK_OPEN ListStatement BLOCK_CLOSE  {
                                                    $$ = $2;
                                                    codeGen($2);
                                                    fprintf(target_file, "HALT");
                                                    exit(0);
                                                }
        | BLOCK_OPEN BLOCK_CLOSE                {$$ = NULL;}
        ;

ListStatement   : ListStatement Statement   {$$ = makeConnectorNode($1,$2);}
                | Statement                 {$$ = $1;}
                ;

Statement   : InputStatement        {$$ = $1;}
            | OutputStatement       {$$ = $1;}
            | AssignmentStatement   {$$ = $1;}
            ;

InputStatement  : READ P_OPEN ID P_CLOSE ';'    {$$ = makeOperatorNode("read",$3);}
                ;

OutputStatement : WRITE P_OPEN Expression P_CLOSE ';'   {$$ = makeOperatorNode("write",$3);}
                ;

AssignmentStatement : ID '=' Expression ';' {$$ = makeOperatorNode('=',$1,$3);}
                    ;

Expression  : Expression EQ Expression      {$$ = makeOperatorNode('E',$1,$3);}
            | Expression NE Expression      {$$ = makeOperatorNode('N',$1,$3);}
            | Expression LE Expression      {$$ = makeOperatorNode('L',$1,$3);}
            | Expression GE Expression      {$$ = makeOperatorNode('G',$1,$3);}
            | Expression LT Expression      {$$ = makeOperatorNode('<',$1,$3);}
            | Expression GT Expression      {$$ = makeOperatorNode('>',$1,$3);}
            | Expression PLUS Expression    {$$ = makeOperatorNode('+',$1,$3);}
            | Expression MINUS Expression   {$$ = makeOperatorNode('-',$1,$3);}
            | Expression MUL Expression     {$$ = makeOperatorNode('*',$1,$3);}
            | Expression DIV Expression     {$$ = makeOperatorNode('/',$1,$3);}
            | P_OPEN Expression P_CLOSE     {$$ = $2;}
            | ID                            {$$ = $1;}
            | NUM                           {$$ = $1;}
            ;

%%

void yyerror(char const *s)
{
    printf("\nERROR during parse : %s, Deleting temp file status: %d\n\n", s, remove("TEMP.gsm"));
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
    fclose(target_file);
    return 0;
}