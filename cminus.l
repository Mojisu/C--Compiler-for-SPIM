/****************************************************/
/* File: cminus.l									*/
/* Lex specification for C-							*/
/* Compiler Construction: Principles and Practice   */
/* Lee SangBeom                                     */
/****************************************************/

%{
#include "globals.h"
#include "util.h"
#include "scan.h"
#include "cminus.tab.h"

/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];
char prevToken[MAXTOKENLEN+1];
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
"int"			{return INT;}
"return"		{return RETURN;}
"void"			{return VOID;}
"while"			{return WHILE;}
"+"             {return PLUS;}
"-"             {return MINUS;}
"*"             {return MUL;}
"/"             {return BACKSLASH;}
"<"				{return LT;}
"<="			{return LTE;}
">"				{return GT;}
">="			{return GTE;}
"=="			{return EQ;}
"!="			{return NEQ;}
"="				{return ASSIGN;}
";"				{return SEMI;}
","				{return COMMA;}
"("             {return LPAREN;}
")"             {return RPAREN;}
"["				{return LSQUBRACKET;}
"]"				{return RSQUBRACKET;}
"{"				{return LCURLYBRACKET;}
"}"				{return RCURLYBRACKET;}
{number}        {return NUM;}
{identifier}    {return ID;}
{newline}       {lineno++;}
{whitespace}    {/* skip whitespace */}
"/*"            { register int c;
					for( ; ; )
					{
						while((c = input()) != '*' && c != EOF){
							if(c == '\n') lineno++;
						}
						if(c == '*'){
							while((c = input()) == '*' || c=='\n'){
								if(c == '\n') lineno++;
							}
							if(c == '/'){ break;
							}
						}
						if(c == EOF){
							return ERROR;
						}
					}
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
  strncpy(prevToken, tokenString, MAXTOKENLEN);
  strncpy(tokenString,yytext,MAXTOKENLEN);
  if (TraceScan) {
    fprintf(listing,"\t    %d			        ",lineno);
    printToken(currentToken,tokenString);
  }
  return currentToken;
}

