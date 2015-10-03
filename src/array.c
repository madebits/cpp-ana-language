/* (c) Vasian CEPA 2002, http://madebits.com */
#include "array.h"

ARRAY* ar_createArray(int dims, int *size, int type){
	int st, r = 1, i;
	ARRAY *a;
	if(size == NULL) return NULL;
	if(dims <= 0) return NULL;
	switch(type){
		case SEM_AINT: st = sizeof(int); break;
		case SEM_AREAL: st = sizeof(double); break;
		default: return NULL;
	}
	if((a = (ARRAY*)malloc(sizeof(ARRAY))) == NULL) fatal(STR_OUT_OF_MEMORY);
	a->dims = dims;
	a->size = size;
	for(i = 0; i < dims; i++) r *= size[i];
	a->max = r - 1;
	st = st * r;
	if((a->data = malloc(st)) == NULL) fatal(STR_OUT_OF_MEMORY);
	memset(a->data, 0, st); // init array to 0-s;
	a->max = r - 1;
	return a;
}

void ar_deleteArray(ARRAY* a){
	if(a == NULL) return;
	if(a->data != NULL) free(a->data);
	if(a->size != NULL) free(a->size);
	free(a);
	a = NULL;
}

VALUE ar_getValue(SEMTYPE type, ARRAY* a, int pos){
	VALUE v;
	if(a == NULL) return NO_VALUE;
	if(a->data == NULL) return NO_VALUE;
	if(pos > a->max) return NO_VALUE;
	switch(type){
		case SEM_RAINT:
		case SEM_AINT:	v.vint = ((int*)a->data)[pos];
						v.semType = SEM_INT;
						break;
		case SEM_RAREAL:
		case SEM_AREAL:	v.vdouble = ((double*)a->data)[pos];
						v.semType = SEM_REAL;
						break;
		default:		return NO_VALUE;
	}
	return v;
}

int ar_setValue(SEMTYPE type, ARRAY*a, int pos, VALUE v){
	//printf("D: (%d) ar_setValue ", type); se_dumpVal(v);
	if((a == NULL) || (a->data == NULL)) return 0;
	if(pos > a->max) return 0;
	if(se_isArrayVal(v)) return 0;
	switch(type){
		case SEM_RAINT:
		case SEM_AINT:	((int*)a->data)[pos] = se_getIntValue(v); break;
		case SEM_RAREAL:
		case SEM_AREAL:	((double*)a->data)[pos] = se_getDoubleValue(v); break;
		default: 		return 0;
	}
	return 1;
}

int ar_expr2pos(ARRAY *a, STNODE* eList){
	STACKNODE *vals, *t;
	int i = 0, pos = 0, ival;
	VALUE *v;
	if(a == NULL) return -1;
	if(eList == NULL) return -1;
	vals = fu_evalExprList(eList);
	if(vals == NULL) return -1;
 	for(t = vals; t != NULL; t = t->next){
		if(++i >= a->dims) break;
		pos += a->size[i] * ar_getIntVal(vals, a->size[i - 1], t);
	}
	if(t == NULL || t->next !=NULL){
		sk_destroyStack(&vals);
		fatal("number of dims does not match");
	}
	pos += ar_getIntVal(vals, a->size[i - 1], t);
	sk_destroyStack(&vals);
	if((pos < 0) || (pos > a->max)){
		printf("\n%d ", pos);
		fatal("array access out of bounds");
	}
	return pos;
}

int ar_getIntVal(STACKNODE* vals, int size, STACKNODE* t){
	int ival;
	VALUE* v = v = (VALUE*)t->data1;
	if(se_isArrayVal(*v)) return -1;
	ival = se_getIntValue(*v);
	if(ival >= size){
		sk_destroyStack(&vals);
		printf("\n%d ", ival);
		fatal("array access out of bounds or number of dims does not match");
	}
	return ival;
}

int* ar_expr2size(int *dims, STNODE* eList){
	STACKNODE *vals, *t;
	VALUE *v;
	int *size, i = 0;
	vals = fu_evalExprList(eList);
	if(vals == NULL) return NULL;
	for(t = vals; t != NULL; t = t->next) (*dims)++;
	size = (int*)malloc(sizeof(int)* (*dims));
	if(size == NULL) fatal(STR_OUT_OF_MEMORY);
	for(t = vals; t != NULL; t = t->next){
		v = (VALUE*)t->data1;
		if(se_isArrayVal(*v)) return NULL;
		size[i++] = se_getIntValue(*v);
	}
	sk_destroyStack(&vals);
	return size;
}

ARRAY* ar_initArray(SEMTYPE type, ARRAY *a, STNODE* eList){
	STACKNODE *vals, *t;
	VALUE *v;
	int i = 0;
	if(a == NULL) return NULL;
	if(eList == NULL) return NULL;
	vals = fu_evalExprList(eList);
	if(vals == NULL) return NULL;
	for(t = vals; t != NULL; t = t->next){
		v = (VALUE*)t->data1;
		if(se_isArrayVal(*v)) return NULL;
		if(!ar_setValue(type, a, i, *v)) return NULL;
		i++;
	}
	sk_destroyStack(&vals);
	return a;
}

ARRAY* ar_val2Array(VALUE v){
	if(se_isArrayVal(v)) return (ARRAY*)v.vint;
	return NULL;
}

void ar_dumpArray(VALUE v){
	ARRAY *a;
	int i;
	if(!se_isArrayVal(v)){
		puts("not an array");
		return;
	}
	a = (ARRAY*)v.vint;
	if(a == NULL){
		puts("null array");
		return;
	}
	printf("D%d M%d [", a->dims, a->max);
	for(i = 0; i < a->dims; i++){
		printf("%d", a->size[i]);
		if(i != (a->dims -1)) printf(", ");
	}
	printf("] {");
	for(i = 0; i <= a->max; i++){
		switch(v.semType){
		case SEM_RAINT:
		case SEM_AINT:	printf("%d", ((int*)a->data)[i]); break;
		case SEM_RAREAL:
		case SEM_AREAL:	printf("%.2lf", ((double*)a->data)[i]); break;
		}
		if(i != a->max) printf(", ");
	}
	printf("} (T%d)\n", v.semType);
}