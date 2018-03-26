#include <stdio.h>

int main(void)
{
  int ch;
  char tb[16] = "pabcdefghijklmno";
  while ((ch = getchar()) != EOF) {
    if (ch == '\n') puts("n();");
    else if (ch == ' ') printf("s();");
    else {
      printf("w(%c,%c);", tb[ch>>4 & 15], tb[ch & 15]);
    }
  }
  return 0;
}
