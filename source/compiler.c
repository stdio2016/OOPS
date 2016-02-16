#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "seriousError.h"
#include "compiler.h"
#define error(...)  (errorOccured=1,setColor(0x47),fprintf(stderr,__VA_ARGS__),setColor(0x7))
#define error2nd(...)  (errorOccured2nd=1,setColor(0x47),fprintf(stderr,__VA_ARGS__),setColor(0x7))

int errorOccured2nd=0;
int errorOccured=0;
char *copystr(char *str)
{
    char *cpy;
    cpy=malloc((strlen(str)+1)*sizeof(char));
    strcpy(cpy,str);
    return cpy;
}

struct parseNode *newNode(int type)
{
    struct parseNode *n;
    n=malloc(sizeof*n);
    n->type=type;
    if(type & 1)
        n->dat.n=NULL;
    else
        n->dat.s=NULL;
    n->next=NULL;
    return n;
}

void freeAll(struct parseNode *p)
{
    if(p!=NULL)
    {
        freeAll(p->next);
        if(p->type & 1)
            freeAll(p->dat.n);
        else
            free(p->dat.s);
        free(p);
    }
}
static int state;

static struct parseNode *repairPT(struct parseNode *node)
{
    struct parseNode *a=node,*prev=NULL,*b,*c;
    ///  name
    ///  ( expr )
    ///  ( type ) { object literal }
    ///  (digit1) { _nex = (digit){} }
    while(a!=NULL)
    {
        if(a->type==13) // ( ... )
        {
            a->dat.n=repairPT(a->dat.n);
            b=a->dat.n;
            if(b!=NULL && b->next==NULL && b->type==0 && isNameChar(b->dat.s[0]))
            if(a->next!=NULL)
            {
                b=a->next;
                if(b->type==17)
                {
                    b->dat.n=repairPT(b->dat.n);
                    a->dat.n->next=b;
                    a->next=b->next;
                    b->next=NULL;
                }
            }
        }
        a=a->next;
    }
    ///   expr . name
    ///   expr ( expr )

    a=node; prev=NULL;
    while(a!=NULL)
    {
        int notOp=(a->type!=0 || isNameChar(a->dat.s[0]));
        b=a->next;
        if(b==NULL)
            break;
        if(notOp && b->type==0 && strcmp(b->dat.s,".")==0)
        {
            // expr . name
            c=b->next;
            if(c==NULL || c->type!=0 || !isNameChar(c->dat.s[0])
               || isdigit(c->dat.s[0]))
            {
                error2nd("Error! expected identifier after . at %d\n",
                      b->lineno);
            }
            free(b->dat.s);
            b->type=23;
            b->dat.n=a;
            if(c==NULL) b->next=NULL;
            else b->next=c->next;

            if(prev==NULL) node=b;
            else prev->next=b;

            a->next=c;
            if(c!=NULL)
                c->next=NULL;
            a=b;
        }
        else if(notOp && b->type==13)
        {
            // expr ( expr )
            b->type=25;
            c=b->dat.n;
            b->dat.n=a;
            a->next=c;
            if(prev==NULL) node=b;
            else prev->next=b;
            a=b;
        }
        else
        {
            prev=a;
            a=a->next;
        }
    }
    /// new    expr
    a=prev=node; c=NULL;
    while(a!=NULL)
    {
        if(c==NULL)
            b=a->next;
        else
            b=c;
        if(b==NULL)
            break;
        if(a->type==0 && strcmp(a->dat.s,"new")==0)
        {
            // new expr
            free(a->dat.s);
            a->type=27;
            a->dat.n=b;
            prev->next=c=b->next;
            a=b;
            b->next=NULL;
        }
        /*else if(a->type==13 && a->dat.n!=NULL
                && a->dat.n->type==0 && a->dat.n->next==NULL)
        {
            // (type) expr
            a->type=29;
            a->dat.n->next=b;
            prev->next=c=b->next;
            a=b;
            b->next=NULL;
        }*/
        else
        {
            c=NULL;
            a=b;
            prev=a;
        }
    }

