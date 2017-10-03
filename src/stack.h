/* (c) Vasian CEPA 2002 */
#ifndef ANA_STACK_H
#define ANA_STACK_H

#include <stdio.h>

typedef struct stack_elem {
	void* data1;
	struct stack_elem *next;
} STACKNODE;

#define PSTACK STACKNODE**

void sk_init(PSTACK);
int sk_push(PSTACK, void*);
STACKNODE* sk_top(PSTACK);
void sk_pop(PSTACK);
void sk_destroyStack(PSTACK);

#endif