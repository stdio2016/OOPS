#include <stdio.h>

int a[1000] = {1}, b[1000];
int lena, lenb;

int add(int *a, int *b, int lena, int lenb) {
  int i;
  for (i = 0; i < lena; i++) {
    a[i] += b[i];
    if (a[i] >= 100000000) {
      a[i+1]++;
      a[i] -= 100000000;
    }
  }
  for (i = i; i < lenb; i++) {
    a[i] += b[i];
    if (a[i] >= 100000000) {
      a[i+1]++;
      a[i] -= 100000000;
    }
  }
  if (a[i] > 0) {
    return i+1;
  }
  return i;
}

int main(void)
{
  int i = 0;
  puts("Press enter to get next Fibonacci number");
  while (i < 8492) {
    if (i&1) lenb = add(b, a, lenb, lena);
    else lena = add(a, b, lena, lenb);
    i++;
    printf("F[%d] = ", i);
    int len = i&1 ? lena : lenb;
    int *n = i&1 ? a : b;
    int j;
    printf("%d", n[len-1]);
    for (j = len-2; j >= 0; j--) {
      printf("%08d", n[j]);
    }
    puts("");
  }
  return 0;
}
