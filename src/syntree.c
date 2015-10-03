/* (c) Vasian CEPA 2002, http://madebits.com */
#include "syntree.h"

STACKNODE* st_mem = NULL;

void* st_malloc(size_t s){
	void *temp;
	if(st_mem == NULL) sk_init(&st_mem);
	temp = malloc(s);
	if(sk_push(&st_mem, temp) == 0) fatal(STR_OUT_OF_MEMORY);
	return temp;
}


STNODE* st_createEmptyNode(NODETYPE type){
	STNODE *temp = NULL;
	temp = (STNODE*)st_malloc(sizeof(STNODE));
	if(temp != NULL){
		temp->type = type;
		temp->value.semType = SEM_UNDEF;
		temp->value.vint = 0;
		temp->nops = 0;
		temp->op = NULL;
	} else fatal(STR_OUT_OF_MEMORY);
	return temp;
}

void st_destroyAll(){
	STNODE* n;
	STACKNODE *sn = NULL;
	while((sn = sk_top(&st_mem)) != NULL){
		if(sn->data1 != NULL) free(sn->data1);
		sk_pop(&st_mem);
	}
}

/*
void st_freeNode(STNODE *node){
	int i;
	if(node == NULL) return;
	if(node->op != NULL){
		for(i = 0; i < node->nops; i++){
			st_freeNode(node->op[i]);
		}
		free(node->op);
		node->op = NULL;
		node->nops = 0;
	}
	free(node);
	node = NULL;
}
*/

STNODE* st_node(NODETYPE type, int value, int nops, ...){
	STNODE **ops = NULL;
	STNODE *temp;
	va_list list;
	int i;
	temp = st_createEmptyNode(type);
	if(temp == NULL) return NULL;
	if(nops < 0) nops = 0;
	if(nops != 0){
		ops = (STNODE**)malloc(sizeof(STNODE*) * nops);
		if(ops == NULL)	fatal(STR_OUT_OF_MEMORY);
	}
	temp->value.vint = value;
	temp->nops = nops;
	temp->op = ops;
	if(nops != 0){
		va_start(list, nops);
		for(i = 0; i < nops; i++)
			temp->op[i] = va_arg(list, STNODE*);
		va_end(list);
	}
	return temp;
}