/* (c) Vasian CEPA 2002 */
#include "dmem.h"

VARREC* dm_create();
void dm_destroy(VARREC*);
VARREC* dm_seachRange(char*, VARREC*, VARREC*);
void dm_dump(VARREC*);

/* global */
int flagGlobal = 1;
VARREC* global = NULL;
STACKNODE* funcStack = NULL;

/* local */
VARREC *current = NULL;
STACKNODE *memory = NULL;

VARREC* dm_create(){
	return NULL;
}

void dm_destroy(VARREC *v){
	VARREC *temp;
	VARREC *next;
	if(v == NULL) return;
	next = v;
	for(temp = v; next != NULL; temp = next){
		next = temp->next;
		if(se_isLocalArrayVal(temp->value)){
			ar_deleteArray(temp->value.vint);
			temp->value.vint = 0;
		}
		free(temp->name);
		free(temp);
		temp = NULL;
	}
	v = NULL;
}

VARREC* dm_seachRange(char* name, VARREC* start, VARREC* end){
	VARREC *temp;
	if((name == NULL) || (start == NULL)) return NULL;
	for(temp = start; temp != end; temp = (VARREC*)temp->next)
		if(strcmp(temp->name, name) == 0) return temp;
	return NULL;
}

VARREC* dm_addVar(char* name){
	VARREC *temp;
	if(name == NULL) return 0;
	temp = (VARREC*)malloc(sizeof(VARREC));
	if(temp == NULL) return 0;
	temp->name = (char*)malloc(strlen(name) + 1);
	if(temp->name == NULL) {
		free(temp);
		return 0;
	}
	strcpy(temp->name, name);
	temp->value.vint = 0;
	temp->value.semType = SEM_UNDEF;
	temp->next = (VARREC*)current;
	current = temp;
	return temp;
}

VARREC* dm_getLocalVar(char* name){
	/* that is the (inner most) current block */
	return dm_seachRange(name, current, NULL);
}

VARREC* dm_getVar(char* name){
	VARREC *sym;
	STACKNODE* temp;
	if((sym = dm_getLocalVar(name)) != NULL) return sym;
	for(temp = memory; temp != NULL; temp = (STACKNODE*)temp->next){
		if((VARREC*)temp->data1 == current) continue;
		if((sym = dm_seachRange(name, (VARREC*)temp->data1, NULL))
			!= NULL) return sym;
	}
	return NULL;
}

void dm_destroyAll(){
	VARREC *v;
	STACKNODE* temp;
	dm_destroy(global);
	dm_destroy(current);
	current = NULL;
	for(temp = memory; temp != NULL; temp = (STACKNODE*)temp->next){
		v = (VARREC*)temp->data1;
		dm_destroy(v);
		v = NULL;
	}
	sk_destroyStack(&memory);
}

void dm_enterBlock(){
	if(flagGlobal){
		flagGlobal = 0;
		gm_init();
		global = current;
	} else {
		if(memory == NULL) sk_init(&memory);
		if(sk_push(&memory, (void*)current) == 0) fatal(STR_OUT_OF_MEMORY);
	}
	current = dm_create();
}


void dm_exitBlock(){
	STACKNODE* last = sk_top(&memory);
	dm_destroy(current);
	current = (last != NULL) ? (VARREC*)last->data1 : dm_create();
	sk_pop(&memory);
}

void dm_dumpMemory(){
	STACKNODE* temp;
	puts("\n--- memory dump ---");
	puts(" - local -");
	dm_dump(current);
	for(temp = memory; temp != NULL; temp = (STACKNODE*)temp->next){
		if((VARREC*)temp->data1 == current) continue;
		dm_dump((VARREC*)temp->data1);
	}
	puts(" - global -");
	dm_dump(global);
	puts(" - stack  -");
		dm_dumpStack();
	puts("-------------------");
}

void dm_dumpStack(){
	STACKNODE* temp, *temp1;
	STACKNODE* stack;
	for(temp = funcStack; temp != NULL; temp = (STACKNODE*)temp->next){
		stack = (STACKNODE*)temp->data1;
		if(stack == NULL) printf("| *\n");
		else {
			for(temp1 = stack; temp1 != NULL; temp1 = (STACKNODE*)temp1->next){
				if((VARREC*)temp1->data1 == current) continue;
				dm_dump((VARREC*)temp1->data1);
			}
		}
		puts("#");
	}
}

void dm_dump(VARREC *v){
	VARREC* t;
	static int tab = 0;
	int c = (current != NULL) && (v == current);
	tab = !tab;
	if(v == NULL){
		printf("%s %s\n", (tab ? "|" : "o"), "*");
		return;
	}
	for(t = v; t != NULL; t = t->next){
		printf("%s %6x%s   %-10s = ", (tab ? "|" : "o"),
			t, (c) ? "." : " ", t->name);
		se_dumpVal(t->value);
	}
}

/* ---------------------------------------- */

void gm_init(){
	sk_init(&funcStack);
}

void gm_enterFunc(){
	/* place dm current in (local) memory stack */
	dm_enterBlock();
	/* save current local memory */
	if(sk_push(&funcStack, (void*)memory) == 0) fatal(STR_OUT_OF_MEMORY);
	/* reset current memory */
	sk_init(&memory);
	/* enter new local memory; func call exec this */
	/* dm_enterBlock(); */
}

void gm_exitFunc(){
	/* leave current local memory; func call exec this */
	/* dm_exitBlock(); */
	/* reset current memory */
	sk_init(&memory);
	/* restore current local memory */
	memory = (STACKNODE*)(sk_top(&funcStack)->data1);
	sk_pop(&funcStack);
	/* retake current from (local) memory stack */
	dm_exitBlock();
}

VARREC* gm_getVar(char *name){
	/* name is block local */
	VARREC* v = dm_getVar(name);
	if(v != NULL) return v;
	/* or global */
	return dm_seachRange(name, global, NULL);
}