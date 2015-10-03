/* (c) Vasian CEPA 2002, http://madebits.com */
#include "interpreter.h"
#include <stdlib.h> // for rand()

#define VAL_RET(val) if(se_isRETURN(val)) return VALUE_RETURN
#define VAL_BRE(val) if(breakOn && se_isBREAK(val)) return VALUE_BREAK

static char* in_opcode2str(int);
static void printTab(int);
static VARREC* in_checkVar(char*);
static VALUE nextRand();

int breakOn = 0;
STNODE* lastNode = NULL;

void in_dumpEval(STNODE* n){
	char *c;
	printf("\n- eval ");
	c = in_opcode2str(n->value.vint);
	if(c != (char)0) printf("%s", c);
	else printf("%c", n->value.vint);
	printf(" %d -\n", n->value.vint);
}

VALUE eval(STNODE* n){
	STNODE *t;
	VARREC *v;
	SYMREC *s;
	VALUE val1, val2;
	int i;
	if(n == NULL) return NO_VALUE;
	if(debug_eval) in_dumpEval(n);
	if(debug_stree) in_dumpSynTree(n, 0);
	lastNode = n;
	switch(n->type){
		case NODE_CON:	return n->value;
		case NODE_ID:	v = in_checkVar(node2sym(n)->name);
						return v->value;
		case NODE_FUN:	gm_enterFunc();
						if(debug_mem){
							printf("D: begin func %s() exec:\n", fu_getFuncName(n));
							dm_dumpMemory();
						}
						for(i = 0; i < (n->nops - 1); i++)
							if(se_isRETURN(eval(n->op[i]))) break;
						s = (SYMREC*)(n->value.vint);
						val1 = in_checkVar(s->name)->value;
						eval(n->op[n->nops - 1]); /* BOUT */
						gm_exitFunc();
						return val1;
		case NODE_OPR:
			switch(n->value.vint){
				case O_PRINT:	if(print_name && (n->op[0]->type == NODE_ID))
									printf(" %s = ", node2sym(n->op[0])->name);
								if(n->op[0]->type == NODE_STR) str_iprint((char*)n->op[0]->value.vint);
								else iprint(eval(n->op[0]));
								return NO_VALUE;
				case O_READ:	iread(in_checkVar(node2sym(n->op[0])->name));
								return NO_VALUE;
				case O_EXIT:	iexit();
				case O_DUMP:	dm_dumpMemory();
								return NO_VALUE;
				case O_RAND:	return nextRand();
				case O_INB:		dm_enterBlock(); return NO_VALUE;
				case O_OUTB:	dm_exitBlock(); return NO_VALUE;
				case O_ADDV:	in_defineVar(n->op[0]);
								return NO_VALUE;
				case O_ADDA:	v = in_defineVar(n->op[0]);
								in_defineArray(v, n->op[1]);
								return NO_VALUE;
				case O_INIA:	in_initArray(n);
								return NO_VALUE;
				case ';':		for(i = 0; i < (n->nops - 1); i++){
									val1 = eval(n->op[i]);
									VAL_BRE(val1);
									VAL_RET(val1);
								}
								return eval(n->op[n->nops - 1]);
				case O_IF:		if(eval(n->op[0]).vint != 0){
									val1 = eval(n->op[1]);
									VAL_BRE(val1);
									VAL_RET(val1);
								}
								else if(n->nops > 2) {
									val1 = eval(n->op[2]);
									VAL_BRE(val1);
									VAL_RET(val1);
								}
								return NO_VALUE;
				case O_WHILE:	breakOn = 1;
								while(se_isTRUE(eval(n->op[0]))){
									val1 = eval(n->op[1]);
									if(se_isRETURN(val1) || se_isBREAK(val1)){
										/* if block, excute OUTB */
										if(in_isBlockNode(n->op[1]))
											eval(n->op[1]->op[n->op[1]->nops - 1]);
										breakOn = 0;
										return se_isRETURN(val1) ? VALUE_RETURN : NO_VALUE;
									}
								}
								breakOn = 0;
								return NO_VALUE;
				case O_BREAK:	return (breakOn) ? VALUE_BREAK : NO_VALUE;
				case O_RET:		if(n->nops > 0) eval(n->op[0]);
								return VALUE_RETURN;
				case O_ASSF:	v = in_checkVar(node2sym(n->op[0])->name);
								if(n->nops == 3)
									val1 = in_setArrayVal(v, n);
								else {
									if(n->op[1]->type == NODE_CON){
										if(se_isArrayVal(v->value)){
											int cstype = n->op[1]->value.semType;
											cstype = ((cstype == SEM_AINT) || (cstype == SEM_RAINT));
											switch(v->value.semType){
												case SEM_RAINT:
												case SEM_AINT: if(!cstype) fatal("types do not match"); break;
												case SEM_RAREAL:
												case SEM_AREAL: if(cstype) fatal("types do not match"); break;
											}
										}
									}
									val1 = v->value = se_assign(v->value.semType, eval(n->op[1]));
								}
								return val1;
				case '=':		v = in_checkVar(node2sym(n->op[0])->name);
								if(n->nops == 3)
									val1 = in_setArrayVal(v, n);
								else {
									if(se_isArrayVal(v->value)){
										printf(" %s ", v->name);
										fatal("array user as static");
									}
									val1 = eval(n->op[1]);
									if(se_isLocalArrayVal(val1)){
										printf("at %s ", v->name);
										fatal("array user as static");
									}
									val1 = v->value = se_assign(v->value.semType, val1);
								}
								return val1;
				case O_GETA:	return in_getArrayVal(n);
				case 'u':		return se_calc('u', eval(n->op[0]), NO_VALUE);
				case '+':		return se_calc('+', eval(n->op[0]), eval(n->op[1]));
				case '-':		return se_calc('-', eval(n->op[0]), eval(n->op[1]));
				case '*':		return se_calc('*', eval(n->op[0]), eval(n->op[1]));
				case '/':		return se_calc('/', eval(n->op[0]), eval(n->op[1]));
				case '%':		return se_calc('%', eval(n->op[0]), eval(n->op[1]));
				case '<':		return se_calc('<', eval(n->op[0]), eval(n->op[1]));
				case '>':		return se_calc('>', eval(n->op[0]), eval(n->op[1]));
				case O_GE:		return se_calc(O_GE, eval(n->op[0]), eval(n->op[1]));
				case O_LE:		return se_calc(O_LE, eval(n->op[0]), eval(n->op[1]));
				case O_EQ:		return se_calc(O_EQ, eval(n->op[0]), eval(n->op[1]));
				case O_NE:		return se_calc(O_NE, eval(n->op[0]), eval(n->op[1]));
				case O_AND:		val1 = se_assign(SEM_INT, eval(n->op[0]));
								if(val1.vint == 0) return VALUE_ZERO;
								else { /* short circuit */
									val2 = se_assign(SEM_INT, eval(n->op[1]));
									if(val1.vint && val2.vint) return VALUE_ONE;
								}
								return VALUE_ZERO;
				case O_OR:		val1 = se_assign(SEM_INT, eval(n->op[0]));
								if(val1.vint != 0) return VALUE_ONE;
								else { /* short circuit */
									val2 = se_assign(SEM_INT, eval(n->op[1]));
									if(val1.vint || val2.vint) return VALUE_ONE;
								}
								return VALUE_ZERO;
				case O_NOT:		val1 = se_assign(SEM_INT, eval(n->op[0]));
								if(val1.vint == 0) return VALUE_ONE;
								return VALUE_ZERO;
				case O_CALL:	s = (SYMREC*)n->op[0]->value.vint;
								if(debug_fcall) printf("D: FUN CALL %s\n", s->name);
								if(s->type != SYM_FUNC){
									printf("[%s()] ", s->name);
									fatal("undefined function");
								}
								if(n->nops == 0) val1 = eval(n->op[0]);
								else val1 = eval(fu_activateFuncNode(n->op[0], n->op[1]));
								if(debug_fcall) {
									printf("D: RET val");
									se_dumpVal(val1);
								}
								if(n->op[0]->value.semType != SEM_VOID) return val1;
								return NO_VALUE;
				case O_SIZE:	return in_isize(n);
				case O_LEN:		return in_ilen(n);
				}
	}
	return NO_VALUE;
}

