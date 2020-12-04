CC = gcc
CFLAGS = 

OBJS = main.o util.o scan.o parse.o symtab.o analyze.o code.o cgen.o
OBJS_SCANNER = main.o util.o scan.o globals.h
OBJS_FLEX = main.o util.o lex.yy.o globals.h
OBJS_BISON = main.o util.o lex.yy.o symtab.o y.tab.o globals.h

cminus: $(OBJS_SCANNER)
	$(CC) $(CFLAGS) $(OBJS_SCANNER) -o cminus

cminus_flex: $(OBJS_FLEX)
	$(CC) $(CFLAGS) main.o util.o lex.yy.o -o cminus_flex -lfl

lex.yy.o: cminus.l scan.h util.h globals.h
	flex cminus.l
	$(CC) $(CFLAGS) -c lex.yy.c -lfl

cminus_bison: $(OBJS_BISON)
	$(CC) $(CFLAGS) $(OBJS_BISON) -o cminus_bison -lfl

y.tab.o: cminus.y globals.h
	bison -d cminus.y --yacc
	$(CC) $(CFLAGS) -c y.tab.c

clean_cminus:
	rm cminus main.o util.o scan.o

clean_cminus_flex:
	rm cminus_flex main.o util.o lex.yy.o

clean_cminus_bison:
	rm cminus_bison main.o util.o lex.yy.o symtab.o y.tab.o
