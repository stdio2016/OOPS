/*
  OOPS grammer
  Use PEG.js to generate the parser
*/
OOPS
  = _ cs:(c:class _ {return c;})+ {return cs;}

class
  = "class" _ n:type _ ii:(":" _ i:type _ {return i;})? b:classbody {
    return [n,ii||"void",b];
  }

classbody
  = "{" _ b:(
    fm:(
      &(type _ methodName _ "(") m:method {return m;}
      / &(type _ "(") m:constructor {return m;}
      / f:field {return f;}
    ) _ {return fm;}
  )* "}" {return b;}

field
  = t:type _ v1:varName _ 
    vn:("," _ v2:varName _ {return v2;})*
    ";" {return ["field",t,[v1].concat(vn)];}

method
  = t:type _ m:methodName _ a:argDeclList _ b:methodBody
  { return ["method",t,m,a,b];
  }

constructor
  = t:type _ a:argDeclList _ b:methodBody
  { return ["constructor",t,a,b];
  }

argDeclList = "("
  a:(
    _ b:argDecl _
    c:(
      "," _ d:argDecl _ {return d;}
    )* 
    {return c===null ? [b] : [b].concat(c);}
  )?
  ")" {return a===null ? [] : a;}

argDecl = t:type _ n:varName {return [t, n];}

methodBody = "{" _
  rve:( r:return _ {return r;}
  / v:var _ {return v;}
  / e:cexpr _ ";" _ {return e;}
  )*
  "}" {return rve;}

return = "return" _ e:cexpr? _ ";" {return ["return", e]}

var
  = t:type _ v1:varName _ i1:init? _
    vn:("," _ v2:varName _ i2:init? {return [v2,i2];})*
    ";" {return ["var",t,[[v1,i1]].concat(vn)];}

init
  = "=" _ e:expr {return e;}

cexpr
  = e1:expr _ en:("," _ en:expr _ {return en;})*
  {return en.length ? [",", e1].concat(en) : e1;}

expr
  = v:varName _ "=" _ e:expr {return ["=", v, e];}
  / e:newExpr {return e;}

newExpr
  = "new" _ a:type _ b:args {return ["new",a,b];}
  / a:call {return a;}

call
  = r:selfCall c:(_ "." _ m:methodName _ a:args {return [m,a]})*
  {return c.length ? ["call",r,c] : r;}

selfCall
  = m:methodName _ a:args {return ["call","this",[m,a]];}
  / a:atom {return a;}

atom
  = v:varName {return v;}
  / "(" _ e:cexpr ")" {return e;}
  / s:string { return s; }

string = "\"" ([^"] / "\\" .)* "\"" {return text();}

args = "(" _
  a:(
    b:expr _
    c:(
      "," _ d:expr _ {return d;}
    )* 
    {return c===null ? [b] : [b].concat(c);}
  )?
  ")" {return a===null ? [] : a;}

type = name
varName = name
methodName = name

name
  =
  nameFirstChar nameChar* {return text();}

nameFirstChar = [A-Za-z_$]
nameChar = nameFirstChar / [0-9]

_ "whitespace"
  = ([ \t\n\r]
  / "//" [^\n]* "\n"
  / "/*" ([^*] / "*" [^/])* "*/")*
