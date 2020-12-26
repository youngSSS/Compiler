/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"

#define SIZE 211

static ExpType curFuncType;
static int preserve = FALSE;
static int elseCheck = FALSE;
static char * scopeName;

/* counter for variable memory locations */
static int location = 0;

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ 
	if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}


void reverse(char s[]) {
	int i, j;
	char c;

	for (int i = 0, j = strlen(s) - 1; i < j; j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}


char * itoa(int value, char * buf, int radix) {
	char * p = buf;

	while (value) {
		if (radix <= 10) *p++ = (value % radix) + '0';
		else {
			if ((value % radix) <= 9) *p++ = (value % radix) + '0';
			else *p++ = (value % radix) - 10 + 'a';
		}
		value = value / radix;
	}
	*p = '\0';

	return buf;
}


char * getString(char *funcName, int lineno) {
	char * tag;
	char buf[100];

	tag = (char*)malloc(strlen(funcName));
	strcpy(tag, funcName);
	itoa(lineno, buf, 10);
	strcat(tag, buf);

	return tag;
}


static void symbolError(TreeNode * t, char * message)
{ fprintf(listing,"Symbol error line %d => %s\n",t->lineno,message);
  Error = TRUE;
}



/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{ 
	switch (t->nodekind)
  	{ 
	  	case StmtK: 
		    switch (t->kind.stmt)
		    { 
		    	case CompK: 
		        	t->scope = ss_top()->name;
		          	if (preserve) preserve = FALSE;
		          	else ss_push(scopeName, t->lineno);
		          	break;
		        
		        case IterK:
		          	if (t->child[1]->kind.stmt == CompK) 
		            	scopeName = getString("while:", t->lineno);
		          	break;
		        
		        case SelectK: 
		          	if (t->child[1]->kind.stmt == CompK) 
		            	scopeName = getString("if:", t->lineno);
		          	if (t->child[2] != NULL && t->child[2]->kind.stmt == CompK)  
		            	elseCheck = TRUE;
		          	break;
		        
		        default:
		          break;
		      }
		      break;
	    
	    case ExpK:
	      	switch (t->kind.exp)
	      	{ 
	      		case IdK: 
	          		if (ss_lookup(t->attr.name) != NULL)
	            		ss_add_l(t->attr.name, t->lineno, t);
	          		else 
	            		symbolError(t, "undefined variable reference");
	          		break;
	        
	        	case ArrIdK: 
	          		if (ss_lookup(t->attr.arr.name) != NULL) 
	            		ss_add_l(t->attr.arr.name, t->lineno, t);
	          		else 
	            		symbolError(t, "undefined array reference");
	          		break;
	        
	        	case CallK:
	          		if (ss_lookup(t->attr.name) != NULL) {
	            		ss_add_l(t->attr.name, t->lineno, t);
	            		TreeNode *arg = t->child[0];

		            	if (arg != NULL) { 
		              		if (ss_lookup(arg->attr.name) == NULL) 
		                		symbolError(t, "undefined variable argument");
		              		else 
		                		arg = arg->sibling;
		            	}
	          		} 
	          		else symbolError(t, "undefined function calling");
	          		break;
	        
	        	default:
	          		break;
	      	}
	      	break;
	    

	    case DeclK:
	      	switch (t->kind.decl)
	      	{ 
	      		case FuncK: 
	          		if (ss_lookup_no_p(t->attr.name) == NULL) {
	            		if (t->child[0]->attr.type == INT) t->type = Integer;
	            		else if (t->child[0]->attr.type == VOID) t->type = Void;
	            		ss_add_b(t->attr.name, t->type, t->lineno, t);
	            		t->scope = ss_top()->name;
	            		ss_push(t->attr.name, t->lineno);
	            		preserve = TRUE;
	          		} 
	          		else symbolError(t, "duplicate function name");
	          		break;
	        
	        	case VarK: 
	          		if (ss_lookup_no_p(t->attr.name) == NULL) {
	            		if (t->child[0]->attr.type == INT) t->type = Integer;
	            		else if (t->child[0]->attr.type == VOID) symbolError(t, "VOID type variable");
	            		ss_add_b(t->attr.name, t->type, t->lineno, t);
	            		t->scope = ss_top()->name;
	          		} 
	          		else symbolError(t, "duplicate variable name");
	          		break;
	        

	        	case ArrVarK:
	          		if (ss_lookup_no_p(t->attr.arr.name) == NULL) {
	            		if (t->child[0]->attr.type == INT) 
	              			t->type = IntegerArray;
	            		else if (t->child[0]->attr.type == VOID) 
	              			symbolError(t, "VOID type array variable");

	            		ss_add_b(t->attr.arr.name, t->type, t->lineno, t);
	            		t->scope = ss_top()->name;
	          		} 
	          		else symbolError(t, "duplicate array name");
	          		break;
	        
	        	default:
	          		break;
	      	}
	      	break;
    

    	case ParamK: 
	      	switch (t->kind.param) 
	      	{
	        	case SingleParamK: 
	          		if (t->attr.name != NULL) {
	            		if (t->child[0]->attr.type == INT) 
	              			t->type = Integer;
	            		if (ss_lookup_no_p(t->attr.name) == NULL) {
	              			ss_add_b(t->attr.name, t->type, t->lineno, t);
	              			t->scope = ss_top()->name;
	            		} 
	            		else 
	              			symbolError(t, "duplicate parameter name");
	          		}
	          		break;
	        
	        	case ArrParamK:
	          		if (t->attr.name != NULL) {
	            		if (t->child[0]->attr.type == INT) 
	             			t->type = IntegerArray;
	            		if (ss_lookup_no_p(t->attr.arr.name) == NULL) {
	              			ss_add_b(t->attr.arr.name, t->type, t->lineno, t);
	              			t->scope = ss_top()->name;
	            		} 
	            		else symbolError(t, "duplicate array parameter name");
	          		}
	          		break;
	        
	        	default:
	          		break;
	      	}
      		break;
    	
    	default:
      		break;
  	}

}


static void afterInsertNode(TreeNode *t) 
{ 
	switch (t->nodekind) 
	{ 
		case StmtK:
			switch (t->kind.stmt)
			{ 
				case CompK:
					st_insert();
					if (elseCheck) {
						ss_push(getString("else:", t->lineno), t->lineno);
						elseCheck = FALSE;
						preserve = TRUE;
					}
					break;
			}
			break;
	}
}


/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ 
	ss_init();
	traverse(syntaxTree,insertNode,afterInsertNode);
	st_insert();
	if (TraceAnalyze) printSymTab(listing);
}