int in_isBlockNode(STNODE* n){
	STNODE* t;
	if((n != NULL) && (n->op != NULL)){
		t = n->op[0];
		if((t != NULL) && (t->value.vint == O_INB))
			return 1;
	}
	return 0;
}

VALUE in_ilen(STNODE* n){
	VALUE val;
	ARRAY* a;
	VARREC* v;
	v = in_checkVar(node2sym(n->op[0])->name);
	a = ar_val2Array(v->value);
	if(a == NULL) return VALUE_ONE;
	val.vint = a->max + 1;
	return val;
}

VALUE in_isize(STNODE* n){
	VALUE val, VM1 = {-1, SEM_INT};
	VARREC* v;
	ARRAY* a;
	int ss, np;
	val.semType = SEM_INT;
	np = (n->nops == 2);
	v = in_checkVar(node2sym(n->op[0])->name);
	if(np) ss = se_getIntValue(eval(n->op[1]));
	a = ar_val2Array(v->value);
	if(a == NULL){ // var
		if(np)
			return (ss == 1) ? VALUE_ONE : VM1;
		else return VALUE_ONE;
	}
	if(np){
		if((ss < 0) || (ss >= a->dims)) return VM1;
		val.vint = a->size[ss];
		return val;
	}
	val.vint = a->dims;
	return val;
}

