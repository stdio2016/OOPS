%{
#include <string.h>
#include <ctype.h>
#include "ast.h"
#include "y.tab.h"
#include "errReport.h"
#include "MyHash.h"
int linenum = 1;
struct MyHash strLitHash;
struct SizedString *strLitTable;
%}

%x C_COMMENT
%option noyywrap
%option noinput

newline  \r\n|[\r\n]
bom \xEF\xBB\xBF
%%
class { return CLASS; }
return { return RETURN; }
new { return NEW; }
this { return THIS; }
super { return SUPER; }
null { return NUL; }

[(),.:;={}] { return yytext[0]; }

[A-Za-z_$][A-Za-z_$0-9]* {
  yylval.str = dupstr(yytext);
  return IDENTIFIER;
}

\"(\\.|[^\\"\r\n])*\" {
  char *str = malloc(yyleng - 1);
  int i, j, oct, pos;
  for (i = 1, j = 0; i < yyleng-1; i++, j++) {
    if (yytext[i] == '\\') {
      i++;
      switch (yytext[i]) {
        case 'a': str[j] = '\a'; break;
        case 'b': str[j] = '\b'; break;
        case 'e': str[j] = '\x1B'; break;
        case 'f': str[j] = '\f'; break;
        case 'n': str[j] = '\n'; break;
        case 'r': str[j] = '\r'; break;
        case 't': str[j] = '\t'; break;
        case 'v': str[j] = '\v'; break;
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
          oct = 'o', pos = 0;
          sscanf(&yytext[i], "%3o%n", &oct, &pos);
          if (pos > 0) i += pos-1;
          str[j] = oct;
          break;
        case 'x':
          oct = 'x', pos = 0;
          sscanf(&yytext[i+1], "%2x%n", &oct, &pos);
          if (pos > 0) i += pos;
          str[j] = oct;
          break;
        default: str[j] = yytext[i]; break;
      }
    }
    else {
      str[j] = yytext[i];
    }
  }
  str[j] = '\0';
  str = realloc(str, j+1);
  struct SizedString *ss = malloc(sizeof(struct SizedString));
  ss->len = j;
  ss->str = str;
  struct SizedString *g = MyHash_get(&strLitHash, ss);
  if (g == NULL) {
    yylval.strId = ss->id = strLitHash._size;
    MyHash_set(&strLitHash, ss, ss);
  }
  else {
    yylval.strId = g->id;
    free(ss);
    free(str);
  }
  return STRING;
}
\"(\\.|[^\\"\r\n])*({newline}) {
  syntaxError("unterminated string literal\n");
  return ERROR;
}

"//".*   ;

"/*" { BEGIN (C_COMMENT); }
<C_COMMENT>. ;
<C_COMMENT>"*/" { BEGIN(INITIAL); }
<C_COMMENT><<EOF>> {
  syntaxError("missing end of /* ... */ comment\n");
  return ERROR;
}

<*>{newline} {
  linenum++;
}

[ \t]+ ;
{bom}  { /* skip */ }
. { return ERROR; }
%%

static size_t strLitHashFn(const void *ss) {
  const struct SizedString *s = ss;
  const char *str = s->str;
  size_t n = s->len, i;
  size_t hash = 88487;
  for (i = 0; i < n; i++) {
    hash = (hash << 5) + hash + (size_t) str[i];
  }
  return hash;
}

static int strLitCmp(const void *s1, const void *s2) {
  const struct SizedString *a1 = s1, *a2 = s2;
  size_t i;
  size_t m = a1->len;
  if (a2->len < m) m = a2->len;
  for (i = 0; i < m && a1->str[i] == a2->str[i]; i++) {
    ;
  }
  if (i == m) {
    if (a1->len > a2->len) return 1;
    if (a1->len < a2->len) return -1;
    return 0;
  }
  return a1->str[i] - a2->str[i];
}

void initStrLitTable(void) {
  MyHash_init(&strLitHash, strLitCmp, strLitHashFn);
}

static void destroyStrLit(struct HashBucket *b) {
  free(((struct SizedString *)b->key)->str);
  free(b->key);
  free(b);
}

void destroyStrLitTable(void) {
  MyHash_destroy(&strLitHash, destroyStrLit);
}
