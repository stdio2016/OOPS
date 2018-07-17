#include <stdio.h>
#include <stdlib.h>
#include "symtable.h"
#include "class.h"
#include "builtin.h"
#include "codegen.h"
#include "vm.h"
extern FILE* yyin;
extern int yyparse();
extern void yylex_destroy();

extern void initStrLitTable(void);
extern void destroyStrLitTable(void);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <file>\n", argv[0]);
    return 1;
  }
  FILE *f = fopen(argv[1], "r");
  if (f == NULL) {
    printf("Cannot open file \"%s\"\n", argv[1]);
    return 1;
  }
  yyin = f;

  // parse OOPS code
  // semantic check 1: redeclared variables/classes/methods
  initSymTable();
  initClassTable();
  initStrLitTable();
  addBuiltinMethods();
  int n = yyparse();
  if (n == 0) {
#ifdef DEBUG
    printf("There is no syntax error! :-)\n");
#endif
  }
  else {
    printf("There is syntax error ;-(\n");
  }

  fclose(f);
  yylex_destroy();
  destroySymTable();

  extern int errorCount; // defined in errReport.h
  // semantic check 2: cyclic class inheritance and undefined classes
  if (n == 0) { // no syntax error
    // give each class an id
    giveClassId();
    n = errorCount;
  }

  // add inherited methods and fields
  // semantic check 3: overriden method should have compatible return type
  if (n == 0) { // no semantic error
    processInheritance();
    n = errorCount;
  }

  // generate bytecode
  // semantic check 4: function overloading and type check
  if (n == 0) {
    compileAllClasses();
    n = errorCount;
  }

  // run program!
  if (n == 0) {
    struct VM_State vm;
    vm.stack = malloc(sizeof(union VM_StackType) * 16000);
    vm.stackLimit = vm.stack + 16000;
    vm.heap = malloc(sizeof(union VM_Object) * 16000);
    vm.heapLimit = vm.heap + 16000;
    startProgram(&vm);
    free(vm.stack);
    free(vm.heap);
  }

  destroyClassTable();
  destroyStrLitTable();
  return n;
}
