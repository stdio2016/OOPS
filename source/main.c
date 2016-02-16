#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
HANDLE han;
#endif // WIN32

#include "parser.h"
#include "seriousError.h"
#include "compiler.h"

#ifdef WIN32
void setColor(int c)
{
    if(han!=INVALID_HANDLE_VALUE)
        SetConsoleTextAttribute(han,c);
}
#else
void setColor(int c){}
#endif // WIN32

int main(int argc,char*argv[])
{
    char *fileName="test.txt";
    FILE *f;
#ifdef WIN32
    han=GetStdHandle(STD_OUTPUT_HANDLE);
#endif // WIN32
    if(argc>1)
        fileName=argv[1];
    f=fopen(fileName,"r");
    if(f==NULL)
        seriousError("File cannot open!");
    struct parserState ps;
    parseFile(f,&ps);
    int k,lineno=0,i=1;
    while(!feof(f))
    {
        k=getToken(&ps);
        if(ps.equivLineno!=lineno)
        {
            lineno=ps.equivLineno;
            if(lineno>i*25)
            {
                setColor(0x7);
                #ifdef WIN32
                //system("pause");
                #endif // WIN32
                i++;
            }
            setColor(0x5);
            printf("\n#%03d ",lineno);
        }
#ifdef WIN32
        switch(k)
        {
            case OperatorToken:
                setColor(FOREGROUND_BLUE | FOREGROUND_GREEN);
                break;
            case NameToken:
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                break;
            case StringToken:
                setColor(FOREGROUND_RED | FOREGROUND_GREEN);
                break;
            case CharToken:
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                break;
            case NumberToken:
                setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                break;
            case StrangeToken:
                setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
                break;
            case NoMoreToken:
                setColor(0x7);
                break;
            default:
                setColor(BACKGROUND_RED | 0xf);
                break;
        }
#endif // WIN32
        printf("%s ",ps.token);

    }
    stopParsing(&ps);
    puts("");
    f=fopen(fileName,"r");
    if(f==NULL)
        seriousError("File cannot open!");
    parseFile(f,&ps);
    struct parseNode *pn;
    pn=getParseTree(&ps);
    showNode(pn,0);
    stopParsing(&ps);
    setColor(0x7);
#ifdef DEBUG
    puts("Press enter to exit.");
    getchar();
#endif
    return 0;
}
