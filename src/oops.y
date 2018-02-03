%{
#include <stdio.h>
#include <ctype.h>
#include "errReport.h"
#include "ast.h"
#include "symtable.h"
int yyerror(const char *message);
int yylex(void);
char *yytext;
ClassType currentType;
ClassType thisClass;
%}

%union {
  char *str;
  ClassType cls;
  struct ArgType argList;
  int scope;
}

// keywords
%token CLASS RETURN NEW THIS SUPER
%token<str> IDENTIFIER STRING
%token ERROR

%start program
%type<str> name varDecl
%type<cls> inherit type
%type<argList> argumentList
%type<scope> PushScope

%destructor { free($$); } <str>
%destructor { } <cls>
%destructor { free($$.types); } <argList>
%destructor { showScope(0); popScope(); } <scope>

%%
program:
	/* empty */
	| program class
	;

class:
	CLASS name inherit {
	  printf("class %s\n", $2);
	  thisClass = createClass($2, $3);
	}
	'{' classBody '}' {
	  printf("end of class %s\n", $2);
	  free($2);
	}
	;

inherit:
	/* no base class */ { $$ = getVoidClass(); }
	| ':' type { $$ = $2; }
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
	name { addField(thisClass, currentType, $1); free($1); }
	| fieldList ',' name { addField(thisClass, currentType, $3); free($3); }
	;

method:
	methodHead methodBody
	;

methodHead:
	type name '(' PushScope argumentList ')' {
	  int scope = $4;
	  addMethod(Method_METHOD, thisClass, $1, $2, $5);
	  free($2);
	}
	;

constructor:
	constructorHead methodBody
	;

constructorHead:
	name '(' PushScope argumentList ')' {
	  int scope = $3;
	  if (strcmp($1, thisClass->name)) {
	    semanticError("constructor name and class name differs\n");
	  }
	  addMethod(Method_CONSTRUCTOR, thisClass, getVoidClass(), $1, $4);
	  free($1);
	}
	;

argumentList:
	/* empty */ { $$ = getArgumentList(); }
	| argumentListNonEmpty { $$ = getArgumentList(); }
	;

argumentListNonEmpty:
	argument
	| argumentListNonEmpty ',' argument
	;

argument:
	type name { addParamVar($1, $2); free($2); }
	;

methodBody:
	'{' statements '}' { showScope(0); popScope(); }
	;

block:
	'{' PushScope statements '}' { int s = $2; showScope(0); popScope(); }
	;

PushScope: { $$ = pushScope(); };

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
	varDecl { free($1); }
	| varList ',' varDecl { free($3); }
	;

varDecl:
	name { addLocalVar(currentType, $1); $$ = $1; }
	| name '=' expression { addLocalVar(currentType, $1); $$ = $1; }
	;

return:
	RETURN expression ';'
	;

expression:
	newExpr
	| newExpr '=' expression
	;

newExpr:
	callExpr
	| NEW newExpr
	;

callExpr:
	atom
	| callExpr '(' callArgs ')'
	| callExpr '.' name { free($3); }
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
	IDENTIFIER { free($1); }
	| THIS
	| SUPER
	| STRING
	| '(' expression ')'
	;

type: IDENTIFIER {
  $$ = currentType = getClass($1);
  free($1);
};

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
  else {
    syntaxError("unexpected end of file\n");
  }
  return 1;
}
