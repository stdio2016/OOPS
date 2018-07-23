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

enum VM_RunResult {
  VM_RunResult_Finish,
  VM_RunResult_OOM,
  VM_RunResult_StackOverflow,
  VM_RunResult_Interrupt,
  VM_RunResult_InternalError
};

void showBytecode(unsigned char *code);

// stack structure
// 1. this
// 2. arguments <- fp
// 3. locals
// 4. frame pointer 2, frame pointer, return address <- fp2
// 5. stack <- sp

union VM_Object {
  int classId;
  union VM_Object *field;
};

// flags for classId
#define VM_STRING_LIT 0x40000
#define VM_MARKED 0x10000
#define VM_CLASS_MASK 0xFFFF

union VM_StackType {
  union VM_Object *obj;
  unsigned char *ip;
  union VM_StackType *sp;
};

struct VM_State {
  union VM_StackType *sp, *fp, *fp2;
  union VM_StackType *stack, *stackLimit;
  union VM_Object *heap, *heapLimit;
  union VM_Object *heapUsed;
  unsigned char *pc;
};

typedef union VM_StackType vm_stack_t;
typedef struct VM_State vm_state;
typedef union VM_Object vm_object;

int startProgram(struct VM_State *state);
int runByteCode(struct VM_State *state);
#endif
