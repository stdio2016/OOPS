#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "errReport.h"
#include "ArrayList.h"
#include "StringBuffer.h"
#include "vm.h"
#define EMIT(b1) StrBuf_appendChar(&CodeBuf, b1)

extern int linenum;

static void genClassHeader(struct Class *c);
static void genMethod(struct Method *m);

struct ClassTypeAndIntPair{
  ClassType type;
  int n;
};
// return type of the last statement
// return 0: no value on stack
// return 1: a value on stack
// return -1: value returned
static struct ClassTypeAndIntPair genStatement(struct Statement *s, int first, ClassType thisType);

static void genExpr(struct Expr *expr, ClassType thisType);
static void genTypeConvert(struct Class *from, struct Class *to);
static void genAssign(struct Expr *expr, ClassType thisType);
static void genVarExpr(struct Expr *expr, ClassType thisType);
static void genDotExpr(struct Expr *expr, ClassType thisType);
static void genNewExpr(struct Expr *expr, ClassType thisType);
static void genFuncExpr(struct Expr *expr, ClassType thisType);

static struct StringBuffer CodeBuf;
static int localVarCount;

void compileAllClasses(void) {
  int i;
  StrBuf_init(&CodeBuf);
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
  StrBuf_destroy(&CodeBuf);
}

static void emitOpWithOneArg(enum VM_Bytecodes op, unsigned num) {
  unsigned char a[4];
  a[0] = op;
  a[1] = num & 0xff;
  a[2] = num >> 8;
  StrBuf_appendN(&CodeBuf, a, 3);
}

static void emitOpWithTwoArgs(enum VM_Bytecodes op, unsigned num, unsigned num2) {
  unsigned char a[8];
  a[0] = op;
  a[1] = num & 0xff;
  a[2] = num >> 8;
  a[3] = num2 & 0xff;
  a[4] = num2 >> 8;
  StrBuf_appendN(&CodeBuf, a, 5);
}

static void genClassHeader(struct Class *c) {

}

static void genMethod(struct Method *m) {
  printf("%s %s::%s", m->returnType->name, m->thisClass->name, m->name);
  showSignature(m->args);
  puts(" compiling");
  StrBuf_clear(&CodeBuf);
  localVarCount = m->args.arity;
  if (m->ast != NULL) {
    struct ClassTypeAndIntPair status = genStatement(m->ast, 1, m->thisClass);
    if (status.n == 0) { // no statement
      if (!isKindOf(m->thisClass, m->returnType)) {
        semanticError("return type mismatch, return type is \"");
        printf("%s", m->thisClass->name);
        printf("\" but \"");
        printf("%s", m->returnType->name);
        printf("\" is expected\n");
      }
      EMIT(Instr_THIS);
      EMIT(Instr_RETURN);
    }
    else { // has statement
      if (status.type != NULL) {
        if (!isKindOf(status.type, m->returnType)) {
          semanticError("return type mismatch, return type is \"");
          printf("%s", status.type->name);
          printf("\" but \"");
          printf("%s", m->returnType->name);
          printf("\" is expected\n");
        }
      }
      EMIT(Instr_RETURN);
    }
    m->bytecode = malloc(CodeBuf.size);
    m->localCount = localVarCount;
    memcpy(m->bytecode, CodeBuf.buf, CodeBuf.size);
    printf(" locals %d\n", localVarCount);
    showBytecode(m->bytecode);
  }
}

