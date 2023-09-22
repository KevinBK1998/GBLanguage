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
    };
    target_file = fopen("TEMP.gsm","w+");
    // Redirect to Source code
    fprintf(target_file, "JP START\n", 0x150);
    // Store data for logo
    for (int i=0; i< 0x30; i++){
        fprintf(target_file, "DATA 0x%X\n", logo_data[i]);
    }

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
    /* printf("CHECKSUM-HI:%X\n", (checksum/0x100)&0xFF); */
    /* fprintf(target_file, "DATA 0x%X\n", (checksum/0x100)&0xFF); */
    /* printf("COMPLEMENT-CHECK:%X\n",checksum&0xFF); */
    fprintf(target_file, "DATA 0x%X\n", checksum&0xFF);
    fprintf(target_file, "DATA 0x%X\n", 0);
    fprintf(target_file, "DATA 0x%X\n", 0);
    fprintf(target_file, "START:");
    yyparse();
    fclose(target_file);
    return 0;
}