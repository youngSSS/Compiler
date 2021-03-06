/****************************************************/
/* File: cminus.l                                   */
/* Lex specification for C-MINUS					*/
/****************************************************/

%{
#include "globals.h"
#include "util.h"
#include "scan.h"
/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];
%}

digit       [0-9]
number      {digit}+
letter      [a-zA-Z]
identifier  {letter}+
newline     \n
whitespace  [ \t]+

%%

"if"            {return IF;}
"else"          {return ELSE;}
"while"			{return WHILE;}
"return"		{return RETURN;}
"int"			{return INT;}
"void"			{return VOID;}
"="				{return ASSIGN;}
"=="            {return EQ;}
"!="			{return NE;}
"<"             {return LT;}
"<="			{return LE;}
">"				{return GT;}
">="			{return GE;}
"("				{return LPAREN;}
")"				{return RPAREN;}
"["				{return LBRACE;}
"]"				{return RBRACE;}
"{"				{return LCURLY;}
"}"				{return RCURLY;}
";"             {return SEMI;}
","				{return COMMA;}
"+"				{return PLUS;}
"-"				{return MINUS;}
"*"				{return TIMES;}
"/"				{return OVER;}
{number}        {return NUM;}
{identifier}    {return ID;}
{newline}       {lineno++;}
{whitespace}    {/* skip whitespace */}
"/*"            { char old_c, c;
                  do
                  { old_c = c;
					c = input();
                    if (c == EOF) break;
                    if (c == '\n') lineno++;
                  } while (!(old_c == '*' && c == '/'));
                }
.               {return ERROR;}

%%

TokenType getToken(void)
{ static int firstTime = TRUE;
  TokenType currentToken;
  if (firstTime)
  { firstTime = FALSE;
    lineno++;
    yyin = source;
    yyout = listing;
  }
  currentToken = yylex();
  strncpy(tokenString,yytext,MAXTOKENLEN);
  if (TraceScan) {
    fprintf(listing,"\t%d: ",lineno);
    printToken(currentToken,tokenString);
  }
  return currentToken;
}

