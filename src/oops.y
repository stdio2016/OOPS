%{
#include <ctype.h>
#include <stdio.h>
#include <string.h>
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
  struct Expr *expr;
  struct ExprList args;
  struct Statement *stmt;
  struct StatementList stmtList;
  struct Method *method;
}

// keywords
%token CLASS RETURN NEW THIS SUPER NUL
%token<str> IDENTIFIER STRING
%token ERROR

%start program
%type<str> name
%type<cls> inherit type
%type<argList> argumentList
%type<scope> PushScope
%type<expr> expression newExpr callExpr atom nameNode
%type<args> callArgs callArgsNonEmpty
%type<stmt> statement varDecls return block methodBody varDecl
%type<stmtList> statements varList
%type<method> methodHead constructorHead

%destructor { free($$); } <str>
%destructor { } <cls>
%destructor { free($$.types); } <argList>
%destructor { popScope(); } <scope>
%destructor { destroyExpr($$); } <expr>
%destructor { destroyExpr($$.first); } <args>
%destructor { destroyStmt($$); } <stmt>
%destructor { destroyStmt($$.first); } <stmtList>
%destructor { } <method>

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
	methodHead methodBody {
	  if ($1 != NULL)
	    $1->ast = $2;
	  else
	    destroyStmt($2);
	}
	;

methodHead:
	type name '(' PushScope argumentList ')' {
	  int scope = $4;
	  $$ = addMethod(Method_METHOD, thisClass, $1, $2, $5);
	  free($2);
	}
	;

constructor:
	constructorHead methodBody {
	  if ($1 != NULL)
	    $1->ast = $2;
	  else
	    destroyStmt($2);
	}
	;

constructorHead:
	name '(' PushScope argumentList ')' {
	  int scope = $3;
	  if (strcmp($1, thisClass->name)) {
	    semanticError("constructor name and class name differs\n");
	    $$ = NULL;
	  }
	  else
	    $$ = addConstructor(Method_CONSTRUCTOR, thisClass, $4);
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
	'{' statements '}' { popScope(); $$ = createCompoundStmt($2); }
	;

block:
	'{' PushScope statements '}' { int s = $2; popScope(); $$ = createCompoundStmt($3); }
	;

PushScope: { $$ = pushScope(); };

statements:
	/* empty */ { initStmtList(&$$); }
	| statements statement { addToStmtList(&$1, $2); $$ = $1; }
	;

statement:
	varDecls
	| return
	| block
	| expression ';' { showExpr($1, 2); $$ = createStmt(Stmt_SIMPLE, $1); }
	;

varDecls:
	type varList ';' { currentType = $1; $$ = createCompoundStmt($2); }
	;

varList:
	varDecl { initStmtList(&$$); addToStmtList(&$$, $1); }
	| varList ',' varDecl { addToStmtList(&$1, $3); $$ = $1; }
	;

varDecl:
	name { addLocalVar(currentType, $1); $$ = NULL; free($1); }
	| name '=' expression {
	    int id = addLocalVar(currentType, $1);
	    struct Expr *var = createLocalVarExpr(id);
	    var->type = currentType;
	    struct Expr *as = createExpr(Op_ASSIGN, var, $3);
	    $$ = createStmt(Stmt_SIMPLE, as);
	    showExpr(as, 2);
	    free($1);
	  }
	;

return:
	RETURN expression ';' { showExpr($2, 2); $$ = createStmt(Stmt_RETURN, $2); }
	;

expression:
	newExpr
	| newExpr '=' expression { $$ = createExpr(Op_ASSIGN, $1, $3); }
	;

newExpr:
	callExpr
	| NEW newExpr { $$ = createExpr(Op_NEW, $2, NULL); }
	;

callExpr:
	atom
	| callExpr '(' callArgs ')' { // $1 cannot be a local variable node
	    if ($1->op == Op_LOCAL) {
	      int n = $1->varId;
	      destroyExpr($1);
	      char *name = dupstr(getLocalVarName(n));
	      $1 = createVarExpr(name);
	    }
	    $$ = createFuncExpr($1, $3.first);
	  }
	| callExpr '.' name { $$ = createExpr(Op_DOT, $1, createVarExpr($3)); }
	;

callArgs:
	/* empty */ { initExprList(&$$); }
	| callArgsNonEmpty
	;

callArgsNonEmpty:
	expression
	{
	  initExprList(&$$);
	  addToExprList(&$$, $1);
	}
	| callArgsNonEmpty ',' expression
	{
	  addToExprList(&$1, $3); $$ = $1;
	}
	;

atom:
	nameNode
	| THIS { $$ = createExpr(Op_THIS, NULL, NULL); }
	| SUPER { $$ = createExpr(Op_SUPER, NULL, NULL); }
	| NUL { $$ = createExpr(Op_NULL, NULL, NULL); }
	| STRING {
	    struct Constant a;
	    a.type = Type_STRING;
	    a.str = $1;
	    $$ = createLitExpr(a);
	  }
	| '(' expression ')' { $$ = $2; }
	;

type: IDENTIFIER {
  $$ = currentType = getClass($1);
  free($1);
};

nameNode: IDENTIFIER {
  struct SymTableEntry *e = getSymEntry($1);
  if (e == NULL || e->attr.tag != Attribute_LOCALVAR) {
    $$ = createVarExpr($1);
  }
  else {
    $$ = createLocalVarExpr(e->attr.tmpVarId);
    $$->type = e->type;
    free($1);
  }
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
