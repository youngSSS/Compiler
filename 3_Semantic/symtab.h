/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#define SIZE 211

#include "globals.h"

typedef struct LineList { 
	int lineno;
	struct LineList * next;
} * LineList;

typedef struct BucketList { 
	char * name;
	ExpType type;
	LineList lines;
	TreeNode *treeNode;
	int memloc;
	struct BucketList * next;
} * BucketList;

typedef struct ScopeList { 
	char * name;
	int loc;
	int startLineNo;
	int endLineNo;
	BucketList bucket[SIZE];
	struct ScopeList * parent;
} * ScopeList;


void ss_init();

ScopeList ss_top();
void ss_push(char *scope, int startLineNo);
ScopeList ss_pop();

void ss_add_b(char *name, ExpType type, int lineno, TreeNode *t);
void ss_add_l(char *name, int lineno, TreeNode *tn);
BucketList ss_lookup(char *name);
BucketList ss_lookup_no_p(char *name);

void st_insert();
BucketList st_lookup(char *scope, char *name);


void printScopeStack(FILE * listing);
void printSymTab(FILE * listing);

#endif
