%{
#include "y.tab.h"
%}

%x C_COMMENT
%option noyywrap
%option noinput

%%
class { return CLASS; }
return { return RETURN; }
new { return NEW; }

[A-Za-z_$][A-Za-z_$0-9]* { return IDENTIFIER; }

\"(\\.|[^\\"])*\" { return STRING; }

"//".*   ;

"/*" { BEGIN (C_COMMENT); }
<C_COMMENT>(.|\n) ;
<C_COMMENT>"*/" { BEGIN(INITIAL); }

[ \t\r\n]+ ;
. { return (int) yytext[0]; }
%%
