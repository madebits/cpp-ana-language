/* (c) Vasian CEPA 2002, http://madebits.com */
#include "symtab.h"

SYMREC *symTable = NULL;

SYMREC* sy_putSym(char *name, SYMTYPE type){
	SYMREC *temp;
	if(name == NULL) return NULL;
	temp = (SYMREC*)malloc(sizeof(SYMREC));
	if(temp == NULL) return NULL;
	temp->name = (char*)malloc(strlen(name) + 1);
	if(temp->name == NULL) {
		free(temp);
		return NULL;
	}
	strcpy(temp->name, name);
	temp->type = type;
	temp->vint = 0;
	temp->next = symTable;
	symTable = temp;
	return symTable;
}

SYMREC* sy_getSym(char* name){
	SYMREC *temp;
	if(name == NULL) return NULL;
	for(temp = symTable; temp != NULL; temp = (SYMREC*)temp->next){
		if(strcmp(temp->name, name) == 0) return temp;
	}
	return NULL;
}

void sy_destroyAll(){
	SYMREC *temp;
	SYMREC *next = symTable;
	for(temp = symTable; next != NULL; temp = next){
		int type = temp->type;
		next = temp->next;
		if((type != SYM_KEYWD) || (type == SYM_IFUN)) break; /* continue; */
		/* using break since KW or IF are the static entries */
		free(temp->name);
		free(temp);
		temp = NULL;
	}
	symTable = NULL;
}


void sy_initSymTable(int size){
	int i;
	for(i = 1; i < size; i++){
		symInit[i].next = &symInit[i - 1];
	}
	symTable = &(symInit[i - 1]);
}

void sy_dumpSymTable(){
	SYMREC *temp;
	puts("--- symbol table ---");
	for(temp = symTable; temp != NULL; temp = (SYMREC*)temp->next){
		if((temp->type == SYM_VAR) || (temp->type == SYM_FUNC) || (temp->type == SYM_PRE))
			printf(" %x %-10s = [%d] (S%d)\n", temp, temp->name, temp->vint, temp->type);
	}
	puts("--------------------");
}