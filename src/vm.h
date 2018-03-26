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

#endif