%{
    #include <stdlib.h>
    #include <stdio.h>
    #include "exprtree.h"
    #include "exprtree.c"
    #include "GBCompiler.h"
    #include "GBCompiler.c"
    
    int yylex(void);
%}

%union{
    struct tnode *node;
}

%type <node> expr NUM program END
%token NUM PLUS MINUS MUL DIV END
%left PLUS MINUS
%left MUL DIV

%%

program : expr END          {
                                $$ = $2;
                                codeGen($1);
                                fprintf(target_file, "HALT");
                                exit(0);
                            }
        ;

expr    : expr PLUS expr    {$$ = makeOperatorNode('+',$1,$3);}
        | expr MINUS expr   {$$ = makeOperatorNode('-',$1,$3);}
        | expr MUL expr     {$$ = makeOperatorNode('*',$1,$3);}
        | expr DIV expr     {$$ = makeOperatorNode('/',$1,$3);}
        | '(' expr ')'      {$$ = $2;}
        | NUM               {$$ = $1;}
        ;

%%

yyerror(char const *s)
{
    printf("yyerror %s",s);
}



int main(void) {
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
    fprintf(target_file, "START:");
    yyparse();
    fclose(target_file);
    return 0;
}