
# Makefile for TINY
# Gnu C Version
# K. Louden 2/3/98
#

CC = gcc

CFLAGS = 

OBJS = main.o util.o symtab.o analyze.o lex.yy.o cminus.tab.o code.o cgen.o

project4_2: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -g -o project4_2 -lfl

main.o: main.c globals.h util.h scan.h cgen.h
	$(CC) $(CFLAGS) -c main.c

util.o: util.c util.h globals.h cminus.tab.h
	$(CC) $(CFLAGS) -c util.c

symtab.o: symtab.c symtab.h
	$(CC) $(CFLAGS) -c symtab.c

analyze.o: analyze.c analyze.h globals.h symtab.h
	$(CC) $(CFLAGS) -c analyze.c

code.o: code.c code.h globals.h
	$(CC) $(CFLAGS) -c code.c

cgen.o: cgen.c globals.h symtab.h code.h cgen.h
	$(CC) $(CFLAGS) -c cgen.c

lex.yy.o: lex.yy.c util.h globals.h cminus.tab.h
	$(CC) $(CFLAGS) -c lex.yy.c

lex.yy.c: cminus.l
	flex cminus.l

cminus.tab.o: cminus.tab.c
	$(CC) $(CFLAGS) -c cminus.tab.c

cminus.tab.c:
cminus.tab.h: cminus.y
	bison -d -v cminus.y

clean:
	-rm project4_2
	-rm lex.yy.c
	-rm cminus.tab.c
	-rm cminus.tab.h
	-rm cminus.output
	-rm $(OBJS)
