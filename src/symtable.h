#pragma once
#ifndef SYMTABLE_INCLUDED
#define SYMTABLE_INCLUDED
#include "ast.h"
#include "class.h"

enum SymbolKind {
  SymbolKind_parameter,
  SymbolKind_variable
};

extern const char *SymbolKindName[];

struct Attribute {
  enum {
    Attribute_NONE, Attribute_LOCALVAR
  } tag;
  union {
    int tmpVarId;
  };
};

struct SymTableEntry {
  char *name;
  enum SymbolKind kind;
  int level;
  ClassType type;
  struct Attribute attr;
  struct SymTableEntry *prev;
};

void initSymTable(void);
void destroySymTable(void);

struct ArgType getArgumentList(void);

// name is copied
bool addSymbol(const char *name, enum SymbolKind kind);
void popSymbol(void);

void destroyAttribute(struct Attribute *attr);
void showAttribute(struct Attribute attr);

void addParamVar(ClassType cls, const char *name);
// return local var id or -1 means fail
int addLocalVar(ClassType cls, const char *name);

void showScope(size_t stackstart);
int pushScope(void);
void popScope(void);

struct SymTableEntry *getSymEntry(const char *name);
#endif
