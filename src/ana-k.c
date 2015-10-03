/* (c) Vasian CEPA 2002, http://madebits.com */
#include "ana.h"

int debug_sym = 0;
int debug_mem = 0;
int debug_eval = 0;
int debug_stree = 0;
int debug_ftable = 0;
int debug_access = 0;
int debug_fcall = 0;
int debug_fatal = 0;
int print_name = 1;


int strNEWLINE = 0;
STACKNODE* strTable = NULL;

const char* STR_OUT_OF_MEMORY = "out of memory";

void iprint(VALUE v){
	if(se_isArrayVal(v)) ar_dumpArray(v);
	else if(v.semType == SEM_REAL) printf("%lf", v.vdouble);
	else printf("%d", v.vint);
}

void iread(VARREC* v){
	char line[2048];
	if((v->value.semType != SEM_REAL) && (v->value.semType != SEM_INT)){
		puts("# sorry only int and real can be read");
		return;
	}
	printf("> %s = ", v->name);
	do{
		gets(line);
		if(line[2047] != '\0') puts("\nline too long");
		else break;
	} while (1);
	if(v->value.semType == SEM_REAL)
		v->value.vdouble = atof(line);
	else if(v->value.semType == SEM_INT)
		v->value.vint = atoi(line);
}

void iexit(){
	freeMemory();
	exit(7);
}

void freeMemory(){
	if(debug_mem) printf("\nD: Deallocating memory: ");
	if(debug_mem) printf("str ");
	str_destroyAll();
	if(debug_mem) printf("sym ");
	sy_destroyAll();
	if(debug_mem) printf("mem ");
	dm_destroyAll();
	if(debug_mem) printf("ast ");
	st_destroyAll();
	if(debug_mem) printf("Done!\n");
}

void fatal(const char *s){
	fprintf(stderr, "! Error: %s\n\n", s);
	if(lastNode != NULL) in_dumpEval(lastNode);
	if(debug_fatal){
		dm_dumpMemory();
		if(lastNode != NULL) {
			puts("\n--- exe tree ---\n");
			in_dumpSynTree(lastNode, 0);
		}
		puts("\n----------------");
	} else puts("\nType ana -? for more debug options.");
	freeMemory();
	exit(1);
}

char* fu_getFuncName(STNODE* fNode){
	SYMREC* fname;
	if(fNode == NULL) return NULL;
	fname = (SYMREC*)fNode->value.vint;
	return fname->name;
}

STNODE* fu_getFuncNode(char* name){
	SYMREC* fname;
	if(name == NULL) return NULL;
	if((fname = sy_getSym(name)) == NULL) return NULL;
	return (STNODE*)fname->vint;
};

STNODE* fu_patchFunctionNode(STNODE* fNode, STNODE *aNode, STNODE *bNode){
	SYMREC* fname;
	int i;
	if(fNode == NULL || bNode == NULL) return NULL;
	if((bNode->op == NULL)
		|| (bNode->op[0] == NULL)
		|| (bNode->op[0]->value.vint != O_INB)) yyerror("expected {");
	fname = (SYMREC*)fNode->value.vint;
	fNode->nops = 3 + bNode->nops;
	fNode->op = (STNODE**)malloc(sizeof(STNODE*) * fNode->nops);
	if(fNode->op == NULL) fatal(STR_OUT_OF_MEMORY);
	fNode->op[0] = bNode->op[0]; //INB
	fNode->op[1] = nodeDefVar(fname);
	fNode->op[1]->op[0]->value.semType = fNode->value.semType;
	fNode->op[2] = aNode;
	fNode->op[3] = fu_createAssignBlock(aNode);
	for(i = 1; i < bNode->nops; i++)
		fNode->op[i + 3] = bNode->op[i]; // last fnode->op is OUTB
	free(bNode->op);
	free(bNode);
	bNode->op = NULL;
	bNode = NULL;
	return fNode;
}

