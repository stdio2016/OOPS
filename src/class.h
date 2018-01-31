#pragma once
#ifndef CLASS_INCLUDED
#define CLASS_INCLUDED
#include <stdbool.h>
#include "MyHash.h"
#include "ast.h"

struct Class {
  char *name;
  bool defined;
  struct MyHash methods; // char* name -> list of struct Method*
  struct MyHash fields; // char* name -> struct Field*
};

struct Method {
  struct Class *thisClass;
  struct Class *returnType;
  char *name;
  struct ArgType args;
};

struct Field {
  struct Class *type;
  char *name;
};

ClassType getVoidClass();
ClassType getClass(const char *name);
ClassType createClass(const char *name, ClassType baseClass);

void showClassSignature(struct ArgType args);

void addField(ClassType cls, ClassType type, const char *name);
void addMethod(ClassType cls, ClassType returnType, const char *name, struct ArgType arguments);
void addConstructor(ClassType cls, const char *name, struct ArgType arguments);
#endif
