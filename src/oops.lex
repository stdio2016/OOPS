%{
#include "y.tab.h"
#include "errReport.h"
int linenum = 1;
%}

%x C_COMMENT
%option noyywrap
%option noinput

newline  \r\n|[\r\n]
%%
class { return CLASS; }
return { return RETURN; }
new { return NEW; }
this { return THIS; }
super { return SUPER; }

[(),.:;={}] { return yytext[0]; }

[A-Za-z_$][A-Za-z_$0-9]* { return IDENTIFIER; }

\"(\\.|[^\\"\r\n])*\" { return STRING; }
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

[ \t] ;
. { return ERROR; }
%%
