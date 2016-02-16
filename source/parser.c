#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "seriousError.h"

/*this
 is a
  comment */
static void pushBack(int ch,struct parserState *ps)
{
    if(ch=='\n')
    {
        ps->lineno--;
    }
    if(ps->pushTop < ps->pushedBackN)
    {

        ps->pushedBack[ps->pushTop++]=ch;
    }
    else
    {
        ps->pushedBackN*=2;
        ps->pushedBack=realloc(ps->pushedBack,ps->pushedBackN*sizeof(int));
        if(ps->pushedBack==NULL)
            seriousError("Out of memory!");
        ps->pushedBack[ps->pushTop++]=ch;
    }
}

static void tokenChar(int ch,struct parserState *ps)
{
    if(ps->tokenTop >= ps->tokenLen)
    {
        ps->tokenLen*=2;
        ps->token=realloc(ps->token,ps->tokenLen*sizeof(char));
        if(ps->token==NULL)
            seriousError("Out of memory!");
    }
    ps->token[ps->tokenTop++]=ch;
}

static int readCharHelper(struct parserState *ps)
{
    int ch;
    if(ps->pushTop > 0)
    {
        ch=ps->pushedBack[--ps->pushTop];
    }
    else
    {
        ch=fgetc(ps->f);
    }
    if(ch=='\n')
    {
        ps->lineno++;
    }
    return ch;
}

static int readChar(struct parserState *ps)
{
    int ch=readCharHelper(ps);
    int i=0;
    if(ch=='\\')
    {
        ch=readCharHelper(ps);
        i=ps->tokenTop;
        while(ch==' '||ch=='\t')
        {
            tokenChar(ch,ps);
            ch=readCharHelper(ps);
        }
        if(ch=='\n')
        {
            ch=readCharHelper(ps);
        }
        else
        {
            pushBack(ch,ps);
            for(; ps->tokenTop > i ; )
                pushBack(ps->token[--ps->tokenTop],ps);
            ch='\\';
        }
    }
    else if(ch=='\n')
        ps->equivLineno=ps->lineno;
    return ch;
}

struct parserState *parseFile(FILE *f,struct parserState *p)
{
    struct parserState *ps;
    if(p==NULL)
        ps=malloc(sizeof(struct parserState));
    else
        ps=p;

    if(ps==NULL)
        seriousError("Out of memory!");
    ps->f=f;
    ps->tokenLen=8;
    ps->token=malloc(ps->tokenLen * sizeof(char));
    ps->tokenTop=0;
    ps->pushedBackN=4;
    ps->pushedBack=malloc(ps->pushedBackN * sizeof(int));
    ps->pushTop=0;
    ps->lineno=1;
    ps->equivLineno=ps->lineno;
    return ps;
}

void stopParsing(struct parserState *ps)
{
    free(ps->token);
    free(ps->pushedBack);
    fclose(ps->f);
}

int isNameChar(int ch)
{
    return (ch>='0'&&ch<='9')
        || (ch>='A'&&ch<='Z')
        || (ch>='a'&&ch<='z')
        || ch=='_'
        || ch=='$'
        || ch>0x7F;
}

static void skipStarComment(struct parserState *ps)
{
    // skip /* ... */ comment
    int state=0,ch;
    while(state!=2)
    {
        ch=readChar(ps);
        if(ch=='*'&&state==0)
            state=1;
        else if(ch=='/'&&state==1)
            state=2;
        else if(ch==EOF)
            state=2;
        else
            state=0;
    }
}

static void skipSlashComment(struct parserState *ps)
{
    /*
        skip // ... comment
    */
    int ch;
    do
    {
        ch=readChar(ps);
    } while(ch!='\n'&&ch!=EOF);
}

static int readString(struct parserState *ps,char ch)
{
    int y;
    tokenChar(ch,ps);
    y=readChar(ps);
    while(y!=ch && y!=EOF && y!='\n')
    {
        tokenChar(y,ps);
        if(y=='\\')
        {
            y=readChar(ps);
            tokenChar(y,ps);
        }
        y=readChar(ps);
    }
    if(y==ch)
        tokenChar(ch,ps);
    return ch=='"' ? StringToken : CharToken;
}

static int readNumber(struct parserState *ps)
{
    int a;
    a=readChar(ps);
    while(isNameChar(a)||a=='.')
    {
        tokenChar(a,ps);
        if(a=='e'||a=='E'||a=='p'||a=='P')
        {
            a=readChar(ps);
            if(a=='+'||a=='-')
                tokenChar(a,ps);
            else
                pushBack(a,ps);
        }
        a=readChar(ps);
    }
    pushBack(a,ps);
    return NumberToken;
}

static int read1(struct parserState *ps)
{
    int a=readChar(ps);
    tokenChar('!',ps);
    if(a=='=')
        tokenChar(a,ps);
    else
        pushBack(a,ps);
    return OperatorToken;
}

static int readHash(struct parserState *ps)
{
    int a=readChar(ps);
    tokenChar('#',ps);
    if(a=='#')
        tokenChar(a,ps);
    else
        pushBack(a,ps);
    return OperatorToken;
}

static int readPercent(struct parserState *ps)
{
    int a=readChar(ps);
    tokenChar('%',ps);
    if(a=='=')
        tokenChar(a,ps);
    else
        pushBack(a,ps);
    return OperatorToken;
}

static int readAnd(struct parserState *ps)
{
    int a=readChar(ps);
    tokenChar('&',ps);
    if(a=='='||a=='&')
        tokenChar(a,ps);
    else
        pushBack(a,ps);
    return OperatorToken;
}

