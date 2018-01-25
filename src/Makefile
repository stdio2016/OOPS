TARGET = oops
OBJECT = lex.yy.o y.tab.o MyHash.o errReport.o
GENERATED = lex.yy.c y.tab.c y.tab.h
CC = gcc
LEX = flex
YACC = yacc
LIBS = -lfl

$(TARGET): $(OBJECT)
	$(CC) $^ -o $(TARGET) $(LIBS)

# lex
lex.yy.o: lex.yy.c y.tab.h errReport.h
lex.yy.c: oops.lex
	$(LEX) $<

# yacc
y.tab.o: y.tab.c errReport.h
y.tab.c y.tab.h: oops.y
	$(YACC) -d $<

MyHash.o: MyHash.c MyHash.h
errReport.o: errReport.c errReport.h

.PHONY: clean
clean:
	$(RM) -f $(TARGET) $(OBJECT) $(GENERATED)
