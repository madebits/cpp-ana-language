/* (c) Vasian CEPA 2002 */
#ifndef ANA_SYNTREE_H
#define ANA_SYNTREE_H

#include <stdio.h>
#include <malloc.h>
#include <stdarg.h>
#include "semtype.h"
#include "stack.h"

typedef enum{NODE_CON, NODE_ID, NODE_OPR, NODE_FUN, NODE_STR } NODETYPE;

typedef struct st_node {
	NODETYPE type;
	VALUE value;
	int nops;
	struct st_node **op; /* array of the children */
} STNODE;

STNODE* st_createEmptyNode(NODETYPE);
STNODE* st_node(NODETYPE, int, int, ...);
/* void st_freeNode(STNODE*); */
void st_destroyAll();
void* st_malloc(size_t);

extern STNODE *st_root;

extern void fatal(const char*);
extern const char* STR_OUT_OF_MEMORY;

#endif