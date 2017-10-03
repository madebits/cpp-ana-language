/* (c) Vasian CEPA 2002 */
#include "stack.h"

void sk_init(PSTACK sp){
	*sp = NULL;
	sp = NULL;
}

int sk_push(PSTACK sp, void* data1){
	STACKNODE *temp = NULL;
	if(sp == NULL) return 0;
	if((temp = (STACKNODE*)malloc(sizeof(STACKNODE))) == NULL) return 0;
	temp->data1 = data1;
	temp->next = *sp;
	*sp = temp;
	return 1;
}

STACKNODE* sk_top(PSTACK sp){
	if((sp == NULL) || (*sp == NULL)) return NULL;
	return (*sp);
}

void sk_pop(PSTACK sp){
	STACKNODE *temp;
	if((sp == NULL) || (*sp == NULL)) return;
	temp = (*sp)->next;
	free(*sp);
	*sp = temp;
}

void sk_destroyStack(PSTACK sp){
	if(sp == NULL) return;
	while((*sp) != NULL) sk_pop(sp);
	*sp = NULL;
}