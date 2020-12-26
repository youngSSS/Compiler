/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "symtab.h"
#include "globals.h"

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

static ScopeList symbolTable;
static ScopeList scopeStack;


BucketList make_BucketList(char *name, ExpType type, int lineno, int loc, TreeNode *t) {
	BucketList bucket_list;

	bucket_list = (BucketList)malloc(sizeof(struct BucketList));
	bucket_list->name = name;
	bucket_list->type = type;
	bucket_list->lines = (LineList) malloc(sizeof(struct LineList));
	bucket_list->lines->lineno = lineno;
	bucket_list->lines->next = NULL;
	bucket_list->memloc = loc;
	bucket_list->next = NULL;
	bucket_list->treeNode = t;

	return bucket_list;
}


ScopeList make_ScopeList(char *scope, int loc, int startLineNo, int endLineNo) {
	ScopeList scope_list;

	scope_list = (ScopeList)malloc(sizeof(struct ScopeList));
	scope_list->name = scope;
	scope_list->loc = loc;
	scope_list->startLineNo = startLineNo;
	scope_list->endLineNo = endLineNo;
	scope_list->parent = NULL;

	return scope_list;
}


void ss_init() {

	ss_push("global", 0);

	TreeNode *func;
	TreeNode *typeSpec;
	TreeNode *param;
	TreeNode *compStmt;

	func = newDeclNode(FuncK);
	func->type = Integer;

	typeSpec = newTypeNode(TypeK);
	typeSpec->attr.type = INT;

	compStmt = newStmtNode(CompK);
	compStmt->child[0] = NULL;   
	compStmt->child[1] = NULL;  

	func->lineno = 0;
	func->attr.name = "input";
	func->child[0] = typeSpec;
	func->child[1] = NULL;     
	func->child[2] = compStmt;
	func->scope = "global";

	ss_add_b("input", Integer, 0, func);

	func = newDeclNode(FuncK);
	func->type = Void;

	typeSpec = newTypeNode(TypeK);
	typeSpec->attr.type = VOID;

	param = newParamNode(SingleParamK);
	param->attr.name = "arg";
	param->type = Integer;
	param->child[0] = newTypeNode(TypeK);
	param->child[0]->attr.type = INT;

	compStmt = newStmtNode(CompK);
	compStmt->child[0] = NULL;  
	compStmt->child[1] = NULL;     

	func->lineno = 0;
	func->attr.name = "output";
	func->child[0] = typeSpec;
	func->child[1] = param;
	func->child[2] = compStmt;
	func->scope = "global";

	ss_add_b("output", Void, 0, func);

	scopeStack->loc = 2;
}


ScopeList ss_top() {
	return scopeStack;
}


void ss_push(char *scope, int startLineNo) {
	ScopeList scope_list;

	scope_list = (ScopeList)malloc(sizeof(struct ScopeList));
	scope_list->name = scope;
	scope_list->loc = 0;
	scope_list->startLineNo = startLineNo;
	scope_list->endLineNo = 0;

	scope_list->parent = scopeStack;
	scopeStack = scope_list;
}


ScopeList ss_pop() {
  ScopeList scope_list;

  scope_list = scopeStack;
  scopeStack = scopeStack->parent;

  return scope_list;
}


void ss_add_b(char *name, ExpType type, int lineno, TreeNode *t) {

	BucketList bucket_list;
	int h, loc;
	
	h = hash(name);
	loc = scopeStack->loc;

	bucket_list = make_BucketList(name, type, lineno, loc, t);
	
	scopeStack->loc++;

	bucket_list->next = scopeStack->bucket[h];
	scopeStack->bucket[h] = bucket_list;

}


void ss_add_l(char *name, int lineno, TreeNode *tn) {
	ScopeList scope_list;
	BucketList bucket_list;
	LineList line_list;
  	int h;

  	scope_list = scopeStack;
	h = hash(name);

	while (scope_list != NULL) {

		bucket_list = scope_list->bucket[h];

		while (bucket_list != NULL && strcmp(bucket_list->name,name) != 0) 
			bucket_list = bucket_list->next;

		if (bucket_list == NULL) 
			scope_list = scope_list->parent;

		else {
			line_list = bucket_list->lines;
			while(line_list->next != NULL) 
				line_list = line_list->next;
			line_list->next = (LineList)malloc(sizeof(struct LineList));
			line_list->next->lineno = lineno;
			line_list->next->next = NULL;
			tn->type = bucket_list->type;
			tn->scope = bucket_list->treeNode->scope;
			break;
		}

	}

}


