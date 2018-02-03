#pragma once
#ifndef CLASS_INCLUDED
#define CLASS_INCLUDED
#include <stdbool.h>
#include "MyHash.h"
#include "ast.h"

struct Class {
  char *name;
  ClassType base;
  bool defined;
  struct MyHash methods; // char* name -> list of struct Method*
  struct MyHash fields; // char* name -> struct Field*
};

enum MethodFlags {
  Method_METHOD = 0,
  Method_CONSTRUCTOR = 1
};

struct Method {
  struct Class *thisClass;
  struct Class *returnType;
  char *name;
  enum MethodFlags flag;
  size_t refcount;
  struct ArgType args;
};

struct Field {
  struct Class *type;
  char *name;
  size_t refcount;
};

void initClassTable(void);
void destroyClassTable(void);

ClassType getVoidClass(void);
ClassType getClass(const char *name);
ClassType createClass(const char *name, ClassType baseClass);
void destroyClass(ClassType cls);

void showSignature(struct ArgType args);

void addField(ClassType cls, ClassType type, const char *name);
void addMethod(enum MethodFlags flag, ClassType cls, ClassType returnType, const char *name, struct ArgType arguments);
void showMethodFlag(enum MethodFlags flag);

void destroyMethod(struct Method *method);
void destroyField(struct Field *field);

int showClassName(ClassType type); // returns chars printed
#endif