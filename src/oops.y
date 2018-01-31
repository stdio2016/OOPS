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
}

// keywords
%token CLASS RETURN NEW THIS SUPER
%token<str> IDENTIFIER STRING
%token ERROR

%start program
%type<str> name varDecl
%type<cls> inherit type
%type<argList> argumentList

%destructor { free($$); } <str>
%destructor { } <cls>
%destructor { free($$.types); } <argList>

%%
program:
	/* empty */
	| program class
	;

class:
	CLASS name inherit {
	  thisClass = createClass($2, $3);
	}
	'{' classBody '}' {
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
	type name '(' argumentList ')' {
	  addMethod(thisClass, $1, $2, $4);
	}
	block { free($2); }
	;

constructor:
	name '(' argumentList ')' {
	  struct ArgType a;
	  addConstructor(thisClass, $1, $3);
	}
	block { free($1); }
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