static int readStar(struct parserState *ps)
{
    int a=readChar(ps);
    tokenChar('*',ps);
    if(a=='=')
        tokenChar(a,ps);
    else
        pushBack(a,ps);
    return OperatorToken;
}

static int readPlus(struct parserState *ps)
{
    int a=readChar(ps);
    tokenChar('+',ps);
    if(a=='='||a=='+')
        tokenChar(a,ps);
    else
        pushBack(a,ps);
    return OperatorToken;
}


static int readMinus(struct parserState *ps)
{
    int a=readChar(ps);
    tokenChar('-',ps);
    if(a=='='||a=='-')
    {
        tokenChar(a,ps);
    }
    else if(a=='>')
    {
        tokenChar(a,ps);
        a=readChar(ps);
        if(a=='*')
            tokenChar(a,ps);
        else
            pushBack(a,ps);
    }
    else
        pushBack(a,ps);
    return OperatorToken;
}

static int readPeriod(struct parserState *ps)
{
    int a=readChar(ps);
    tokenChar('.',ps);
    if(a>='0'&&a<='9')
    {
        tokenChar(a,ps);
        return readNumber(ps);
    }
    else
    {
        if(a=='*')
            tokenChar(a,ps);
        else if(a=='.')
        {
            a=readChar(ps);
            if(a=='.')
            {
                tokenChar(a,ps);
                tokenChar(a,ps);
            }
            else
            {
                pushBack(a,ps);
                pushBack('.',ps);
            }
        }
        else
            pushBack(a,ps);
        return OperatorToken;
    }
}

static int readDivide(struct parserState *ps)
{
    int a=readChar(ps);
    tokenChar('/',ps);
    if(a=='=')
        tokenChar(a,ps);
    else
        pushBack(a,ps);
    return OperatorToken;
}

static int readColon(struct parserState *ps)
{
    int a=readChar(ps);
    tokenChar(':',ps);
    if(a==':')
        tokenChar(a,ps);
    else
        pushBack(a,ps);
    return OperatorToken;
}

static int readLessThan(struct parserState *ps)
{
    int a=readChar(ps);
    tokenChar('<',ps);
    if(a=='<')
    {
        tokenChar(a,ps);
        a=readChar(ps);
        if(a=='=')
            tokenChar(a,ps);
        else
            pushBack(a,ps);
    }
    else if(a=='=')
        tokenChar(a,ps);
    else
        pushBack(a,ps);
    return OperatorToken;
}

static int readEqual(struct parserState *ps)
{
    int a=readChar(ps);
    tokenChar('=',ps);
    if(a=='=')
        tokenChar(a,ps);
    else
        pushBack(a,ps);
    return OperatorToken;
}

static int readGreaterThan(struct parserState *ps)
{
    int a=readChar(ps);
    tokenChar('>',ps);
    if(a=='>')
    {
        tokenChar(a,ps);
        a=readChar(ps);
        if(a=='=')
            tokenChar(a,ps);
        else
            pushBack(a,ps);
    }
    else if(a=='=')
        tokenChar(a,ps);
    else
        pushBack(a,ps);
    return OperatorToken;
}

static int readExponent(struct parserState *ps)
{
    int a=readChar(ps);
    tokenChar('^',ps);
    if(a=='=')
        tokenChar(a,ps);
    else

        pushBack(a,ps);
    return OperatorToken;
}

static int readOr(struct parserState *ps)
{
    int a=readChar(ps);
    tokenChar('|',ps);
    if(a=='='||a=='|')
        tokenChar(a,ps);
    else
        pushBack(a,ps);
    return OperatorToken;
}

static int tokenLineno;

static int getTokenP(struct parserState *ps)
{
    ps->tokenTop=0;
    int t=1;
    int ch;
    while(t)
    {
        t=0;
        ch=readChar(ps);
        tokenLineno=ps->equivLineno;
        switch(ch)
        {
            case '!':
                return read1(ps);
            case '\'': case '"':
                return readString(ps,ch);
            case '#':
                return readHash(ps);
            case '%':
                return readPercent(ps);
            case '&':
                return readAnd(ps);
            case '*':
                return readStar(ps);
            case '+':
                return readPlus(ps);
            case '-':
                return readMinus(ps);
            case '.':
                return readPeriod(ps);
            case '/':
                ch=readChar(ps);
                if(ch=='*')
                {
                    skipStarComment(ps);
                    t=1;
                }
                else if(ch=='/')
                {
                    skipSlashComment(ps);
                    t=1;
                }
                else
                {
                    pushBack(ch,ps);
                    return readDivide(ps);
                }
                break;
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                tokenChar(ch,ps);
                return readNumber(ps);
            case ':':
                return readColon(ps);
            case '<':
                return readLessThan(ps);
            case '=':
                return readEqual(ps);
            case '>':
                return readGreaterThan(ps);
            case '^':
                return readExponent(ps);
            case '|':
                return readOr(ps);
            case ' ': case'\n': case'\t': t=1; break;
            case EOF: return NoMoreToken;
            default:
                tokenChar(ch,ps);
                if(!isNameChar(ch))
                    return StrangeToken;
        }
    }
    while(isNameChar(ch=readChar(ps)))
    {
        tokenChar(ch,ps);
    }
    pushBack(ch,ps);
    return NameToken;
}

int getToken(struct parserState *ps)
{
    int y=getTokenP(ps);
    tokenChar(0,ps);
    ps->equivLineno=tokenLineno;
    return y;
}
