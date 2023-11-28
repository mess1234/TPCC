%{  /* lexer.l */
    #include <string.h>
    #include "../inc/tree.h"
    #include "parser.h"

    char string_to_char(char *s);
    int line_nb = 1;
    int char_nb = 1;
%}

%option nounput
%option noinput
%option noyywrap

%x BLOCK_COMMENT

/* number         ([0-9])|([1-9][0-9]+) */
number              [0-9]+
character_simple    '[^'\\\n\t]'
character_special   '\\['\\nt]'
ident               [a-zA-Z_][a-zA-Z0-9_]*
not_ident_char      [^a-zA-Z0-9_]
blank_not_lf        [ \t]
other_tokens        [=!;,(){}]

%%

\/\*                        {   char_nb += yyleng;
                                BEGIN BLOCK_COMMENT; }
\/\/.*                      {   /* line comment, pass */ }
(int|char)/{not_ident_char} {   char_nb += yyleng;
                                strcpy(yylval.type, yytext);
                                return TYPE; }
void/{not_ident_char}       {   char_nb += yyleng;
                                return VOID; }
if/{not_ident_char}         {   char_nb += yyleng;
                                return IF; }
else/{not_ident_char}       {   char_nb += yyleng;
                                return ELSE; }
while/{not_ident_char}      {   char_nb += yyleng;
                                return WHILE; }
return/{not_ident_char}     {   char_nb += yyleng;
                                return RETURN; }
\|\|                        {   char_nb += yyleng;
                                return OR; }
&&                          {   char_nb += yyleng;
                                return AND; }
==|!=                       {   char_nb += yyleng;
                                strcpy(yylval.comp, yytext);
                                return EQ; }
[<>]=?                      {   char_nb += yyleng;
                                strcpy(yylval.comp, yytext);
                                return ORDER; }
[-+]                        {   char_nb += yyleng;
                                yylval.byte = yytext[0];
                                return ADDSUB; }
\*|\/|%                     {   char_nb += yyleng;
                                yylval.byte = yytext[0];
                                return DIVSTAR; }
{number}                    {   char_nb += yyleng;
                                yylval.num = atoi(yytext);
                                return NUM; }
{character_simple}          {   char_nb += yyleng;
                                yylval.byte = yytext[1];
                                return CHARACTER; }
{character_special}         {   char_nb += yyleng;
                                yylval.byte = string_to_char(yytext);
                                return CHARACTER; }
{ident}                     {   char_nb += yyleng;
                                strcpy(yylval.ident, yytext);
                                return IDENT; }
{other_tokens}              {   char_nb++;
                                return yytext[0]; }
\n                          {   line_nb++;
                                char_nb = 1; }
{blank_not_lf}+             {   char_nb += yyleng; }
.                           {   char_nb += yyleng;
                                return yytext[0]; }
<BLOCK_COMMENT>\*\/         {   char_nb += yyleng;
                                BEGIN INITIAL; }
<BLOCK_COMMENT>\n           {   line_nb++;
                                char_nb = 1; }
<BLOCK_COMMENT>[^\n]        {   char_nb++; }

%%

/**
 * @brief Convert a special character string to its char value.
 *
 * @param s special character string ('\'', '\n', '\t')
 * @return char corresponding to the string
 */
char string_to_char(char *s)
{
    switch (s[2])
    {
        case '\'':  return '\'';
        case '\\':  return '\\';
        case 'n':   return '\n';
        case 't':   return '\t';
        default: /* should not be here */ return 0;
    }
}
