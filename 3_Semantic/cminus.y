/****************************************************/
/* File: tiny.y                                     */
/* The TINY Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static char * savedName; /* for use in assignments */
static int savedNumber;
static int savedLineNo;  /* ditto */
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void); // added 11/2/11 to ensure no conflict with lex

%}

/* Keywords */
%token IF ELSE WHILE RETURN INT VOID
/* ID & NUM */
%token ID NUM 
/* Special Symbols */
%token PLUS MINUS TIMES OVER GT GE LT LE EQ NE ASSIGN LPAREN RPAREN LBRACE RBRACE LCURLY RCURLY SEMI COMMA
%token ERROR 

%% /* Grammar for C-MINUS */

program             : declaration_list { savedTree = $1;} 
                    ;

declaration_list    : declaration_list declaration
                      { 
                        if ($1 != NULL)
                        { YYSTYPE node = $1;
                          while (node->sibling != NULL)
                            node = node->sibling;
                          node->sibling = $2;
                          $$ = $1;
                        }
                        else $$ = $2;
                      }
                    | declaration { $$ = $1; }
                    ;

declaration         : var_declaration { $$ = $1; }
                    | fun_declaration { $$ = $1; }
                    ;

identifier          : ID { savedName = copyString(tokenString); }
                    ;

number              : NUM { savedNumber = atoi(tokenString); }
                    ;

var_declaration     : type_specifier identifier SEMI
                      {
                        $$ = newDeclNode(VarK);
                        $$->attr.name = savedName;
                        $$->child[0] = $1;
                      }
                    | type_specifier identifier LBRACE number RBRACE SEMI
                      {
                        $$ = newDeclNode(ArrVarK);
                        $$->attr.arr.name = savedName;
                        $$->attr.arr.size = savedNumber;
                        $$->child[0] = $1;
                      }
                    ;

type_specifier      : INT 
                      { 
                        $$ = newTypeNode(TypeNameK);
                        $$->attr.type = INT;
                      } 
                    | VOID 
                      { 
                        $$ = newTypeNode(TypeNameK);
                        $$->attr.type = VOID;
                      }
                    ;

fun_declaration     : type_specifier identifier 
                      {
                        $$ = newDeclNode(FuncK);
                        $$->attr.name = savedName;
                      }
                      LPAREN params RPAREN compound_stmt
                      {
                        $$ = $3;
                        $$->child[0] = $1;
                        $$->child[1] = $5;
                        $$->child[2] = $7;
                      }
                    ;

params              : param_list { $$ = $1; } 
                    | param_void
                      {
                        $$ = newParamNode(SingleParamK);
                        $$->attr.name = NULL;
                        $$->child[0] = $1;
                      }
                    ;

param_void          : VOID 
                      {
                        $$ = newTypeNode(TypeNameK);
                        $$->attr.type = VOID;
                      }
                    ;            

param_list          : param_list COMMA param
                      {
                        if ($1 != NULL)
                        { YYSTYPE node = $1;
                          while (node->sibling != NULL)
                            node = node->sibling;
                          node->sibling = $3;
                          $$ = $1;
                        }
                        else $$ = $3;
                      }
                    | param { $$ = $1; }
                    ;

param               : type_specifier identifier
                      {
                        $$ = newParamNode(SingleParamK);
                        $$->attr.name = savedName;
                        $$->child[0] = $1;
                      }
                    | type_specifier identifier LBRACE RBRACE
                      {
                        $$ = newParamNode(ArrParamK);
                        $$->attr.arr.name = savedName;
                        $$->child[0] = $1;
                      }
                    ;

compound_stmt       : LCURLY local_declaration statement_list RCURLY
                      {
                        $$ = newStmtNode(CompK);
                        $$->child[0] = $2;
                        $$->child[1] = $3;
                      }
                    ;

local_declaration   : local_declaration var_declaration 
                      {
                        if ($1 != NULL)
                        { YYSTYPE node = $1;
                          while (node->sibling != NULL)
                            node = node->sibling;
                          node->sibling = $2;
                          $$ = $1;
                        }
                        else $$ = $2;
                      }
                    | { $$ = NULL; }
                    ;

statement_list      : statement_list statement
                      {
                        if ($1 != NULL)
                        { YYSTYPE node = $1;
                          while (node->sibling != NULL)
                            node = node->sibling;
                          node->sibling = $2;
                          $$ = $1;
                        }
                        else $$ = $2;
                      }
                    | { $$ = NULL; }
                    ;

statement           : expression_stmt { $$ = $1; }
                    | compound_stmt { $$ = $1; }
                    | selection_stmt { $$ = $1; }
                    | iteration_stmt { $$ = $1; }
                    | return_stmt { $$ = $1; }
                    ;

