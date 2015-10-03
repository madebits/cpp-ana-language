/* (c) Vasian CEPA 2002, http://madebits.com */
#ifndef ANA_ANA_H
#define ANA_ANA_H

#include <stdio.h>
#include "semtype.h"
#include "syntree.h"
#include "dmem.h"
#include "interpreter.h"
#include "stack.h"

#define vLength(v) (sizeof(v)/sizeof(v[0]))

void iprint(VALUE);
void iread(VARREC*);
void iexit();
void fatal(const char*);
void freeMemory();

STNODE* fu_patchFunctionNode(STNODE*, STNODE*, STNODE*);
STNODE* fu_activateFuncNode(STNODE*, STNODE*);
char* fu_getFuncName(STNODE*);
STNODE* fu_getFuncNode(char*);
STACKNODE* fu_evalExprList(STNODE*);
int fu_stackVal(PSTACK, VALUE);
STNODE* fu_createAssignBlock(STNODE*);

extern STNODE* createAssignNode(SYMREC*, STNODE*);
extern STNODE* nodeDefVar(SYMREC*);

extern STACKNODE* strTable;
void str_destroyAll();
int str_addStr(char*);
void str_iprint(char*);
int str_getNEWLINE();
int str_findStr(char*);

extern const char* STR_OUT_OF_MEMORY;

extern int debug_sym;
extern int debug_mem;
extern int debug_eval;
extern int debug_stree;
extern int debug_ftable;
extern int debug_access;
extern int debug_fcall;
extern int debug_fatal;
extern int print_name;


#endif






