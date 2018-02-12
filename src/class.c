#include <stdio.h>
#include <stdlib.h>
#include "class.h"
#include "errReport.h"
#include "ArrayList.h"

struct MyHash allClasses;
ClassType *classTable;
ClassType VoidClass;
int classCount;

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
  t->methodTable = NULL;
  t->fieldTable = NULL;
  t->fieldCount = 0;
  t->methodCount = 0;
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
  free(cls->fieldTable);
  free(cls->methodTable);
  free(cls);
}

bool isKindOf(struct Class *some, struct Class *base) {
  return some->id >= base->id && some->id < base->maxId;
}

void addField(ClassType cls, ClassType type, const char *name) {
  struct Field *f = MyHash_get(&cls->fields, name);
  if (f == NULL) {
    f = malloc(sizeof(*f));
    f->name = dupstr(name);
    f->type = type;
    f->refcount = 1;
    f->thisClass = cls;
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

bool isSameSignature(struct ArgType args1, struct ArgType args2) {
  if (args1.arity != args2.arity) return false; // arg count differs
  int i;
  for (i = 0; i < args1.arity; i++) {
    if (args1.types[i] != args2.types[i])
      return false; // arg type differs
  }
  return true;
}

struct Method *addMethod(enum MethodFlags flag, ClassType cls, ClassType returnType, const char *name, struct ArgType arguments) {
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
    if (isSameSignature(m->args, arguments)) {
      semanticError("");
      showMethodFlag(flag);
      printf(" " BOLD_TEXT "%s" NORMAL_TEXT, name);
      showSignature(arguments);
      printf(" is already defined in class %s\n", cls->name);
      success = false;
      break;
    }
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
    m->id = ID_UNASSIGNED;
    m->linenum = linenum;
    ArrayList_add(arr, m);
    cls->methodCount++;
    return m;
  }
  else {
    destroyArgType(arguments);
  }
  return NULL;
}

struct Method *addConstructor(enum MethodFlags flag, ClassType cls, struct ArgType arguments) {
  return addMethod(flag | Method_CONSTRUCTOR, cls, VoidClass, "<init>", arguments);
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

void showClassInterfaces(struct Class *cls) {
  printf("class %s : %s {\n", cls->name, cls->base == NULL ? "<null>" : cls->base->name);
  int i;
  for (i = 0; i < cls->fieldCount; i++) {
    struct Field *f = cls->fieldTable[i];
    printf("  %s %s; // from %s, id=%d\n", f->type->name, f->name, f->thisClass->name, f->id);
  }
  for (i = 0; i < cls->methodCount; i++) {
    struct Method *m = cls->methodTable[i];
    printf("  %s %s", m->returnType->name, m->name);
    showSignature(m->args);
    printf("; // from %s, id=%d\n", m->thisClass->name, m->id);
  }
  puts("}");
}

static int dfsSubClasses(ClassType base, int id) {
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
    cls->id = ID_UNASSIGNED;
    cls->maxId = ID_UNASSIGNED;
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
    else if (cls != VoidClass) {
      ArrayList_add(VoidClass->subclasses, cls);
    }
    MyHash_next(&it);
  }
  // really give class id
  classCount = dfsSubClasses(VoidClass, 0);

  // find classes without id
  MyHash_iterate(&allClasses, &it);
  for (i = 0; i < n; i++) {
    ClassType cls = it.it->value;
    if (cls->id == ID_UNASSIGNED && cls->defined && cls->base->defined) {
      linenum = cls->linenum;
      semanticError("class %s has circular inheritance\n", cls->name);
    }
    MyHash_next(&it);
  }
}

static void inheritFields(struct Class *cls) {
  struct Class *base = cls->base;
  int i;
  // add my fields
  int size = base->fieldCount + cls->fields._size;
  cls->fieldTable = malloc(sizeof(struct Field *) * size);
  struct MyHashIterator it;
  MyHash_iterate(&cls->fields, &it);
  for (i = base->fieldCount; i < size; i++) {
    struct Field *f = it.it->value;
    cls->fieldTable[i] = f;
    f->id = i;
    MyHash_next(&it);
  }
  // add inherited fields
  for (i = 0; i < base->fieldCount; i++) {
    struct Field *f = base->fieldTable[i];
    cls->fieldTable[i] = f;
    if (MyHash_get(&cls->fields, f->name) == NULL) {
      MyHash_set(&cls->fields, f->name, f);
      f->refcount++;
    }
  }
  cls->fieldCount = size;
}

static void inheritMethods(struct Class *cls) {
  struct Class *base = cls->base;
  int i;
  struct MyHashIterator it;
  // add inherited methods
  if (base != NULL) {
    MyHash_iterate(&base->methods, &it);
    while (it.it != NULL) {
      char *name = it.it->key;
      struct ArrayList *marr = it.it->value;
      struct ArrayList *mymarr = MyHash_get(&cls->methods, name);
      if (mymarr == NULL) {
        mymarr = malloc(sizeof(struct ArrayList));
        ArrayList_init(mymarr);
        MyHash_set(&cls->methods, dupstr(name), mymarr);
      }
      for (i = 0; i < marr->size; i++) {
        struct Method *m = ArrayList_get(marr, i), *mym;
        size_t j, n = mymarr->size;
        for (j = 0; j < n; j++) {
          mym = ArrayList_get(mymarr, j);
          if (isSameSignature(m->args, mym->args))
            break;
        }
        if (j < n) { // overrided
          mym->id = m->id;
          if (!isKindOf(mym->returnType, m->returnType)) {
            linenum = mym->linenum;
            semanticError("method ");
            printf("%s", name);
            showSignature(mym->args);
            printf(" in %s cannot override method ", cls->name);
            printf("in %s because return type is not compatible\n", base->name);
          }
        }
        else { // not overrided
          cls->methodCount++;
          m->refcount++;
          ArrayList_add(mymarr, m);
        }
      }
      MyHash_next(&it);
    }
  }
  int id = 0;
  if (base != NULL) id = base->methodCount; // first own method id
  cls->methodTable = malloc(sizeof(struct Method*) * cls->methodCount);
  // process my own method
  MyHash_iterate(&cls->methods, &it);
  while (it.it != NULL) {
    char *name = it.it->key;
    struct ArrayList *arr = it.it->value;
    for (i = 0; i < arr->size; i++) {
      struct Method *m = ArrayList_get(arr, i);
      if (m->id == ID_UNASSIGNED) {
        m->id = id++;
      }
      cls->methodTable[m->id] = m;
    }
    MyHash_next(&it);
  }
}

static void dfsProcessInheritance(struct Class *cls) {
  if (cls != VoidClass) inheritFields(cls);
  inheritMethods(cls);
  showClassInterfaces(cls);
  size_t n = cls->subclasses->size, i;
  for (i = 0; i < n; i++) {
    ClassType sub = ArrayList_get(cls->subclasses, i);
    dfsProcessInheritance(sub);
  }
}

void processInheritance(void) {
  dfsProcessInheritance(VoidClass);
}
