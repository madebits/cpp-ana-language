/* (c) Vasian CEPA 2002, http://madebits.com */
#include "semtype.h"

VALUE NO_VALUE = {0, SEM_UNDEF};
VALUE VALUE_ONE = {1, SEM_INT};
VALUE VALUE_ZERO = {0, SEM_INT};
VALUE VALUE_RETURN = {1, SEM_VOID};
VALUE VALUE_BREAK = {2, SEM_VOID};

int se_isLocalArrayVal(VALUE v){
	return ((v.semType == SEM_AINT) || (v.semType == SEM_AREAL));
}

int se_isArrayVal(VALUE v){
	return ((v.semType == SEM_RAINT) || (v.semType == SEM_RAREAL)) || se_isLocalArrayVal(v);
}

int se_isEqual(VALUE v1, VALUE v2){
	if(v1.semType != v2.semType) return 0;
	if(v1.semType == SEM_REAL)
		return (v1.vdouble == v2.vdouble);
	else return (v1.vint == v2.vint);
}

int se_isRETURN(VALUE v){
	return se_isEqual(v, VALUE_RETURN);
}

int se_isBREAK(VALUE v){
	return se_isEqual(v, VALUE_BREAK);
}

int se_isTRUE(VALUE v){
	return (se_getDoubleValue(v) > 0.0);
}

int se_isFALSE(VALUE v){
	return (se_getDoubleValue(v) == 0.0);
}

int se_isNOVAL(VALUE v){
	return se_isEqual(v, NO_VALUE);
}

SEMTYPE se_semType(SEMTYPE s1, SEMTYPE s2){
	if((s1 == SEM_UNDEF) || (s2 == SEM_UNDEF)){
		printf("(%d) ST%d ST%d\n", SEM_UNDEF, s1, s2);
		yyerror("no value used in expr");
	}
	if((s1 == SEM_REAL)
		|| (s2 == SEM_REAL)
		) return SEM_REAL;
	return SEM_INT;
}

int se_getIntValue(VALUE v){
	if(v.semType == SEM_REAL) return (int)v.vdouble;
	return v.vint;
}

double se_getDoubleValue(VALUE v){
	if(v.semType == SEM_REAL) return v.vdouble;
	else return (double)v.vint;
}

VALUE se_assign(SEMTYPE semType, VALUE v2){
	VALUE temp;
	temp.semType = semType;
	if(temp.semType == SEM_INT){
		if(v2.semType == SEM_INT) temp.vint = v2.vint;
		else temp.vint = (int)v2.vdouble;
	} else {
		if(v2.semType == SEM_INT) temp.vdouble = (double)v2.vint;
		else temp.vdouble = v2.vdouble;
	}
	return temp;
}

VALUE se_calc(int op, VALUE v1, VALUE v2){
	VALUE temp;
	double dv1, dv2, result;
	if(se_isNOVAL(v1)){
		if((op != 'u') && se_isNOVAL(v2)){
			se_dumpVal(v1); se_dumpVal(v2);
			fatal("no value used in expr");
		}
	}
	dv1 = se_getDoubleValue(v1);
	dv2 = se_getDoubleValue(v2);
	if(op == 'u') temp.semType = v1.semType;
	else temp.semType = se_semType(v1.semType, v2.semType);
	switch(op){
		case 'u': result = -dv1; break;
		case '+': result = dv1 + dv2; break;
		case '-': result = dv1 - dv2; break;
		case '*': result = dv1 * dv2; break;
		case '/': if(dv2 == 0.0) result = 0.0;
			  else result = dv1 / dv2; break;
		case '%': result = (int)dv1 % (int)dv2; break;
		case '<': temp.semType = SEM_INT; result = (dv1 < dv2) ? 1.0 : 0.0; break;
		case '>': temp.semType = SEM_INT; result = (dv1 > dv2) ? 1.0 : 0.0; break;
		case O_GE: temp.semType = SEM_INT; result = (dv1 >= dv2) ? 1.0 : 0.0; break;
		case O_LE: temp.semType = SEM_INT; result = (dv1 <= dv2) ? 1.0 : 0.0; break;
		case O_EQ: temp.semType = SEM_INT; result = (dv1 == dv2) ? 1.0 : 0.0; break;
		case O_NE: temp.semType = SEM_INT; result = (dv1 != dv2) ? 1.0 : 0.0; break;
	}
	if(temp.semType == SEM_INT) temp.vint = (int)result;
	else temp.vdouble = result;
	return temp;
}

void se_dumpVal(VALUE value){
	if(se_isArrayVal(value)){
		ar_dumpArray(value);
		return;
	}
	if(value.semType == SEM_REAL) printf("%10.5lf", value.vdouble);
	else printf("%10d", value.vint);
	printf("  (T%d)\n", value.semType);
}
