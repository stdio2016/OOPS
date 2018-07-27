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

// 323 * 8B to run 99bottles.txt
// 8656 * 8B to run quine.txt

  // run program!
  if (n == 0) {
    struct VM_State vm;
    vm.stack = malloc(sizeof(union VM_StackType) * 16000);
    vm.stackLimit = vm.stack + 16000;
    vm.heap = malloc(sizeof(union VM_Object) * 16000);
    vm.heapLimit = vm.heap + 16000;
    int r = startProgram(&vm);
    if (r != VM_RunResult_Finish) {
      fflush(stdout);
      switch (r) {
        case VM_RunResult_OOM:
          fprintf(stderr, "Out of memory\n"); break;
        case VM_RunResult_StackOverflow:
          fprintf(stderr, "Stack overflow\n"); break;
        case VM_RunResult_Interrupt:
          fprintf(stderr, "Interrupt\n"); break;
        case VM_RunResult_InternalError:
          fprintf(stderr, "Internal error\n"); break;
        case VM_RunResult_NoEntryPoint:
          fprintf(stderr, "constructor main() not found\n"); break;
        case VM_RunResult_NoMainClass:
          fprintf(stderr, "entry class \"main\" not found\n"); break;
      }
      stackTrace(&vm);
    }
    free(vm.stack);
    free(vm.heap);
  }

  destroyClassTable();
  destroyStrLitTable();
  return n;
}
