#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "errReport.h"
#include "ArrayList.h"

void compileAllClasses(void) {
  int i;
  for (i = 0; i < classCount; i++) {
    struct Class *c = classTable[i];
    if (c == getVoidClass()) continue;
    size_t nlen = strlen(c->name);
    genClassHeader(c);
    int j;
    for (j = 0; j < c->methodCount; j++) {
      struct Method *m = c->methodTable[j];
      if (m->thisClass == c) { // own class
        genMethod(m);
      }
    }
  }
}

void genClassHeader(struct Class *c) {

}

void genMethod(struct Method *m) {
  printf("%s %s::%s", m->returnType->name, m->thisClass->name, m->name);
  showSignature(m->args);
  puts(" compiling");
  if (m->ast != NULL) {
    struct ClassTypeAndIntPair status = genStatement(m->ast, 1, m->thisClass);
    if (status.n == 0) { // no statement
      printf("  this\n  return\n");
    }
    else { // has statement
      if (status.type != NULL) {
        if (!isKindOf(status.type, m->returnType)) {
          semanticError("return type mismatch, return type is ");
          printf("%s", status.type->name);
          printf(" but ");
          printf("%s", m->returnType->name);
          printf(" is expected\n");
        }
      }
      printf("  return\n");
    }
  }
}

struct ClassTypeAndIntPair genStatement(struct Statement *s, int first, ClassType thisType) {
  struct ClassTypeAndIntPair a;
  int ret = 0;
  a.type = NULL;
  while (s != NULL && !ret) {
    if (s->type == Stmt_COMPOUND) {
      struct ClassTypeAndIntPair st = genStatement(s->stmt, first, thisType);
      if (st.n == -1) {
        ret = 1;
        a.type = st.type;
      }
      else if (st.n == 1) {
        first = 0;
        a.type = st.type;
      }
    }
    else if (s->type == Stmt_RETURN || s->type == Stmt_SIMPLE) {
      if (!first) {
        printf("  pop\n");
      }
      genExpr(s->expr, thisType);
      a.type = s->expr->type;
      if (s->type == Stmt_RETURN) {
        ret = 1;
      }
      first = 0;
    }
    else {
      printf("unknown statement type %d\n", s->type);
      exit(EXIT_FAILURE);
    }
    s = s->next;
  }
  if (ret && s != NULL) {
    //semanticError("unreachable statement\n");
    printf("warning: unreachable statement\n");
  }
  a.n = ret ? -1 : (first ? 0 : 1);
  return a;
}

void genExpr(struct Expr *expr, ClassType thisType) {
  switch (expr->op) {
    case Op_ASSIGN: genAssign(expr, thisType); break;
    case Op_NEW: genNewExpr(expr, thisType); break;
    case Op_DOT: genDotExpr(expr, thisType); break;
    case Op_FUNC: genFuncExpr(expr, thisType); break;
    case Op_LIT: printf("  str \"%s\"\n", expr->lit.str); break;
    case Op_THIS: printf("  this\n"); expr->type = thisType; break;
    case Op_SUPER: printf("  this\n"); expr->type = thisType; break;
    case Op_VAR: genVarExpr(expr, thisType); break;
    case Op_LOCAL: printf("  load %d\n", expr->varId); break;
    case Op_NULL: printf("  null\n"); break;
    default:
      printf("unknown expression type %d\n", expr->op);
      exit(EXIT_FAILURE);
  }
}

void genTypeConvert(struct Class *from, struct Class *to) {
  if (from == NULL) {
    return;
  }
  if (isKindOf(from, to)) return; // no need to convert
  if (isKindOf(to, from)) {
    printf("  convert %d (class %s)\n", to->id, to->name);
  }
  else {
    semanticError("cannot convert %s to %s\n", from->name, to->name);
  }
}

void genAssign(struct Expr *expr, ClassType thisType) {
  genExpr(expr->args->next, thisType);
  /*
  * assign can be:
  * 1. obj DOT name = expr
  * 2. LOCAL = expr
  * 3. VAR = expr
  */
  if (expr->args->op == Op_DOT) {
    struct Class *c;
    if (expr->args->args->op == Op_THIS) c = thisType;
    else if (expr->args->args->op == Op_SUPER) c = thisType->base;
    else {
      semanticError("only field variable of this or super is accessible\n");
      return ;
    }
    char *name = expr->args->args->next->name;
    struct Field *f = MyHash_get(&c->fields, name);
    if (f == NULL) {
      semanticError("undefined variable %s%s\n", name, c == thisType ? "" : " in super class");
    }
    else {
      expr->type = f->type;
      genTypeConvert(expr->args->next->type, expr->type);
      printf("  putfield %d\n", f->id);
    }
  }
  else if (expr->args->op == Op_LOCAL) {
    expr->type = expr->args->type;
    genTypeConvert(expr->args->next->type, expr->type);
    printf("  store %d\n", expr->args->varId);
  }
  else if (expr->args->op == Op_VAR) {
    struct Field *f = MyHash_get(&thisType->fields, expr->args->name);
    if (f == NULL) {
      semanticError("undefined variable: %s\n", expr->args->name);
    }
    else {
      expr->type = f->type;
      genTypeConvert(expr->args->next->type, expr->type);
      printf("  putfield %d\n", f->id);
    }
  }
  else {
    semanticError("left hand side of \"=\" is not an lvalue\n");
  }
}

void genVarExpr(struct Expr *expr, ClassType thisType) {
  // can only be field variable
  struct Field *f = MyHash_get(&thisType->fields, expr->name);
  if (f == NULL) {
    semanticError("undefined variable: %s\n", expr->name);
  }
  else {
    expr->type = f->type;
    printf("  getfield %d\n", f->id);
  }
}

