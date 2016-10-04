#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <stdexcept>

using namespace std;

class SyntaxError : public runtime_error{

};

class tokenizer{
    istream *f;
    stringstream ss;
    void skiplinecomment();
public:
    tokenizer(istream &in): f(&in), ss(){}
    int next();
    string val;
    static const int word=0;
    static const int eof=-1;
};

int tokenizer::next(){
    val="";
    int ch=f->get();
    if(ch==EOF)
        return eof;
    ss.clear();
    skiplinecomment();
    return word;
}

void tokenizer::skiplinecomment()
{
    int ch=f->get();
    while(ch!=EOF && ch!='\n' && ch!='\r'){
        ch=f->get();
    }
    f->unget();
}

bool isNameChar(int ch){
    return (ch>='a'&&ch<='z') || (ch>='A'&&ch<='Z') || (ch>='0'&&ch<='9') || ch=='_' || ch=='$';
}

bool isNameStartChar(int ch){
    return (ch>='a'&&ch<='z') || (ch>='A'&&ch<='Z') || ch=='_' || ch=='$';
}

bool isSpace(int ch){
    return ch==' ' || ch=='\n' || ch=='\r' || ch=='\t';
}

int main()
{
    ifstream f;
    f.open("o.txt");
    if(!f){
        return 404;
    }
    tokenizer tt(f);
    int status;
    while(tt.next()==tokenizer::word){
        cout<<tt.val;
    }
    f.close();
    return 0;
}