    /// expr = expr
    struct parseNode e,*next=node,*f=NULL;
    do
    {
        a=next;b=NULL;
        prev=&e;
        e.next=NULL;
        while(a!=NULL)
        {
            b=a->next;
            if(b==NULL)
            {
                prev->next=a;
                prev=a;
                next=NULL;
                break;
            }
            if(b->type==0 && strcmp(b->dat.s,"=")==0)
            {
                prev->next=b;
                free(b->dat.s);
                b->dat.n=a;
                b->type=31;
                a->next=NULL;
                prev=a;
            }
            else
            {
                prev->next=a;
                prev=a;
                next=b;
                a->next=NULL;
                break;
            }
            a=b->next;
            b->next=NULL;
        }
        if(a==NULL && b!=NULL)
        {
            error2nd("Error! Missing expression after = at %d\n",
                     b->lineno);
            next=NULL;
        }
        prev->next=NULL;
        if(f==NULL) node=e.next;
        else f->next=e.next;
        f=e.next;
    } while(next!=NULL);
    if(f!=NULL)
    f->next=NULL;

    /// expr , expr
    a=node; prev=NULL;
    while(a!=NULL)
    {
        b=a->next;
        if(b==NULL)
            break;
        if(b->type==0 && strcmp(b->dat.s,",")==0)
        {
            c=b->next;
            free(b->dat.s);
            b->type=33;
            b->dat.n=a;
            if(c==NULL)
            {
                b->next=NULL;
            }
            else b->next=c->next;

            if(prev==NULL) node=b;
            else prev->next=b;

            a->next=c;
            if(c!=NULL)
                c->next=NULL;
            a=b;
        }
        else
        {
            prev=a;
            a=a->next;
        }
    }
    return node;
}

static struct parseNode *repairPTStmt(struct parseNode *pn)
{
    int yeah=1;
    struct parseNode *ptr=pn,*newPtr=pn,*prev=NULL,*prevNewPtr=NULL;
    while(ptr!=NULL) // semicolon ; separated nodes
    {
        if(ptr->type==0 && ptr->dat.s[0]==';')
        {
            ptr->type=19; // statement
            free(ptr->dat.s);
            if(newPtr==ptr)
            {
                ptr->dat.n=NULL;
            }
            else
            {
                ptr->dat.n=newPtr;
                if(prev!=NULL)
                    prev->next=NULL;
            }
            newPtr=ptr->next;
            if(prevNewPtr==NULL)
                pn=ptr;
            else
                prevNewPtr->next=ptr;
            prevNewPtr=ptr;
            yeah=1;
        }
        else
            yeah=0;
        prev=ptr;
        ptr=ptr->next;
    }
    if(!yeah)
    {
        error2nd("Error! Missing semicolon ; at %d\n",prev->lineno);
        return pn;
    }
    ptr=pn;
    while(ptr!=NULL)
    {
        newPtr=ptr->dat.n;
        if(newPtr==NULL);
        else if(!(newPtr->type & 1) && isNameChar(newPtr->dat.s[0]))
        {
            if(strcmp(newPtr->dat.s,"return")==0)
            {
                // return statement
                ptr->type=21;
                ptr->dat.n=repairPT(newPtr->next);
                newPtr->next=NULL;
                freeAll(newPtr);
            }
            else
            {
                struct parseNode *q=newPtr->next;
                if(q!=NULL && !(q->type & 1) && isNameChar(q->dat.s[0]))
                {
                    // variable declaration
                    ptr->type=5;
                    newPtr->next=repairPT(newPtr->next);
                }
                else
                {
                    // normal statement
                    ptr->dat.n=repairPT(ptr->dat.n);
                }
            }
        }
        else
        {
            // normal statement
            ptr->dat.n=repairPT(ptr->dat.n);
        }
        ptr=ptr->next;
    }
    return pn;
}

static struct parseNode *getParseTreeBlock(struct parserState *ps,int term)
{
    static int termTable[3]={')',']','}'};
    static int typeT[3]={13,15,17};
    int y=getToken(ps);
    struct parseNode *result,*next,*ptr;
    result=newNode(typeT[term]);
    result->lineno=ps->lineno;
    ptr=NULL;
    while(y!=NoMoreToken && ps->token[0]!='}'
          && ps->token[0]!=']' && ps->token[0]!=')')
    {
        char tt=ps->token[0];
        if(tt=='(') next=getParseTreeBlock(ps,0);
        else if(tt=='[') next=getParseTreeBlock(ps,1);
        else if(tt=='{') next=getParseTreeBlock(ps,2);
        else
        {
            next=newNode(0);
            next->lineno=ps->lineno;
            next->dat.s=copystr(ps->token);
        }
        if(ptr==NULL)
        {
            ptr=next;
            result->dat.n=next;
        }
        else
        {
            ptr->next=next;
            ptr=next;
        }

        if(errorOccured)
            return result;
        y=getToken(ps);
    }
    if(y==NoMoreToken)
    {
        error("Error! Missing right brace } at %d\n",ps->lineno);
    }
    else if(ps->token[0]!=termTable[term])
        error("Error! Missing right brace } at %d, found %s\n"
              ,ps->lineno,ps->token);
    return result;
}

