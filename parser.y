%{
    #include <iostream>
    #include <stdlib.h>
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

%type <node> Program ListStatement Statement InputStatement OutputStatement AssignmentStatement Expression
%token BLOCK_OPEN BLOCK_CLOSE PLUS MINUS MUL DIV READ WRITE
%token <node> ID NUM
%left PLUS MINUS
%left MUL DIV

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

InputStatement  : READ '(' ID ')' ';'   {$$ = makeOperatorNode("read",$3);}
                ;

OutputStatement : WRITE '(' Expression ')' ';' {$$ = makeOperatorNode("write",$3);}
                ;

AssignmentStatement : ID '=' Expression ';' {$$ = makeOperatorNode('=',$1,$3);}
                    ;

Expression  : Expression PLUS Expression    {$$ = makeOperatorNode('+',$1,$3);}
            | Expression MINUS Expression   {$$ = makeOperatorNode('-',$1,$3);}
            | Expression MUL Expression     {$$ = makeOperatorNode('*',$1,$3);}
            | Expression DIV Expression     {$$ = makeOperatorNode('/',$1,$3);}
            | '(' Expression ')'            {$$ = $2;}
            | ID                            {$$ = $1;}
            | NUM                           {$$ = $1;}
            ;

%%

void yyerror(char const *s)
{
    printf("\nERROR during parse : %s\n",s);
}



int main(int argc, char* argv[]) {
    if(argc > 1)
    {
        FILE *fp = fopen(argv[1], "r");
        if(fp)
            yyin = fp;
    }
    target_file = fopen("TEMP.gsm","w+");
    int checksum = 0;
    int rom_reg_address = 0x134;

    // Store name of the program upto 11 char
    char *name = "TEMP";
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