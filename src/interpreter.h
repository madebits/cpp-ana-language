/* (c) Vasian CEPA 2002, http://madebits.com */
#ifndef ANA_INTERPRETER_H
#define ANA_INTERPRETER_H

#include <time.h> // for random
#include <stdlib.h> // for RAND_MAX
#include <string.h> // for toupper
#include <limits.h> // for INT_MAX
#include <float.h> // for DBL_MAX

#include "symtab.h"
#include "syntree.h"
#include "semtype.h"
#include "ana.h"
#include "oper.h"
#include "dmem.h"
#include "array.h"

VALUE eval(STNODE*);
SYMREC* node2sym(STNODE*);
STNODE* sym2node(SYMREC*);
VARREC* in_defineVar(STNODE*);
VARREC* in_defineArray(VARREC*, STNODE*);
VARREC* in_initArray(STNODE*);
VALUE in_setArrayVal(VARREC*,STNODE*);
VALUE in_getArrayVal(STNODE*);
VALUE in_isize(STNODE*);
VALUE in_ilen(STNODE*);
void in_dumpEval(STNODE* n);
int in_isBlockNode(STNODE*);

void in_dumpSynTree(STNODE*, int);
extern STNODE* fu_activateFuncNode(STNODE*, STNODE*);
extern STNODE* lastNode;

#endif