/* (c) Vasian CEPA 2002 */
#ifndef ANA_SYMTAB_H
#define ANA_SYMTAB_H

/*
	simple symbol table (a set)
	scope is delegated to the interpreter
	This makes typechecking during parsing difficult
	and delegates more effort in the interpreter
*/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "stack.h"

typedef enum { SYM_KEYWD, SYM_VAR, SYM_IFUN, SYM_FUNC, SYM_PRE } SYMTYPE;

typedef struct symrec {
	char *name;
	SYMTYPE type;
	int vint;
	struct symrec *next;
}SYMREC;

extern SYMREC *symTable;
extern SYMREC symInit[];
extern int verbose;

SYMREC* sy_putSym(char*, SYMTYPE);
SYMREC* sy_getSym(char*);
void sy_initSymTable(int);
void sy_destroyAll();
void sy_dumpSymTable();

#endif