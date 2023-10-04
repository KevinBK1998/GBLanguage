%{
    #include <stdlib.h>
    #include <string.h>
    #include <stdio.h>
    #include "y.tab.h"
    #include "ASTree.h"

    int number;
    extern void yyerror(char const *s);
%}

comment "//".*
num [0-9]+
id  [a-zA-Z][a-zA-Z0-9_]*
ch      "'"[0-9A-Za-z ]"'"
str     "\""[0-9A-Za-z ]*"\""
null_c  "'\\0'"

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
"["     {return B_OPEN;}
"]"     {return B_CLOSE;}
[=;,(){}]   {return *yytext;}

"byte"      {return BYTE;}
"bool"      {return BOOL;}
"char"      {return CHAR;}
"string"    {return STRING_T;}

"true"      {return TRUE;}
"false"     {return FALSE;}
"if"        {return IF;}
"else"      {return ELSE;}
"do"        {return DO;}
"while"     {return WHILE;}
"break"     {return BREAK;}
"continue"  {return CONTINUE;}
"read"      {return READ;}
"write"     {return WRITE;}
"writeln"   {return WRITE_LN;}

[ \t\n]     {/*ignore white spaces*/}
{comment}   {/*ignore comment*/}

{null_c}    {return NULL_C;}
{id}    {
            char* temp = strdup(yytext);
            yylval.node = makeVarNode(temp);
            return ID;
        }

{ch}    {
            char* temp = strdup(yytext);
            yylval.node = makeLeafNode(temp[1]);
            return CHARACTER;
        }

{str}   {
            char* temp = strdup(yytext);
            temp[strlen(temp)-1]='\0';
            temp++;
            yylval.node = makeLeafNode(temp);
            return STRING;
        }

{num}   {
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