expression_stmt     : expression SEMI { $$ = $1; }
                    | SEMI { $$ = NULL; }
                    ;

selection_stmt      : IF LPAREN expression RPAREN statement 
                      {
                        $$ = newStmtNode(SelectK);
                        $$->child[0] = $3;
                        $$->child[1] = $5;
                      }
                    | IF LPAREN expression RPAREN statement ELSE statement
                      {
                        $$ = newStmtNode(SelectK);
                        $$->child[0] = $3;
                        $$->child[1] = $5;
                        $$->child[2] = $7;
                      }
                    ;

iteration_stmt      : WHILE LPAREN expression RPAREN statement
                      {
                        $$ = newStmtNode(IterK);
                        $$->child[0] = $3;
                        $$->child[1] = $5;
                      } 
                    ;

return_stmt         : RETURN SEMI { $$ = newStmtNode(RetK); }
                    | RETURN expression SEMI
                      {
                        $$ = newStmtNode(RetK);
                        $$->child[0] = $2;
                      }
                    ;

expression          : var ASSIGN expression 
                      {
                        $$ = newExpNode(AssignK);
                        $$->child[0] = $1;
                        $$->child[1] = $3;
                      }
                    | simple_expression { $$ = $1; }
                    ;

var                 : identifier
                      {
                        $$ = newExpNode(IdK);
                        $$->attr.name = savedName;
                      }
                    | identifier 
                      {
                        $$ = newExpNode(ArrIdK);
                        $$->attr.arr.name = savedName;
                      }
                      LBRACE expression RBRACE
                      {
                        $$ = $2;
                        $$->child[0] = $4;
                      }
                    ;

simple_expression   : additive_expression relop additive_expression 
                      {
                        $$ = $2;
                        $$->child[0] = $1;
                        $$->child[1] = $3;
                      }
                    | additive_expression { $$ = $1; }
                    ;

relop               : LT 
                      {
                        $$ = newExpNode(OpK);
                        $$->attr.op = LT;
                      }
                    | LE 
                      {
                        $$ = newExpNode(OpK);
                        $$->attr.op = LE;
                      }
                    | GT 
                      {
                        $$ = newExpNode(OpK);
                        $$->attr.op = GT;
                      }
                    | GE
                      {
                        $$ = newExpNode(OpK);
                        $$->attr.op = GE;
                      } 
                    | EQ 
                      {
                        $$ = newExpNode(OpK);
                        $$->attr.op = EQ;
                      }
                    | NE
                      {
                        $$ = newExpNode(OpK);
                        $$->attr.op = NE;
                      }
                    ;

additive_expression : additive_expression addop term 
                      {
                        $$ = $2;
                        $$->child[0] = $1;
                        $$->child[1] = $3;
                      }
                    | term { $$ = $1; }
                    ;

addop               : PLUS 
                      {
                        $$ = newExpNode(OpK);
                        $$->attr.op = PLUS;
                      }
                    | MINUS
                      {
                        $$ = newExpNode(OpK);
                        $$->attr.op = MINUS;
                      }
                    ;

term                : term mulop factor 
                      {
                        $$ = $2;
                        $$->child[0] = $1;
                        $$->child[1] = $3;
                      }
                    | factor { $$ = $1; }
                    ;

mulop               : TIMES 
                      {
                        $$ = newExpNode(OpK);
                        $$->attr.op = TIMES;
                      }
                    | OVER
                      {
                        $$ = newExpNode(OpK);
                        $$->attr.op = OVER;
                      }
                    ;

factor              : LPAREN expression RPAREN { $$ = $2; }
                    | var { $$ = $1; }
                    | call { $$ = $1; }
                    | NUM 
                      {
                        $$ = newExpNode(ConstK);
                        $$->attr.val = atoi(tokenString);
                      }
                    ;

call                : identifier
                      {
                        $$ = newExpNode(CallK);
                        $$->attr.name = savedName;
                      }
                      LPAREN args RPAREN
                      {
                        $$ = $2;
                        $$->child[0] = $4;
                      }
                    ;

args                : arg_list { $$ = $1; }
                    | { $$ = NULL; }
                    ;

arg_list            : arg_list COMMA expression 
                      {
                        if ($1 != NULL)
                        { YYSTYPE node = $1;
                          while (node->sibling != NULL)
                            node = node->sibling;
                          node->sibling = $3;
                          $$ = $1;
                        }
                        else $$ = $3;
                      }
                    | expression { $$ = $1; }
                    ;
%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