STNODE* fu_createAssignBlock(STNODE* aNode){
	STACKNODE* stack;
	STNODE* id;
	STNODE* n = aNode;
	int i, count = 0;
	if(n == NULL) return NULL;
	while(n->value.vint == ';'){
		count++;
		id = n->op[1]->op[0];
		id = createAssignNode((SYMREC*)id->value.vint, st_node(NODE_CON, 0, 0));
		id->value.vint = O_ASSF;
		if(sk_push(&stack, (void*)id) == 0) fatal(STR_OUT_OF_MEMORY);
		n = n->op[0];
	}
	count++;
	id = n->op[0];
	id = createAssignNode((SYMREC*)id->value.vint, st_node(NODE_CON, 0, 0));
	id->value.vint = O_ASSF;
	if(sk_push(&stack, (void*)id) == 0) fatal(STR_OUT_OF_MEMORY);
	n = st_node(NODE_OPR, ';', 0, 0);
	n->nops = count;
	n->op = (STNODE**)malloc(sizeof(STNODE*) * count);
	for(i = 0; i < count; i++){
		n->op[i] = (STNODE*)(sk_top(&stack)->data1);
		sk_pop(&stack);
	}
	return n;
}

STNODE* fu_activateFuncNode(STNODE* fNode, STNODE* eNode) {
	STACKNODE* evalStack, *t;
	VALUE* v;
	STNODE *nt, * n = fNode->op[3];
	int i = -1;
	const char* args = "number of args does not match";
	if(
		(n == NULL && eNode != NULL)
		||
		(n != NULL && eNode == NULL)
	) fatal(args);
	if(n == NULL) return fNode;
	evalStack = fu_evalExprList(eNode);
	for(t = evalStack; t != NULL; t = t->next){
		if(++i >= n->nops) fatal(args);
		v = (VALUE*)t->data1;
		if(v == NULL) fatal(args);
		nt = n->op[i]->op[1];
		memcpy(&(nt->value), v, sizeof(VALUE));
		free(v);
	}
	sk_destroyStack(&evalStack);
	if(debug_access){
		puts("ACTIVATION");
		in_dumpSynTree(n, 0);
	}
	return fNode;
}

STACKNODE* fu_evalExprList(STNODE* eNode){
	STNODE* n;
	VALUE v;
	STACKNODE* evalStack;
	if(eNode == NULL) return NULL;
	n = eNode;
	sk_init(&evalStack);
	while(n->value.vint == ';'){
		v = eval(n->op[1]);
		if(fu_stackVal(&evalStack, v) == 0) fatal(STR_OUT_OF_MEMORY);
		n = n->op[0];
	}
	v = eval(n);
	if(fu_stackVal(&evalStack, v) == 0) fatal(STR_OUT_OF_MEMORY);
	return evalStack;
}

int fu_stackVal(PSTACK stack, VALUE val){
	VALUE* v;
	static size_t size = sizeof(VALUE);
	v = (VALUE*)malloc(size);
	if(v == NULL) fatal(STR_OUT_OF_MEMORY);
	memcpy(v, &val, size);
	return sk_push(stack, (void*)v);
}

/* -------------------------------------------- */

int str_addStr(char * s){
	int i;
	if(strTable == NULL) sk_init(&strTable);
	i = str_findStr(s);
	if(i != 0) return i;
	if(sk_push(&strTable, (void*)s) == 0) fatal(STR_OUT_OF_MEMORY);
	return (int)s;
}

int str_getNEWLINE(){
	if(strNEWLINE == 0) return str_addStr("\n");
	return strNEWLINE;
}

int str_findStr(char* s){
	STACKNODE* t;
	for(t = strTable; t != NULL; t = t->next)
		if(strcmp(s, (char*)t->data1) == 0) return (int)t->data1;
	return 0;
}

void str_destroyAll(){
	STACKNODE* t;
	for(t = strTable; t != NULL; t = t->next)
		free(t->data1);
	sk_destroyStack(&strTable);
}

void str_dumpStrTable(){
	STACKNODE* t;
	puts("--- string table ---");
	for(t = strTable; t != NULL; t = t->next){
		if(strcmp((char*)t->data1, "\n") == 0) continue;
		printf(" %x \"%s\"\n", t->data1, (char*)t->data1);
	}
	puts("--------------------");
}

void str_iprint(char *s){
	char *p = s;
	char c;
	if(p == NULL) printf("?STR?");
	while((c = *(p++)) != '\0'){
		if(c == '\\') {
			char nc = *(p++);
			switch(nc){
				case 'n':	putchar('\n'); break;
				case 't':	putchar('\t'); break;
				case '\\':	putchar('\\'); break;
				case 'q':	putchar('\"'); break;
				case 'b':	putchar('\b'); break;
				case '\0':	return;
				default:	putchar('?'); break;
			}
		} else putchar(c);
	}
}