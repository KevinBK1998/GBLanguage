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
                                $$ = $2;codeGen($1);
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
    int logo_data[] = {
        0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
        0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
        0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
        0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
        0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
        0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
        0x43, 0x50, 0x55, 0x5F, 0x49, 0x4E, 0x53, 0x54, 0x52, 0x53,
    };
    target_file = fopen("TEMP.gsm","w+");
    fprintf(target_file, "CALL 0x%X\n", 0x200);
    for (int i=0; i< 56; i++){
        fprintf(target_file, "DATA 0x%X\n", logo_data[i]);
    }
    yyparse();
    fclose(target_file);
    return 0;
}