static struct parseNode *getParseTreeMethod(struct parserState *ps)
{
    int y=getToken(ps);
    struct parseNode *result,*next,*ptr,*type,*arg;
    result=newNode(11);
    result->lineno=ps->lineno;
    if(y==NameToken)
    {
        next=newNode(5);
        next->lineno=ps->lineno;
        result->dat.n=next;
        ptr=next;
        type=newNode(0);
        type->lineno=ps->lineno;
        type->dat.s=copystr(ps->token);
        next->dat.n=type;

        y=getToken(ps);
        if(y==NameToken)
        {
            arg=newNode(0);
            arg->lineno=ps->lineno;
            arg->dat.s=copystr(ps->token);
            type->next=arg;
            y=getToken(ps);
        }
        else
        {
            error("Error! Missing type name or argument type at %d, found %s\n",
                  ps->lineno,ps->token);
            return result;
        }

        while(y!=NoMoreToken && ps->token[0]!=')')
        {
            if(ps->token[0]!=',')
            {
                error("Error! Expected ',' or ';' before %s at %d\n",
                      ps->token,ps->lineno);
                return result;
            }
            y=getToken(ps);
            if(y!=NameToken)
            {
                error("Error! Missing type name or argument type at %d, found %s\n",
                      ps->lineno,ps->token);
                return result;
            }
            next=newNode(5);
            next->lineno=ps->lineno;
            ptr->next=next;
            ptr=next;
            type=newNode(0);
            type->lineno=ps->lineno;
            type->dat.s=copystr(ps->token);
            next->dat.n=type;

            y=getToken(ps);
            if(y==NameToken)
            {
                arg=newNode(0);
                arg->lineno=ps->lineno;
                arg->dat.s=copystr(ps->token);
                type->next=arg;
            }
            else
            {
                error("Error! Missing type name or argument type at %d, found %s\n",
                      ps->lineno,ps->token);
                return result;
            }
            y=getToken(ps);
        }
    }
    y=getToken(ps);
    if(ps->token[0]!='{')
    {
        error("Error! Left bracket { expected at %d, found %s\n"
              ,ps->lineno,ps->token);
        return result;
    }
    result->next=getParseTreeBlock(ps,2);
    result->next->dat.n=repairPTStmt(result->next->dat.n);
    state=getToken(ps);
    return result;
}

static struct parseNode *getParseTreeVar(struct parserState *ps)
{
    struct parseNode *result=NULL,*next,*ptr;
    int y;
    char *name;
    if(ps->token[0]=='}' || ps->token[0]==0)
        return NULL;
    name=copystr(ps->token);
    result=newNode(9);
    result->lineno=ps->lineno;

    next=newNode(0);
    next->lineno=ps->lineno;
    next->dat.s=name;
    result->dat.n=next;
    ptr=next;
    y=getToken(ps);
    if(y!=NameToken && ps->token[0]!='(')
    {
        state=y;
        error("Error! Method or variable name expected at %d, found %s\n"
              ,ps->lineno,ps->token);
        return result;
    }
    if(y==NameToken)
    {
        result->type=7;
        name=copystr(ps->token);
        next=newNode(0);
        next->lineno=ps->lineno;
        next->dat.s=name;
        ptr->next=next;
        ptr=next;
        y=getToken(ps);
    }
    if(ps->token[0]=='(')
    {
        ptr->next=getParseTreeMethod(ps);
    }
    else
    {
        result->type=5;
        while(ps->token[0]==',')
        {
            y=getToken(ps);
            if(y!=NameToken)
            {
                error("Error! Variable name expected at %d, found %s\n"
                      ,ps->lineno,ps->token);
                return result;
            }
            name=copystr(ps->token);
            next=newNode(0);
            next->lineno=ps->lineno;
            next->dat.s=name;
            ptr->next=next;
            ptr=next;
            y=getToken(ps);
        }
        if(ps->token[0]!=';')
        {
            error("Error! Semicolon ; expected at %d\n",ps->lineno);
            return result;
        }
        y=getToken(ps);
        state=y;
    }
    return result;
}

