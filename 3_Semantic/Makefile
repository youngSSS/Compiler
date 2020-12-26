CC = gcc

CFLAGS = -Wall -w

OBJS = main.o util.o lex.yy.o y.tab.o symtab.o analyze.o 

cminus: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o cminus -lfl

main.o: main.c globals.h util.h scan.h parse.h analyze.h cgen.h
	$(CC) $(CFLAGS) -c main.c

util.o: util.c util.h globals.h
	$(CC) $(CFLAGS) -c util.c

symtab.o: symtab.c symtab.h globals.h util.h
	$(CC) $(CFLAGS) -c symtab.c

analyze.o: analyze.c globals.h symtab.h analyze.h
	$(CC) $(CFLAGS) -c analyze.c

lex.yy.o: cminus.l scan.h util.h globals.h
	flex cminus.l
	$(CC) $(CFLAGS) -c lex.yy.c -lfl

y.tab.o: cminus.y globals.h util.h scan.h parse.h
	yacc -d cminus.y
	$(CC) $(CFLAGS) -c y.tab.c -lfl

clean:
	-rm cminus
	-rm $(OBJS)