BucketList ss_lookup(char *name) {
	ScopeList scope_list;
	BucketList bucket_list;
	int h;

	scope_list = scopeStack;
	h = hash(name);

	while (scope_list != NULL) {
		bucket_list = scope_list->bucket[h];
		while (bucket_list != NULL && strcmp(name,bucket_list->name) != 0) 
		  	bucket_list = bucket_list->next;
		if (bucket_list == NULL) 
			scope_list = scope_list->parent;
		else 
			return bucket_list;
	}

	return NULL;
}


BucketList ss_lookup_no_p(char *name) {
	BucketList bucket_list;
	int h;

	h = hash(name);
	bucket_list = scopeStack->bucket[h];

	while (bucket_list != NULL && strcmp(name,bucket_list->name) != 0)
		bucket_list = bucket_list->next;

	return bucket_list;
}


void st_insert() {
	ScopeList scope_list;

	scope_list = ss_pop();
	scope_list->parent = symbolTable;
	symbolTable = scope_list;
}


BucketList st_lookup(char *scope, char *name) {
  
	ScopeList scope_list;
	BucketList bucket_list;
	int h;

	scope_list = symbolTable;
	bucket_list = NULL;
	h = hash(name);

	while (scope_list != NULL && strcmp(scope, scope_list->name) != 0) 
		scope_list = scope_list->parent;

	while (scope_list != NULL) {

		bucket_list = scope_list->bucket[h];

		while (bucket_list != NULL && strcmp(name, bucket_list->name) != 0) 
			bucket_list = bucket_list->next;

		if (bucket_list == NULL) scope_list = scope_list->parent;
		else return bucket_list; 

	}

}

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing)
{ 
	ScopeList scope_list;
	BucketList bucket_list;
	LineList line_list;

	fprintf(listing,
		"\nSymbolTable :                                              \n"
		"<globals and function>                                       \n"
		"Variable Name |      Type      |  Location  |     Scope      \n"
		"-------------------------------------------------------------\n"
		);

	scope_list = symbolTable;\

	while (scope_list != NULL) {

		for (int i = 0; i < SIZE; ++i) {

			if (scope_list->bucket[i] != NULL) {

				bucket_list = scope_list->bucket[i];

				while (bucket_list != NULL && scope_list->name == "global") {

					line_list = bucket_list->lines;
					fprintf(listing,"%-14s| ",bucket_list->name);

					switch (bucket_list->type) {
						case Void:
							fprintf(listing,"%-15s| ","Void");
							break;
						case Integer:
							fprintf(listing,"%-15s| ","Integer");
							break;
						case IntegerArray:
							fprintf(listing, "%-15s| ","IntegerArray");
							break;
						default:
							break;
					}

					fprintf(listing,"%-11d| ",bucket_list->memloc);
					fprintf(listing,"%-14s| ",scope_list->name);
					fprintf(listing,"\n");

					bucket_list = bucket_list->next;
				}
			}

		}

		scope_list = scope_list->parent;
	}

	fprintf(listing,
		"-------------------------------------------------------------\n\n"
		"<function parmeter and local Variable>                       \n"
		"Variable Name |      Type      |  Location  |     Scope      \n"
		"-------------------------------------------------------------\n"
		);

	scope_list = symbolTable;

	while (scope_list != NULL) {

		for (int i = 0; i < SIZE; ++i) {

		  if (scope_list->bucket[i] != NULL) {

		    	bucket_list = scope_list->bucket[i];

		    	while (bucket_list != NULL && scope_list->name != "global") {

		      		line_list = bucket_list->lines;
		      		fprintf(listing,"%-14s| ",bucket_list->name);

			      	switch(bucket_list->type) {
			        	case Void:
			          		fprintf(listing,"%-15s| ","Void");
			          		break;
			        	case Integer:
			          		fprintf(listing,"%-15s| ","Integer");
			          		break;
			        	case IntegerArray:
			          		fprintf(listing, "%-15s| ","IntegerArray");
			          		break;
			        	default:
			          		break;
			      	}

		      		fprintf(listing,"%-11d| ",bucket_list->memloc);
		      		fprintf(listing,"%-14s| ",scope_list->name);
		      		fprintf(listing,"\n");
		      		
		      		bucket_list = bucket_list->next;
		    	}
		  	}
		}

		scope_list = scope_list->parent;
	}

	fprintf(listing,"-------------------------------------------------------------\n\n");
} /* printSymTab */
