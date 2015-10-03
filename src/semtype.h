/* (c) Vasian CEPA 2002, http://madebits.com */
#ifndef ANA_SEMTYPE_H
#define ANA_SEMTYPE_H

#include "oper.h"

typedef enum {SEM_INT, SEM_REAL, SEM_VOID, SEM_UNDEF, SEM_AINT, SEM_AREAL, SEM_RAINT, SEM_RAREAL } SEMTYPE;

typedef struct {
	union {
		int vint;
		double vdouble;
	};
	SEMTYPE semType;
} VALUE;

extern VALUE NO_VALUE;
extern VALUE VALUE_ONE;
extern VALUE VALUE_ZERO;
extern VALUE VALUE_RETURN;
extern VALUE VALUE_BREAK;

SEMTYPE se_semType(SEMTYPE, SEMTYPE);
double se_getDoubleValue(VALUE);
int se_getIntValue(VALUE);
VALUE se_calc(int op, VALUE, VALUE);
VALUE se_assign(SEMTYPE, VALUE);
void se_dumpVal(VALUE);
int se_isEqual(VALUE, VALUE);
int se_isRETURN(VALUE);
int se_isTRUE(VALUE);
int se_isFALSE(VALUE);
int se_isNOVAL(VALUE);
int se_isArrayVal(VALUE);
int se_isLocalArrayVal(VALUE);

#endif