void genDotExpr(struct Expr *expr, ClassType thisType) {
  // can only be field variable
  struct Class *c;
  if (expr->args->op == Op_THIS) c = thisType;
  else if (expr->args->op == Op_SUPER) c = thisType->base;
  else {
    semanticError("only field variable of this or super is accessible\n");
    return ;
  }
  char *name = expr->args->next->name;
  struct Field *f = MyHash_get(&c->fields, name);
  if (f == NULL) {
    semanticError("undefined variable %s%s\n", name, c == thisType ? "" : " in super class");
  }
  else {
    expr->type = f->type;
    genTypeConvert(expr->args->next->type, expr->type);
    printf("  getfield %d\n", f->id);
  }
}

void genNewExpr(struct Expr *expr, ClassType thisType) {
  /*
  * new can be:
  * 1. new name without args
  * 2. new name(args)
  */
  struct Expr *p = expr;
  char *name;
  struct Method *m;
  if (p->args->op == Op_FUNC) {
    p = p->args->args->next;
    while (p != NULL) {
      genExpr(p, thisType);
      p = p->next;
    }
    name = expr->args->args->name;
    p = expr->args->args->next;
  }
  else {
    name = expr->args->name;
    p = NULL;
  }
  struct Class *c = getClass(name);
  m = getBestFitMethod(c, "<init>", p);
  if (c->defined) {
    printf("  new %d (class %s)\n", c->id, c->name);
    if (m != NULL) {
      printf("  dup\n");
      printf("  callspecial %d %d\n", m->thisClass->id, m->id);
      printf("  pop\n");
    }
    expr->type = c;
  }
  else {
    semanticError("class %s is undefined\n", c->name);
  }
}

void genFuncExpr(struct Expr *expr, ClassType thisType) {
  /*
  * calls can be:
  * 1. this ( args... ) -> constructor
  * 2. super ( args... ) -> constructor
  * 3. name ( args... ) -> method
  * 4. obj . name ( args... ) -> method
  * 5. super . name ( args... ) -> method
  */
  struct Expr *p;
  struct Class *c;
  char *name;
  struct Method *m;
  bool special = false;
  switch (expr->args->op) {
    case Op_DOT:
      p = expr->args->args;
      genExpr(expr->args->args, thisType);
      if (p->op == Op_SUPER) {
        c = thisType->base;
        special = true;
      }
      else {
        c = expr->args->args->type;
      }
      name = expr->args->args->next->name;
      break;
    case Op_THIS:
      c = thisType;
      name = "<init>";
      special = true;
      break;
    case Op_SUPER:
      c = thisType->base;
      name = "<init>";
      special = true;
      break;
    case Op_VAR:
      c = thisType;
      name = expr->args->name;
      break;
    default:
      printf("unknown op %d in func expr\n", expr->args->op);
      exit(EXIT_FAILURE);
  }
  p = expr->args->next;
  while (p != NULL) {
    genExpr(p, thisType);
    p = p->next;
  }
  m = getBestFitMethod(c, name, expr->args->next);
  if (m != NULL) {
    if (special)
      printf("  callspacial %d %d\n", m->thisClass->id, m->id);
    else
      printf("  call %d\n", m->id);
    expr->type = m->returnType;
  }
}

struct Method *getBestFitMethod(struct Class *cls, const char *name, struct Expr *args) {
  struct Expr *p = args;
  int argn = 0, i;
  struct Class **types;
  struct ArrayList *candidates;
  if (cls == NULL) return NULL;
  candidates = MyHash_get(&cls->methods, name);
  if (candidates == NULL || candidates->size == 0) {
    semanticError("class %s doesn't have method %s\n", cls->name, name);
    return NULL;
  }
  while (p != NULL) {
    argn++;
    if (p->type == NULL) return NULL; /* unknown type */
    p = p->next;
  }
  types = malloc(sizeof(struct Class*) * argn);
  for (i = 0; i < argn; i++) {
    types[i] = getVoidClass();
  }
  /* get best fit argument type */
  for (i = 0; i < candidates->size; i++) {
    struct Method *m = ArrayList_get(candidates, i);
    int j;
    p = args;
    if (m->args.arity != argn) continue;
    for (j = 0; j < argn; j++) {
      if (isKindOf(p->type, m->args.types[j])) {
        if (isKindOf(m->args.types[j], types[j])) { /* better fit */
          types[j] = m->args.types[j];
        }
      }
      else break;
      p = p->next;
    }
  }
  /* get method with exactly the same signature */
  for (i = 0; i < candidates->size; i++) {
    struct Method *m = ArrayList_get(candidates, i);
    int j;
    if (m->args.arity != argn) continue;
    for (j = 0; j < argn; j++) {
      if (types[j] != m->args.types[j])
        break;
    }
    if (j == argn) {
      free(types);
      return m; /* best fit found */
    }
  }
  /* no best fit overloading exists -> list possible overloading */
  free(types);
  semanticError("call of overloaded method \"%s\" is ambiguous. candidates are:\n", name);
  for (i = 0; i < candidates->size; i++) {
    struct Method *m = ArrayList_get(candidates, i);
    int j;
    p = args;
    if (m->args.arity != argn) continue;
    for (j = 0; j < argn; j++) {
      if (!isKindOf(p->type, m->args.types[j]))
        break;
      p = p->next;
    }
    if (j == argn) { /* print */
      printf("%s %s", m->returnType->name, m->name);
      showSignature(m->args);
      printf(" in line %d\n", m->linenum);
    }
  }
  return NULL;
}