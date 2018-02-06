#include <stdio.h>
#include <stdlib.h>
#include "MyHash.h"
#include "symtable.h"
#include "errReport.h"
#include "ArrayList.h"

const char *SymbolKindName[] = {
  "parameter",
  "variable"
};

static struct ArrayList argTypeList = {0};
struct MyHash symTable; // char* -> SymTableEntry

struct SymTableEntry **symStack;
size_t stackSize, stackTop;
#define INIT_STACK_SIZE 5
int localVarCount;

int curScopeLevel;

static inline void OutOfMemory() {
  fprintf(stderr, "Out of memory!\n");
  exit(EXIT_FAILURE);
}

void initSymTable(void) {
  ArrayList_init(&argTypeList);
  MyHash_init(&symTable, MyHash_strcmp, MyHash_strhash);
  stackSize = INIT_STACK_SIZE;
  stackTop = 0;
  symStack = malloc(sizeof(struct SymTableEntry *) * stackSize);
  if (symStack == NULL) OutOfMemory();
  curScopeLevel = 0;
}

void destroySymTable(void) {
  while (stackTop > 0) {
    popSymbol();
  }
  stackSize = 0;
  free(symStack);
  curScopeLevel = 0;
  ArrayList_destroy(&argTypeList);
  free(symTable._buckets);
}

struct ArgType getArgumentList(void) {
  struct ArgType a;
  int i;
  a.arity = argTypeList.size;
  a.types = malloc(a.arity * sizeof(ClassType));
  for (i = 0; i < a.arity; i++) {
    a.types[i] = argTypeList.items[i];
  }
  ArrayList_clear(&argTypeList);
  return a;
}

static void needResizeStack(void) {
  if (stackTop >= stackSize) {
    stackSize *= 2;
    struct SymTableEntry **newstack = realloc(symStack, sizeof(struct SymTableEntry *) * stackSize);
    if (newstack == NULL) OutOfMemory();
    symStack = newstack;
  }
}

static struct SymTableEntry *createSymEntry(char *name, enum SymbolKind kind) {
  struct SymTableEntry *en = malloc(sizeof(struct SymTableEntry));
  if (NULL == en) OutOfMemory();
  en->name = name;
  en->kind = kind;
  en->level = curScopeLevel;
  en->type = NULL;
  en->attr.tag = Attribute_NONE;
  en->prev = NULL;
  return en;
}

// name is copied
bool addSymbol(const char *name, enum SymbolKind kind) {
  char *nameDup = dupstr(name);
  needResizeStack();
  struct SymTableEntry *en = createSymEntry(nameDup, kind);
  struct SymTableEntry *old = MyHash_set(&symTable, nameDup, en);
  if (old != NULL) {
    if (old->level == curScopeLevel) {
      semanticError("symbol " BOLD_TEXT "%s" NORMAL_TEXT " is redeclared\n", name);
      MyHash_set(&symTable, nameDup, old);
      free(en);
      free(nameDup);
      return false;
    }
    else { // symbol in lower level scope
      symStack[stackTop] = en;
      stackTop++;
      en->prev = old;
    }
  }
  else {
    symStack[stackTop] = en;
    stackTop++;
  }
  return true;
}

void popSymbol(void) {
  stackTop--;
  struct SymTableEntry *top = symStack[stackTop];
  if (top->attr.tag == Attribute_LOCALVAR) {
    localVarCount--;
  }
  if (top->prev == NULL) { // no shadowed symbol
    struct HashBucket *b = MyHash_delete(&symTable, top->name);
    free(b);
  }
  else {
    MyHash_set(&symTable, top->name, top->prev);
  }
  free(top->name);
  destroyAttribute(&top->attr);
  free(top);
  symStack[stackTop] = NULL;
}

void destroyAttribute(struct Attribute *attr) {
}

void showAttribute(struct Attribute attr) {
  if (attr.tag == Attribute_LOCALVAR) {
    printf("temp var id %d", attr.tmpVarId);
  }
}

void addParamVar(ClassType cls, const char *name) {
  ArrayList_add(&argTypeList, cls);
  bool yes = addSymbol(name, SymbolKind_parameter);
  if (yes) {
    symStack[stackTop-1]->type = cls;
    symStack[stackTop-1]->attr.tag = Attribute_LOCALVAR;
    symStack[stackTop-1]->attr.tmpVarId = localVarCount++;
  }
}

int addLocalVar(ClassType cls, const char *name) {
  bool yes = addSymbol(name, SymbolKind_variable);
  if (yes) {
    symStack[stackTop-1]->type = cls;
    symStack[stackTop-1]->attr.tag = Attribute_LOCALVAR;
    symStack[stackTop-1]->attr.tmpVarId = localVarCount;
    return localVarCount++;
  }
  return -1;
}

void showScope(size_t stackstart) {
  int i;
  for (i = 0; i < 110; i++)
    printf("=");
  printf("\n");
  printf("%-33s%-11s%-11s%-17s%-11s\n","Name","Kind","Level","Type","Attribute");
  for (i = 0; i < 110; i++)
    printf("-");
  printf("\n");
  size_t j;
  for (j = stackstart; j < stackTop; j++) {
    printf("%-33s", symStack[j]->name);
    printf("%-11s", SymbolKindName[symStack[j]->kind]);
    printf("%d%-10s", symStack[j]->level, symStack[j]->level == 0 ? "(global)" : "(local)");
    int n = showClassName(symStack[j]->type), k;
    for (k = n; k < 17; k++) putchar(' '); // align
    showAttribute(symStack[j]->attr);
    printf("\n");
  }
  for (i = 0; i < 110; i++)
    printf("-");
  printf("\n");
}

int pushScope(void) {
  curScopeLevel++;
  if (curScopeLevel == 1) {
    localVarCount = 0;
  }
  return curScopeLevel;
}

void popScope(void) {
  curScopeLevel--;
  size_t i = stackTop;
  while (i > 0 && symStack[i-1]->level > curScopeLevel) {
    i--;
  }
  size_t j;
  for (j = stackTop; j > i; j--) {
    popSymbol();
  }
}

struct SymTableEntry *getSymEntry(const char *name) {
  return MyHash_get(&symTable, name);
}

const char *getLocalVarName(int tmpVarId) {
  struct SymTableEntry *e = symStack[tmpVarId];
  if (e->attr.tag == Attribute_LOCALVAR && e->attr.tmpVarId == tmpVarId) {
    return e->name;
  }
  printf("assert failed! tmp var %d is not symStack[%d]\n", tmpVarId, tmpVarId);
  exit(EXIT_FAILURE);
}
