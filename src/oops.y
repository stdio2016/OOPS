%{
#include <stdio.h>
int yyerror(char *message);
int yylex(void);
%}

// keywords
%token CLASS RETURN NEW
%token IDENTIFIER STRING
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
	| stepExpression ';'
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
	RETURN stepExpression ';'
	;

stepExpression:
	expression
	| stepExpression ',' expression
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
	| STRING
	| '(' stepExpression ')'
	;

type: IDENTIFIER;

name: IDENTIFIER;
%%
int yyerror(char *str)
{
  fprintf(stderr,"error: %s\n",str);
  return 1;
}

int main() {
  return yyparse();
}
