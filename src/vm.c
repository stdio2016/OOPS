#include <stdio.h>
#include "vm.h"

void showBytecode(unsigned char *code) {
  while (*code != Instr_RETURN) {
    printf(" %.2X", *code);
    code++;
  }
  puts(" 0E");
}