VARREC* in_defineVar(STNODE* n){
	VARREC* v;
	SYMREC* s = node2sym(n);
	v = dm_getLocalVar(s->name);
	if(v != NULL){
		printf("[%s] ", s->name);
		fatal("duplicate var definition inside scope");
	} else {
		v = dm_addVar(s->name);
		if(v == NULL) fatal("out of memory");
		v->value.semType = n->value.semType;
	}
	return v;
}

VARREC* in_defineArray(VARREC* v, STNODE* eList){
	ARRAY* a;
	int dims = 0;
	int *size = ar_expr2size(&dims, eList);
	a = ar_createArray(dims, size, v->value.semType);
	if(a == NULL) fatal("array failed");
	v->value.vint = (int)a;
	return v;
}

VARREC* in_initArray(STNODE* n){
	VARREC* v;
	ARRAY* a;
	v = in_checkVar(node2sym(n->op[0])->name);
	a = ar_val2Array(v->value);
	if(a == NULL) fatal("not a array");
	a = ar_initArray(v->value.semType, a, n->op[1]);
	v->value.vint = (int)a;
	// ar_dumpArray(v->value);
	return v;
}

VALUE in_setArrayVal(VARREC* v, STNODE* n){
	ARRAY* a;
	VALUE val;
	a = ar_val2Array(v->value);
	if(a == NULL) fatal("not a array");
	val = eval(n->op[2]);
	if(

		!ar_setValue(v->value.semType, a, ar_expr2pos(a, n->op[1]), val)
	) return NO_VALUE;
	return val;
}

VALUE in_getArrayVal(STNODE *n){
	VARREC* v = in_checkVar(node2sym(n->op[0])->name);
	ARRAY *a = ar_val2Array(v->value);
	return ar_getValue(v->value.semType, a, ar_expr2pos(a, n->op[1]));
}

SYMREC* node2sym(STNODE* n){
	return (SYMREC*)n->value.vint;
}

STNODE* sym2node(SYMREC* s){
	STNODE *n = st_createEmptyNode(NODE_ID);
	n->value.vint = (int)s;
	n->value.semType = SEM_VOID;
	return n;
}

static VARREC* in_checkVar(char* name){
	VARREC* v = gm_getVar(name);
	if(v == NULL){
		printf("[%s] ", name);
		fatal("no such variable in current scope");
	}
	if(debug_access > 1){
		printf("D: VAR ACCESS: %s ", name);
		se_dumpVal(v->value);
	}
	return v;
}