int the_variable_name_is_so_long_that_i_dont_want_to_see_it_but_without_this_variable_the_program_wont_work;
void themethodnameissolongthatidontwanttoseeitbutwithoutthismethodtheprogramwontwork()
{
    puts("the variable name is so long that I don't want to see it but without this variable the program won't work\n");
}

static struct parseNode *getParseTreeClass(struct parserState *ps)
{
    int y;
    char *name,*baseName=NULL;
    y=getToken(ps);
    if(y==NoMoreToken)
        return NULL;
    if(strcmp(ps->token,"class")!=0)
    {
        error("Error! OOPS only supports classes. Unexpected token %s at %d\n",
              ps->token,ps->lineno);
        return NULL;
    }
    struct parseNode *result,*next,*ptr,*cc;
    result=newNode(1);
    result->lineno=ps->lineno;
    y=getToken(ps);
    if(y==NoMoreToken || y!=NameToken)
    {
        error("Error! Class name expected at line %d\n",ps->lineno);
        return NULL;
    }
    name=copystr(ps->token);

    next=newNode(3);
    next->lineno=ps->lineno;

    cc=newNode(0);
    next->dat.n=cc;
    cc->lineno=ps->lineno;
    cc->dat.s=name;

    result->dat.n=next;
    ptr=next;
    y=getToken(ps);
    if(y!=NoMoreToken && strcmp(ps->token,":")==0)
    {
        y=getToken(ps);
        if(y!=NameToken)
        {
            error("Error! Base class name expected at line %d\n",ps->lineno);
            return result;
        }
        baseName=copystr(ps->token);

        cc=newNode(0);
        cc->lineno=ps->lineno;
        cc->dat.s=baseName;
        next->dat.n->next=cc;

        y=getToken(ps);
    }
    if(y!=NoMoreToken && strcmp(ps->token,"{")==0)
    {
        y=getToken(ps);
        if(y!=NameToken && strcmp(ps->token,"}")!=0)
        {
            error("Error! Method or variable definition expected at %d\n",ps->lineno);
            return result;
        }
        next=getParseTreeVar(ps);
        state=NameToken;
        while(next!=NULL)
        {
            ptr->next=next;
            ptr=next;
            ptr->next=NULL;
            if(errorOccured)
            {
                return result;
            }
            next=getParseTreeVar(ps);
        }

        if(strcmp(ps->token,"}")!=0)
        {
            if(state==NoMoreToken)
                error("Error! Missing right brace } at line %d\n",ps->lineno);
            else if(state!=NameToken)
                error("Error! Method or variable definition expected at %d\n",ps->lineno);
        }
    }
    else
    {
        error("Error! Left brace { expected at line %d\n",ps->lineno);
        return result;
    }
    return result;
}

struct parseNode *getParseTree(struct parserState *ps)
{
    struct parseNode *result,*next,*ptr;
    result=next=ptr=getParseTreeClass(ps);
    if(next!=NULL)
    {
        next=getParseTreeClass(ps);
        while(next!=NULL)
        {
            ptr->next=next;
            ptr=next;
            ptr->next=NULL;
            if(errorOccured)
            {
                return result;
            }
            next=getParseTreeClass(ps);
        }

    }

    return result;
}

void showNode(struct parseNode *pn,int indent)
{
    int i;
    if(pn==NULL)
    {
        for(i=0;i<indent;i++)
            printf("  ");
        puts("<empty>");
    }
    while(pn!=NULL)
    {
        for(i=0;i<indent;i++)
            printf("  ");
        if(pn->type & 1)
        {
            switch(pn->type)
            {
                case 1: puts("class"); break;
                case 3: puts("name and base class"); break;
                case 5: puts("variable"); break;
                case 7: puts("method"); break;
                case 9: puts("constructor"); break;
                case 11: puts("arguments"); break;
                case 13: puts("( ... )"); break;
                case 15: puts("[ ... ]"); break;
                case 17: puts("{ ... }"); break;
                case 19: puts("statement"); break;
                case 21: puts("return"); break;
                case 23: puts("."); break;
                case 25: puts("call"); break;
                case 27: puts("new"); break;
                case 29: puts("type cast"); break;
                case 31: puts("="); break;
                case 33: puts(","); break;
                default: printf("node type %d\n",pn->type); break;
            }
            showNode(pn->dat.n,indent+1);
        }
        else
        {
            puts(pn->dat.s);
        }
        pn=pn->next;
    }
}
