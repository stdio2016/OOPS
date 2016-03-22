#ifndef COMPILER_H_INCLUDED
#define COMPILER_H_INCLUDED

struct parseNode
{
    int type;
    int lineno;
    struct parseNode *next;
    union parseNodeData
    {
        struct parseNode *n;
        char *s;
    } dat;
};

struct parseNode *getParseTree(struct parserState *ps);

void showNode(struct parseNode *pn,int indent);

void freeAll(struct parseNode *pn);

struct arrayType
{
    int length;
    int maxLen;
    void **data;
};

struct stringType
{
    int length;
    int maxLen;
    char *data;
};

#endif // COMPILER_H_INCLUDED
