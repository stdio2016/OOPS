#include<stdio.h>
#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED
enum TokenType
{
    OperatorToken=1,
    NameToken    =2,
    StringToken  =3,
    CharToken    =4,
    NumberToken  =5,
    ParenToken   =6,
    StrangeToken =123,
    NoMoreToken  =-1,
};

struct parserState
{
    int tokenLen;
    int tokenTop;
    char *token;
    FILE *f;
    int pushedBackN;  // length of pushedBack
    int pushTop;      // # of chars in pushedBack
    int *pushedBack; // not parsed chars
    int lineno;
    int equivLineno;
};

// get a parseState object and parse program
struct parserState *parseFile(FILE *f,struct parserState *p);
// stop parsing and close file
void stopParsing(struct parserState *ps);

int isNameChar(int ch);
// get a token and store it in ps->token
int getToken(struct parserState *ps);


#endif // PARSER_H_INCLUDED
