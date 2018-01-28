TARGET = oops
OBJECT = lex.yy.o y.tab.o MyHash.o errReport.o ast.o symtable.o main.o class.o
GENERATED = lex.yy.c y.tab.c y.tab.h
CC = gcc
LEX = flex
YACC = yacc
LIBS = -lfl

$(TARGET): $(OBJECT)
	$(CC) $^ -o $(TARGET) $(LIBS)

# lex
lex.yy.o: lex.yy.c y.tab.h errReport.h ast.h
lex.yy.c: oops.lex
	$(LEX) $<

# yacc
y.tab.o: y.tab.c errReport.h ast.h symtable.h class.h MyHash.h
y.tab.c y.tab.h: oops.y
	$(YACC) -d $<

MyHash.o: MyHash.c MyHash.h
errReport.o: errReport.c errReport.h
ast.o: ast.c ast.h
symtable.o: symtable.c symtable.h ast.h class.h MyHash.h errReport.h
main.o: main.c
class.o: class.c class.h MyHash.h ast.h

.PHONY: clean
clean:
	$(RM) -f $(TARGET) $(OBJECT) $(GENERATED)
