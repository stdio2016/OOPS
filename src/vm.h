#pragma once
#ifndef VM_INCLUDED
#define VM_INCLUDED

enum VM_Bytecodes {
  Instr_NOP,
  Instr_POP,
  Instr_DUP,
  Instr_CALL,
  Instr_STR,
  Instr_CALLSPECIAL,
  Instr_NEW,
  Instr_THIS,
  Instr_NULL,
  Instr_LOAD,
  Instr_STORE,
  Instr_PUTFIELD,
  Instr_GETFIELD,
  Instr_CONVERT,
  Instr_RETURN
};

void showBytecode(unsigned char *code);

// stack structure
// 1. this
// 2. arguments
// 3. locals <- fp
// 4. stack pointer, frame pointer, return address
// 5. stack <- sp

union VM_Object {
  int classId;
  union VM_Object *field;
};

union VM_StackType {
  union VM_Object *obj;
  unsigned char *ip;
  union VM_StackType *sp;
};

struct VM_State {
  union VM_StackType *sp, *fp;
  union VM_StackType *stack, *stackLimit;
  union VM_Object *heap, *heapLimit;
  union VM_Object *heapUsed;
};

void runProgram(struct VM_State *state);
#endif
