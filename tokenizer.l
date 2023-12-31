%{
    #include <stdlib.h>
    #include <stdio.h>
    #include "y.tab.h"
    #include "exprtree.h"

    int number;
%}

%%

[0-9]+  {
            number = atoi(yytext);
            yylval.node = makeLeafNode(number);
            return NUM;
        }
"+"     {return PLUS;}
"-"     {return MINUS;}
"*"     {return MUL;}
"/"     {return DIV;}
[ \t]   {/*do nothing*/}
[()]    {return *yytext;}
[\n]    {return END;}
.       {yyerror("unknown character\n");exit(1);}

%%

int yywrap(void) {
    return 1;
}
