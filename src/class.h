#pragma once
#ifndef CLASS_INCLUDED
#define CLASS_INCLUDED
#include <stdbool.h>
#include "MyHash.h"
#include "ast.h"
#define ID_UNASSIGNED -1

extern int classCount;
extern struct Class **classTable;

struct Class {
  char *name;
  struct Class *base;
  bool defined;
  int linenum;
  int id, maxId;
  struct MyHash methods; // char* name -> list of struct Method*
  struct MyHash fields; // char* name -> struct Field*
  struct ArrayList *subclasses;
  struct Method **methodTable;
  struct Field **fieldTable;
  int fieldCount;
  int methodCount;
};

enum MethodFlags {
  Method_METHOD = 0,
  Method_CONSTRUCTOR = 1,
  Method_BUILTIN = 2 // no need to compile
};

struct Method {
  struct Class *thisClass;
  struct Class *returnType;
  char *name;
  enum MethodFlags flag;
  int linenum;
  int id;
  size_t refcount;
  struct ArgType args;
  struct Statement *ast;
  union {
    void *(*builtinFun)(void *args);
    unsigned char *bytecode;
  };
};

struct Field {
  struct Class *thisClass;
  struct Class *type;
  char *name;
  size_t refcount;
  int id;
};

void initClassTable(void);
void destroyClassTable(void);

ClassType getVoidClass(void);
ClassType getClass(const char *name);
ClassType createClass(const char *name, ClassType baseClass);
void destroyClass(ClassType cls);
bool isKindOf(struct Class *some, struct Class *base);

void showSignature(struct ArgType args);
bool isSameSignature(struct ArgType args1, struct ArgType args2);

void addField(ClassType cls, ClassType type, const char *name);
struct Method *addMethod(enum MethodFlags flag, ClassType cls, ClassType returnType, const char *name, struct ArgType arguments);
struct Method *addConstructor(enum MethodFlags flag, ClassType cls, struct ArgType arguments);
void showMethodFlag(enum MethodFlags flag);

void destroyMethod(struct Method *method);
void destroyField(struct Field *field);

int showClassName(ClassType type); // returns chars printed
void showClassInterfaces(struct Class *cls);

void giveClassId(void);

void processInheritance(void);
#endif