static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}


static void beforeCheckNode(TreeNode * t)
{ 
	switch (t->nodekind)
  	{ 
  		case DeclK:
      		switch (t->kind.decl) {
  				case FuncK:
          			if (t->child[0]->attr.type == INT)
            			curFuncType = Integer;
          			else if (t->child[0]->attr.type == VOID)
            			curFuncType = Void;
          			break;
    			default:
          			break;
      		}
      		break;

    	default:
      		break;
	}
}


static void checkNode(TreeNode * t)
{ 
	TreeNode * expr;
	ExpType leftType, rightType;
	TokenType op;
	TreeNode * funcDecl;
  	TreeNode * arg;
  	TreeNode * param;

	switch (t->nodekind)
  	{ 
  		case StmtK:
      		switch (t->kind.stmt)
			{ 
				case IterK: 
					if (t->child[0]->type == Void)
						typeError(t->child[0],"no condition in while statement");
						break;

				case RetK: 
					
				expr = t->child[0];
				
				if (curFuncType == Void && (expr != NULL && expr->type != Void))
						typeError(t,"expected no return value");
					else if (curFuncType == Integer && (expr == NULL || expr->type != Integer))
						typeError(t,"expected integer return value");
					break;
					
				default:
					break;
			}
			break;

	    
	    case ExpK:
			switch (t->kind.exp)
			{ 
				case AssignK:
				  	if (t->child[0]->type == IntegerArray)
				    	typeError(t->child[0], "assignment to array variable");
				  	else if (t->child[1]->type == Void)
				    	typeError(t->child[0],"assignment of VOID value");
				  	break;
				
				case OpK: 
				  	

				  	if (t->child[0]->attr.type == INT) leftType = Integer;

				  	leftType = t->child[0]->type;
				  	rightType = t->child[1]->type;
				  	op = t->attr.op;

				  	if (leftType == Void || rightType == Void) 
				    	typeError(t,"two operands should have non-void type");
				  	else if (leftType != rightType)
				    	typeError(t, "type of two operands are different");
				  	else 
				    	t->type = Integer;
				  	break;

				case ConstK: 
				  	t->type = Integer;
				  	break;
				
				case IdK: 
				  	if (t->scope == NULL || st_lookup(t->scope, t->attr.name) == NULL) 
				    	typeError(t, "can't type check undefined variable");
				  	break;
				
				case ArrIdK:
				  	if (t->scope == NULL || st_lookup(t->scope, t->attr.name) == NULL) 
				    	typeError(t, "can't type check undefined arr variable");
				  	else if (t->child[0]->type != Integer)
				    	typeError(t, "index expression should have integer type");
				  	else 
				    	t->type = Integer;
				  	break;

				case CallK:
				  	if (t->scope == NULL || st_lookup(t->scope, t->attr.name) == NULL) {
				    	typeError(t, "can't type check undefined function");
				    	break;
				  	}

				  	funcDecl = st_lookup(t->scope, t->attr.name)->treeNode;
				  	arg = t->child[0];
				  	param = funcDecl->child[1];

				  	if (funcDecl->kind.decl != FuncK) { 
				    	typeError(t,"expected function symbol");
				    	break;
				  	}

				  	while (arg != NULL) { 
				    	if (param == NULL)
				      		typeError(arg,"Invalid parameters");
				    	else if(arg->type != param->type) 
				      		typeError(arg,"Invalid parameters");
				    	else {  // no problem!
				      		arg = arg->sibling;
				      		param = param->sibling;
				      		continue;
				    	}
				    	break;
				  	}

				  	if (arg == NULL && param != NULL)
				    	typeError(t,"Invalid parameters");
				  	break;

				default:
				  	break;
			}
	      	break;

	    default:
	      break;
	}

}


/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree, beforeCheckNode, checkNode);
}