static struct ClassTypeAndIntPair genStatement(struct Statement *s, int first, ClassType thisType) {
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
        EMIT(Instr_POP);
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

static void genExpr(struct Expr *expr, ClassType thisType) {
  linenum = expr->linenum;
  switch (expr->op) {
    case Op_ASSIGN: genAssign(expr, thisType); break;
    case Op_NEW: genNewExpr(expr, thisType); break;
    case Op_DOT: genDotExpr(expr, thisType); break;
    case Op_FUNC: genFuncExpr(expr, thisType); break;
    case Op_LIT: emitOpWithOneArg(Instr_STR, expr->lit.strId); expr->type = getVoidClass(); break;
    case Op_THIS: EMIT(Instr_THIS); expr->type = thisType; break;
    case Op_SUPER: EMIT(Instr_THIS); expr->type = thisType; break;
    case Op_VAR: genVarExpr(expr, thisType); break;
    case Op_LOCAL:
      emitOpWithOneArg(Instr_LOAD, expr->varId);
      if (expr->varId >= localVarCount) {
        localVarCount = expr->varId + 1;
      }
      break;
    case Op_NULL: EMIT(Instr_NULL); expr->type = getNullClass(); break;
    default:
      printf("unknown expression type %d\n", expr->op);
      exit(EXIT_FAILURE);
  }
}

static void genTypeConvert(struct Class *from, struct Class *to) {
  if (from == NULL) {
    return;
  }
  if (isKindOf(from, to)) return; // no need to convert
  if (isKindOf(to, from)) {
    emitOpWithOneArg(Instr_CONVERT, to->id);
  }
  else {
    semanticError("cannot convert %s to %s\n", from->name, to->name);
  }
}

static void genAssign(struct Expr *expr, ClassType thisType) {
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
      emitOpWithOneArg(Instr_PUTFIELD, f->id);
    }
  }
  else if (expr->args->op == Op_LOCAL) {
    expr->type = expr->args->type;
    genTypeConvert(expr->args->next->type, expr->type);
    emitOpWithOneArg(Instr_STORE, expr->args->varId);
    if (expr->args->varId >= localVarCount) {
      localVarCount = expr->args->varId + 1;
    }
  }
  else if (expr->args->op == Op_VAR) {
    struct Field *f = MyHash_get(&thisType->fields, expr->args->name);
    if (f == NULL) {
      semanticError("undefined variable: %s\n", expr->args->name);
    }
    else {
      expr->type = f->type;
      genTypeConvert(expr->args->next->type, expr->type);
      emitOpWithOneArg(Instr_PUTFIELD, f->id);
    }
  }
  else {
    semanticError("left hand side of \"=\" is not an lvalue\n");
  }
}

static void genVarExpr(struct Expr *expr, ClassType thisType) {
  // can only be field variable
  struct Field *f = MyHash_get(&thisType->fields, expr->name);
  if (f == NULL) {
    semanticError("undefined variable: %s\n", expr->name);
  }
  else {
    expr->type = f->type;
    emitOpWithOneArg(Instr_GETFIELD, f->id);
  }
}

static void genDotExpr(struct Expr *expr, ClassType thisType) {
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
    emitOpWithOneArg(Instr_GETFIELD, f->id);
  }
}

static void genNewExpr(struct Expr *expr, ClassType thisType) {
  /*
  * new can be:
  * 1. new name without args
  * 2. new name(args)
  */
  struct Expr *p = expr;
  char *name;
  struct Method *m;
  if (p->args->op == Op_FUNC) {
    name = expr->args->args->name;
  }
  else {
    name = expr->args->name;
  }
  struct Class *c = getClass(name);
  emitOpWithOneArg(Instr_NEW, c->id);
  EMIT(Instr_DUP);
  if (p->args->op == Op_FUNC) {
    p = p->args->args->next;
    while (p != NULL) {
      genExpr(p, thisType);
      p = p->next;
    }
    p = expr->args->args->next;
  }
  else {
    p = NULL;
  }
  m = getBestFitMethod(c, "<init>", p);
  if (c->defined) {
    if (m != NULL) {
      emitOpWithTwoArgs(Instr_CALLSPECIAL, m->thisClass->id, m->id);
      EMIT(Instr_POP);
    }
    expr->type = c;
  }
  else {
    semanticError("class %s is undefined\n", c->name);
  }
}

static void genFuncExpr(struct Expr *expr, ClassType thisType) {
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
      EMIT(Instr_THIS);
      c = thisType;
      name = "<init>";
      special = true;
      break;
    case Op_SUPER:
      EMIT(Instr_THIS);
      c = thisType->base;
      name = "<init>";
      special = true;
      break;
    case Op_VAR:
      EMIT(Instr_THIS);
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
      emitOpWithTwoArgs(Instr_CALLSPECIAL, m->thisClass->id, m->id);
    else
      emitOpWithTwoArgs(Instr_CALL, m->args.arity, m->id);
    expr->type = m->returnType;
  }
}

struct Method *getBestFitMethod(struct Class *cls, const char *name, struct Expr *args) {
  struct Expr *p = args;
  int argn = 0, i, fits = 0;
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
      if (!isKindOf(p->type, m->args.types[j])) break;
      p = p->next;
    }
    if (j == argn) {
      fits++;
      for (j = 0; j < argn; j++) {
        if (isKindOf(m->args.types[j], types[j])) { /* better fit */
          types[j] = m->args.types[j];
        }
        else if (!isKindOf(types[j], m->args.types[j])) { /* cannot determine which is more specific */
          types[j] = NULL;
        }
      }
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
  if (fits == 0) {
    semanticError("no suitable overloaded method \"%s\" exists\n", name);
    return NULL;
  }
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
