%{
#include <stdio.h>
#include <ctype.h>
#include "errReport.h"
int yyerror(const char *message);
int yylex(void);
char *yytext;
%}

%union {
  char *str;
}

// keywords
%token CLASS RETURN NEW THIS SUPER
%token<str> IDENTIFIER STRING
%token ERROR
%start program
%%
program:
	/* empty */
	| program class
	;

class:
	CLASS name inherit '{' classBody '}'
	;

inherit:
	/* no base class */
	| ':' name
	;

classBody:
	/* empty */
	| classBody classBodyItem
	;

classBodyItem:
	fields
	| method
	| constructor
	;

fields:
	type fieldList ';'
	;

fieldList:
	name
	| fieldList ',' name
	;

method:
	type name '(' argumentList ')' block
	;

constructor:
	name '(' argumentList ')' block
	;

argumentList:
	/* empty */
	| argumentListNonEmpty
	;

argumentListNonEmpty:
	argument
	| argumentListNonEmpty ',' argument
	;

argument:
	type name
	;

block:
	'{' statements '}'
	;

statements:
	/* empty */
	| statements statement
	;

statement:
	varDecls
	| return
	| block
	| expression ';'
	;

varDecls:
	type varList ';'
	;

varList:
	varDecl
	| varList ',' varDecl
	;

varDecl:
	name
	| name '=' expression
	;

return:
	RETURN expression ';'
	;

expression:
	newExpr
	| name '=' expression
	;

newExpr:
	callExpr
	| NEW newExpr
	;

callExpr:
	atom
	| callExpr '(' callArgs ')'
	| callExpr '.' name
	;

callArgs:
	/* empty */
	| callArgsNonEmpty
	;

callArgsNonEmpty:
	expression
	| callArgsNonEmpty ',' expression
	;

atom:
	IDENTIFIER
	| THIS
	| SUPER
	| STRING
	| '(' expression ')'
	;

type: IDENTIFIER;

name: IDENTIFIER;
%%
int yyerror(const char *str)
{
  if (isprint(yytext[0])) {
    syntaxError("unmatched token: %s\n", yytext);
  }
  else if (yytext[0]) {
    syntaxError("illegal character: '\\x%x'\n",yytext[0]);
  }
  return 1;
}

int main() {
  int n = yyparse();
  if (n == 0) {
    printf("There is no syntax error! :-)\n");
  }
  else {
    printf("There is syntax error ;-(\n");
  }
  return n;
}
