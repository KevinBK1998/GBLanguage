%{
    #include <stdlib.h>
    #include <string.h>
    #include <stdio.h>
    #include "y.tab.h"
    #include "ASTree.h"

    int number;
    extern void yyerror(char const *s);
%}

%%
"+"     {return PLUS;}
"-"     {return MINUS;}
"*"     {return MUL;}
"/"     {return DIV;}
"<"     {return LT;}
">"     {return GT;}
"<="    {return LE;}
">="    {return GE;}
"=="    {return EQ;}
"!="    {return NE;}
"{"     {return BLOCK_OPEN;}
"}"     {return BLOCK_CLOSE;}
"["     {return B_OPEN;}
"]"     {return B_CLOSE;}
[=;,()]     {return *yytext;}

"if"        {return IF;}
"else"      {return ELSE;}
"do"        {return DO;}
"while"     {return WHILE;}
"break"     {return BREAK;}
"continue"  {return CONTINUE;}
"byte"      {return BYTE;}
"bool"      {return BOOL;}
"read"      {return READ;}
"write"     {return WRITE;}
"writeln"   {return WRITE_LN;}

[ \t\n] {/*do nothing*/}

[a-zA-Z][a-zA-Z0-9_]*   {
                            char* temp = strdup(yytext);
                            yylval.node = makeLeafNode(temp);
                            return ID;
                        }

[0-9]+  {
            number = atoi(yytext);
            yylval.node = makeLeafNode(number);
            return NUM;
        }

.       {
            yyerror(strcat(yytext, "(Unknown character)"));
            exit(1);
        }

%%

int yywrap(void) {
    return 1;
}
