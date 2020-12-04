CC = gcc
CFLAGS = 

OBJS = main.o util.o scan.o parse.o symtab.o analyze.o code.o cgen.o
OBJS_C_CODE = main.o util.o scan.o globals.h
OBJS_FLEX = main.o util.o lex.yy.o globals.h

cminus: $(OBJS_C_CODE)
	$(CC) $(CFLAGS) $(OBJS_C_CODE) -o cminus

cminus_flex: $(OBJS_FLEX)
	$(CC) $(CFLAGS) main.o util.o lex.yy.o -o cminus_flex -lfl

lex.yy.o: cminus.l scan.h util.h globals.h
	flex cminus.l
	$(CC) $(CFLAGS) -c lex.yy.c -lfl

clean_cminus:
	rm cminus main.o util.o scan.o

clean_cminus_flex:
	rm cminus_flex main.o util.o lex.yy.o
