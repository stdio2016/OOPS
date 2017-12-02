#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MyHash.h"

struct MyHash table;

char *genStr() {
  size_t r = rand() % 5 + 5;
  char *str = malloc(r+1);
  int i;
  for (i = 0; i < r; i++) {
    str[i] = rand() % 26 + 'a';
  }
  str[9] = '\0';
  return str;
}

char *dupStr(const char *str) {
  size_t n = strlen(str);
  char *nstr = malloc(n + 1);
  memcpy(nstr, str, n + 1);
  return nstr;
}

int delKey(char *key) {
  struct HashBucket *buck = MyHash_delete(&table, key);
  if (buck != NULL) {
    free(buck->key);
    free(buck->value);
    free(buck);
    return 1;
  }
  return 0;
}

int main(void)
{
  MyHash_init(&table, MyHash_strcmp, MyHash_strhash);
  int t;
  for (t = 0; t < 1000; t++) {
    char buf[122];
    snprintf(buf, 121, "%d", t);
    MyHash_set(&table, dupStr(buf), dupStr(buf));
  }
  delKey("21");
  struct MyHashIterator iter;
  MyHash_iterate(&table, &iter);
  while (iter.it != NULL) {
    printf("%s -> %s\n", (char*)iter.it->key, (char*)iter.it->value);
    MyHash_next(&iter);
  }
  for (t = 0; t < 1000; t++) {
    char buf[122];
    snprintf(buf, 121, "%d", t);
    char *get = MyHash_get(&table, buf);
    if (get == NULL) printf("NO! %d\n", t);
  }
  return 0;
}
