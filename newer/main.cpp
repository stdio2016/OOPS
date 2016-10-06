#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <stdexcept>

using namespace std;

class SyntaxError : public runtime_error{
public:
    SyntaxError(string aa): runtime_error(aa){}
};

class tokenizer{
    istream *f;
    stringstream ss;
    void skiplinecomment();
    void skipstarcomment();
    void skipspace();
public:
    tokenizer(istream &in): f(&in), ss(){}
    int next();
    string val;
    static const int Name=0;
    static const int String=1;
    static const int Op=2;
    static const int Eof=-1;
    static bool isNameChar(int ch);
    static bool isNameStartChar(int ch);
};

bool tokenizer::isNameChar(int ch){
    return (ch>='a'&&ch<='z') || (ch>='A'&&ch<='Z') || (ch>='0'&&ch<='9') || ch=='_' || ch=='$';
}

bool tokenizer::isNameStartChar(int ch){
    return (ch>='a'&&ch<='z') || (ch>='A'&&ch<='Z') || ch=='_' || ch=='$';
}

bool isSpace(int ch){
    return ch==' ' || ch=='\n' || ch=='\r' || ch=='\t';
}

int tokenizer::next(){
    val="";
    ss.clear();
    skipspace();
    int ch=f->get();
    if(isNameStartChar(ch))
    {
        do{
            ss.put(ch);
            ch=f->get();
        }while(isNameChar(ch)) ;
        f->unget();
        ss>>val;
        return Name;
    }
    else {
        switch(ch){
            case EOF: return Eof;
            case '"':
                return String;
            case ':':
            case '{':
            case '}':
            case '(':
            case ')':
            case ',':
            case '=':
            case '.':
            case ';':
                ss.put(ch);
                ss>>val;
                return Op;
            default:
                ss.put(ch);
                ss>>val;
                throw SyntaxError(string("Illegal character: ")+val);
        }
    }
    return Eof;
}

void tokenizer::skipspace()
{
    int ch;
    bool space=true;
    do {
        ch=f->get();
        switch(ch){
        case EOF:
            return;
        case '/':
            ch=f->get();
            if(ch=='/')
                skiplinecomment();
            else if(ch=='*')
                skipstarcomment();
            else
                throw SyntaxError("Illegal character: /");
            break;
        case ' ':
        case '\n':
        case '\r':
        case '\t':
            break;
        default:
            space=false;
        }
    } while(space);
    f->unget();
}

void tokenizer::skiplinecomment()
{
    int ch=f->get();
    while(ch!=EOF && ch!='\n' && ch!='\r'){
        ch=f->get();
    }
    f->unget();
}

void tokenizer::skipstarcomment()
{
    int ch=f->get();
    int state=0;
    while(state==0 || ch!='/'){
        if(ch=='*')
            state=1;
        else
            state=0;
        ch=f->get();
        if(ch==EOF)
            throw SyntaxError("unterminated comment");
    }
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
    int result=0;
    try{
        status=tt.next();
        while(status!=tokenizer::Eof){
            cout<<tt.val<<endl;
            status=tt.next();
        }
    }
    catch(SyntaxError error){
        cout<<error.what();
        result=1;
    }
    f.close();
    return result;
}
