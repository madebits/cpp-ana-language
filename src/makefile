CFILES = ana.tab.c lex.yy.c symtab.c syntree.c interpreter.c semtype.c stack.c ana-k.c dmem.c array.c

ana: $(CFILES)
	gcc -o ana.exe $(CFILES)

cl: $(CFILES)
	cl /Feana.exe $(CFILES)

lex.yy.c: fl
ana.tab.c ana.tab.h: bs ver

ver:
	./version.exe version.txt
fl: ana-k.l
	flex ana-k.l
	
bs: ana.y
	bison -dt ana.y
	
clean:
	del /Q *.obj ana.tab.h ana.tab.c lex.yy.c ana.exe
