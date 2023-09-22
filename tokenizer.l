%{
    #include <stdlib.h>
    #include <stdio.h>
    #include "y.tab.h"
    #include "exprtree.h"

    int number;
    extern void yyerror(char const *s);
%}

%%

"read"  {return READ;}
"write"  {return WRITE;}
"+"     {return PLUS;}
"-"     {return MINUS;}
"*"     {return MUL;}
"/"     {return DIV;}
"{"     {return BLOCK_OPEN;}
"}"     {return BLOCK_CLOSE;}
[ \t\n] {/*do nothing*/}
[()=;]  {return *yytext;}
[0-9]+  {
            number = atoi(yytext);
            yylval.node = makeLeafNode(number);
            return NUM;
        }
[a-zA-Z][a-zA-Z0-9_]*   {
                            yylval.node = makeLeafNode(yytext);
                            return ID;
                        }
.       {yyerror("unknown character\n");exit(1);}

%%

int yywrap(void) {
    return 1;
}
