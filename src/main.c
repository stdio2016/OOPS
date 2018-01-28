#include <stdio.h>
extern FILE* yyin;
extern int yyparse();
extern void yylex_destroy();

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

  int n = yyparse();
  if (n == 0) {
    printf("There is no syntax error! :-)\n");
  }
  else {
    printf("There is syntax error ;-(\n");
  }
  fclose(f);
  yylex_destroy();
  return n;
}
