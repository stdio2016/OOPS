#include <stdio.h>
#include <stdlib.h>
#include "class.h"
#include "errReport.h"
#include "ArrayList.h"

struct MyHash allClasses;
ClassType *classTable;
ClassType VoidClass;

extern int linenum; // defined in oops.lex

void initClassTable() {
  MyHash_init(&allClasses, MyHash_strcmp, MyHash_strhash);
  VoidClass = createClass("void", NULL);
}

void classTableDestructor(struct HashBucket *hb) {
  destroyClass(hb->value);
  free(hb);
}

void destroyClassTable() {
  MyHash_destroy(&allClasses, classTableDestructor);
  free(classTable);
}

ClassType getVoidClass() {
  return VoidClass;
}

ClassType getClass(const char *name) {
  ClassType t = MyHash_get(&allClasses, name);
  if (t != NULL) return t;
  t = createClass(name, NULL);
  t->defined = false;
  return t;
}

ClassType createClass(const char *name, ClassType baseClass) {
  ClassType t = MyHash_get(&allClasses, name);
  if (t != NULL) {
    if (t->defined) {
      semanticError("class " BOLD_TEXT "%s" NORMAL_TEXT " is redeclared\n", name);
    }
    else {
      t->base = baseClass;
      t->defined = true;
      t->linenum = linenum;
    }
    return t;
  }
  t = malloc(sizeof(struct Class));
  t->name = dupstr(name);
  t->base = baseClass;
  t->defined = true;
  t->linenum = linenum;
  t->subclasses = malloc(sizeof(struct ArrayList));
  ArrayList_init(t->subclasses);
  MyHash_init(&t->methods, MyHash_strcmp, MyHash_strhash);
  MyHash_init(&t->fields, MyHash_strcmp, MyHash_strhash);
  MyHash_set(&allClasses, t->name, t);
  return t;
}

void methodTableDestructor(struct HashBucket *hb) {
  struct ArrayList *arr = hb->value;
  size_t i;
  for (i = 0; i < arr->size; i++) {
    destroyMethod(ArrayList_get(arr, i));
  }
  ArrayList_destroy(arr);
  free(arr);
  free(hb->key);
  free(hb);
}

void fieldTableDestructor(struct HashBucket *hb) {
  destroyField(hb->value);
  free(hb);
}

void destroyClass(ClassType cls) {
  free(cls->name);
  MyHash_destroy(&cls->methods, methodTableDestructor);
  MyHash_destroy(&cls->fields, fieldTableDestructor);
  ArrayList_destroy(cls->subclasses);
  free(cls->subclasses);
  free(cls);
}

void addField(ClassType cls, ClassType type, const char *name) {
  printf("  field %s\n", name);
  struct Field *f = MyHash_get(&cls->fields, name);
  if (f == NULL) {
    f = malloc(sizeof(*f));
    f->name = dupstr(name);
    f->type = type;
    f->refcount = 1;
    MyHash_set(&cls->fields, f->name, f);
  }
  else {
    semanticError("field " BOLD_TEXT "%s" NORMAL_TEXT " is already defined in class %s\n", name, cls->name);
  }
}

void showSignature(struct ArgType args) {
  int i;
  putchar('(');
  for (i = 0; i < args.arity; i++) {
    if (i > 0) putchar(',');
    if (args.types[i] == NULL) {
      printf("??");
    }
    else {
      printf("%s", args.types[i]->name);
    }
  }
  putchar(')');
}

struct Method *addMethod(enum MethodFlags flag, ClassType cls, ClassType returnType, const char *name, struct ArgType arguments) {
  printf("  ");
  showMethodFlag(flag);
  printf(" %s", name);
  showSignature(arguments);
  puts("");
  struct ArrayList *arr = MyHash_get(&cls->methods, name);
  if (arr == NULL) {
    arr = malloc(sizeof(struct ArrayList));
    ArrayList_init(arr);
    MyHash_set(&cls->methods, dupstr(name), arr);
  }
  size_t n = arr->size, i;
  bool success = true;
  for (i = 0; i < n; i++) {
    // check if there is a method with the same signature
    struct Method *m = ArrayList_get(arr, i);
    size_t nargs = m->args.arity, j;
    if (nargs != arguments.arity) continue; // arg count differs
    for (j = 0; j < nargs; j++) {
      if (m->args.types[j] != arguments.types[j])
        break; // arg type differs
    }
    if (j < nargs) continue; // arg type differs
    semanticError("");
    showMethodFlag(flag);
    printf(" " BOLD_TEXT "%s" NORMAL_TEXT, name);
    showSignature(arguments);
    printf(" is already defined in class %s\n", cls->name);
    success = false;
  }
  if (success) {
    struct Method *m = malloc(sizeof(struct Method));
    m->thisClass = cls;
    m->returnType = returnType;
    m->name = dupstr(name);
    m->flag = flag;
    m->refcount = 1;
    m->args = arguments;
    m->ast = NULL;
    ArrayList_add(arr, m);
    return m;
  }
  else {
    destroyArgType(arguments);
  }
  return NULL;
}

void showMethodFlag(enum MethodFlags flag) {
  if (flag & Method_CONSTRUCTOR) printf("constructor");
  else printf("method");
}

void destroyMethod(struct Method *method) {
  method->refcount--;
  if (method->refcount == 0) {
    free(method->name);
    destroyArgType(method->args);
    destroyStmt(method->ast);
    free(method);
  }
}

void destroyField(struct Field *field) {
  field->refcount--;
  if (field->refcount == 0) {
    free(field->name);
    free(field);
  }
}

int showClassName(ClassType type) {
  return printf("%s", type->name);
}

int dfsSubClasses(ClassType base, int id) {
  size_t i, n = base->subclasses->size;
  classTable[id] = base;
  base->id = id;
  ++id;
  for (i = 0; i < n; i++) {
    ClassType sub = ArrayList_get(base->subclasses, i);
    id = dfsSubClasses(sub, id);
  }
  base->maxId = id;
  printf("class %s id is [%d , %d)\n", base->name, base->id, base->maxId);
  return id;
}

void giveClassId() {
  size_t n = allClasses._size, i;
  struct MyHashIterator it;
  classTable = malloc(sizeof(ClassType) * n);
  // find undeclared classes
  MyHash_iterate(&allClasses, &it);
  for (i = 0; i < n; i++) {
    ClassType cls = it.it->value;
    cls->id = -1;
    cls->maxId = -1;
    if (!cls->defined) {
      linenum = cls->linenum;
      semanticError("class %s is undefined\n", cls->name);
    }
    MyHash_next(&it);
  }

  // update subclass list
  MyHash_iterate(&allClasses, &it);
  for (i = 0; i < n; i++) {
    ClassType cls = it.it->value;
    if (cls->base != NULL) {
      ArrayList_add(cls->base->subclasses, cls);
    }
    MyHash_next(&it);
  }
  // really give class id
  dfsSubClasses(VoidClass, 0);

  // find classes without id
  MyHash_iterate(&allClasses, &it);
  for (i = 0; i < n; i++) {
    ClassType cls = it.it->value;
    if (cls->id == -1 && cls->defined) {
      linenum = cls->linenum;
      semanticError("class %s has circular inheritance\n", cls->name);
    }
    MyHash_next(&it);
  }
}
