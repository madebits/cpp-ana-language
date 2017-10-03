/* (c) Vasian CEPA 2002 */
#ifndef ANA_ARRAY_H
#define ANA_ARRAY_H

#include <malloc.h>
#include <string.h>
#include "ana.h"

typedef struct {
	int dims;
	int max; // n - 1;
	int *size;
	void *data;
} ARRAY;

int* ar_expr2size(int *, STNODE*);
ARRAY* ar_initArray(SEMTYPE, ARRAY *, STNODE*);
ARRAY* ar_val2Array(VALUE);
int ar_expr2pos(ARRAY *, STNODE*);
int ar_setValue(SEMTYPE, ARRAY*, int, VALUE);
VALUE ar_getValue(SEMTYPE, ARRAY*, int);
void ar_deleteArray(ARRAY*);
ARRAY* ar_createArray(int, int *, int);
void ar_dumpArray(VALUE);
int ar_getIntVal(STACKNODE*, int, STACKNODE*);

#endif