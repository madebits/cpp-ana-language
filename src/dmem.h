/* (c) Vasian CEPA 2002, http://madebits.com */
#ifndef ANA_DMEM_H
#define ANA_DMEM_H

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "stack.h"
#include "semtype.h"

typedef struct varrec {
	char *name;
	VALUE value;
	struct varrec *next;
}VARREC;


VARREC* dm_addVar(char*);
VARREC* dm_getLocalVar(char*);
VARREC* dm_getVar(char*);
void dm_destroyAll();
void dm_enterBlock();
void dm_exitBlock();
void dm_dumpMemory();
void dm_dumpStack();

void gm_init();
void gm_enterFunc();
void gm_exitFunc();
VARREC* gm_getVar(char*);

extern VARREC *global;
extern STACKNODE *funcStack;
extern VARREC *current;
extern STACKNODE *memory;

extern void fatal(const char*);
extern const char* STR_OUT_OF_MEMORY;
extern int debug_stack;

#endif