VALUE nextRand()
{
	VALUE val;
	val.semType = SEM_INT;
	val.vint = rand();
	return val;
}

/* --- stree debuger --- */

void in_dumpSynTree(STNODE* n, int t){
	int i;
	SYMREC *s;
	char *c;
	if(n == NULL){
		printTab(t); printf("<NULL>\n");
		return;
	}
	printTab(t);
	switch(n->type){
		case NODE_CON:	printf("constant ");
						if(n->value.semType == SEM_REAL)
							printf("%lf ", n->value.vdouble);
						else printf("%d ", n->value.vint);
						break;
		case NODE_ID:	s = (SYMREC*)n->value.vint;
						printf("ID %s 0x%x ", s->name, n->value.vint);
						break;
		case NODE_OPR:	c = in_opcode2str(n->value.vint);
						if(c != (char)0) printf("%s", c);
						else printf("%c", n->value.vint);
						printf(" ");
						/* printf(" %4d ", n->value.vint); */
						break;
		case NODE_FUN:	printf("FUNC %s 0x%x ", fu_getFuncName(n), n->value.vint);
						break;
		case NODE_STR:	printf("STR 0x%x ", n->value.vint); break;
		default:		printf("NODE 0x%x ", n->value.vint); break;
	}
	if((n->nops > 0) && (n->type != NODE_FUN)) printf("nops=%d ", n->nops);
	switch(n->value.semType){
		case SEM_INT:	printf("int"); break;
		case SEM_REAL:	printf("real"); break;
		case SEM_UNDEF:	printf("none"); break;
		case SEM_VOID:	printf("void"); break;
		case SEM_AINT:	printf("aint"); break;
		case SEM_AREAL:	printf("areal"); break;
		case SEM_RAREAL:	printf("ref_areal"); break;
		case SEM_RAINT:	printf("ref_aint"); break;
		default:		printf("???"); break;
	}
	printf("\n");
	t++;
	if(n->type == NODE_FUN) return;
	for(i = 0; i < n->nops; i++)
		in_dumpSynTree(n->op[i], t);
}

void in_dumpFunctions(){
		SYMREC *temp;
		STNODE *n;
		int i;
		puts("--- function table ---");
		for(temp = symTable; temp != NULL; temp = (SYMREC*)temp->next){
			if(temp->type == SYM_FUNC){
				n = (STNODE*)temp->vint;
				printf("\nSTART FUNCTION nops=%d\n", n->nops);
				in_dumpSynTree(n, 0);
				for(i = 0; i < n->nops; i++)
					in_dumpSynTree(n->op[i], 1);
				printf("END FUNCTION\n");
			}
		}
		puts("\n--------------------");
}

static void printTab(int t){
	int i;
	printf("%% ");
	if(t > 0) {
		printf("  ");
		for(i = 1; i < t; i++) printf(". ");
	}
}

static char* in_opcode2str(int op){
	switch(op){
		case O_PRINT: return "PRINT";
		case O_READ: return "READ";
		case O_EXIT: return "EXIT";
		case O_DUMP: return "DUMP";
		case O_LE: return "<=";
		case O_GE: return ">=";
		case O_EQ: return "==";
		case O_NE: return "!=";
		case O_IF: return "IF";
		case O_WHILE: return "WHILE";
		case O_INB: return "ENTER BLOCK";
		case O_OUTB: return "EXIT BLOCK";
		case O_ADDV: return "DEFINE VAR";
		case O_AND:	return "AND";
		case O_OR: return "OR";
		case O_NOT: return "NOT";
		case O_CALL: return "CALL";
		case O_RET: return "RETURN";
		case O_BREAK: return "BREAK";
		case O_ADDA: return "DEFINE ARR";
		case O_INIA: return "INIT ARR";
		case O_GETA: return "GET ARR";
		case O_SIZE: return "SIZE";
		case O_LEN: return "LEN";
		case O_ASSF: return "SET";
		default: return 0;
	}
}