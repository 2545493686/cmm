%{
#include <stdio.h>
#include <cmm_token_type.h>
%}

%option noyywrap

%%
if { printf("%d if ", If); }
return { printf("%d return ", Return); }
\( { printf("%d ( ", LeftBracket); }
\) { printf("%d ) ", RightBracket); }
\{ { printf("%d { ", LeftBrace); }
\} { printf("%d } ", RightBrace); }
\= { printf("%d = ", Equal); }
\- { printf("%d - ", Minus); }
\+ { printf("%d + ", Plus); }
\* { printf("%d * ", Star); }
\/ { printf("%d / ", Slash); }
\< { printf("%d < ", LessThan); }
\> { printf("%d > ", GreaterThan); }
\; { printf("%d ; ", EOL); }
\, { printf("%d , ", Comma); }
[A-Z_a-z][A-Z_a-z0-9]* { printf("%d %s ", Identifier, yytext); }
0|[1-9][0-9]* { printf("%d %s ", Integer, yytext); }
[ \t\r\n]+     ; /* skip whitespace */
%%

int main(int argc, char *argv[])
{
    yylex();
    return 0;
}
