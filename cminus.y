/****************************************************/
/* File: cminus.y                                   */
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
static int savedLineNo;  /* ditto */
static int savedLineFuncNo;
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void); // added 11/2/11 to ensure no conflict with lex
static char *stackCall[100];
static char *savedFuncName;
static char *savedArrayName;
static char *savedVal;
static int top = 0;
static int callLineNo[100];

%}

%token ID NUM
%token IF ELSE INT RETURN VOID WHILE ASSIGN SEMI COMMA LPAREN RPAREN LSQUBRACKET RSQUBRACKET LCURLYBRACKET RCURLYBRACKET
%token PLUS MINUS MUL BACKSLASH
%token LT LTE GT GTE EQ NEQ
%token ERROR

%nonassoc ELSE_NO
%nonassoc ELSE

%% /* Grammar for C- */

program     : declaration_list
                 { savedTree = $1;} 
            ;
declaration_list    : declaration_list declaration
                 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; }
                     else $$ = $2;
                 }
            | declaration  { $$ = $1; }
            ;

declaration	: var_declaration{
				$$ = $1;
			}
			| fun_declaration{
				$$ = $1;
			}
			;

var_declaration : type_specifier ID{
					savedName = copyString(prevToken);
					savedLineNo = lineno;
				}
				SEMI{
					$$ = newDeclNode(VarK);
					$$->attr.name = savedName;
					$$->lineno = savedLineNo;
					$$->child[0] = $1;
					$$->is_array = 0;
				}
				| type_specifier ID{
					savedName = copyString (prevToken);
					savedLineNo = lineno;
				}
				LSQUBRACKET NUM {
					savedVal = copyString(tokenString);
				}
				RSQUBRACKET SEMI {
					$$ = newDeclNode(VarK);
					$$->attr.name = savedName;
					$$->lineno = savedLineNo;
					$$->child[0] = $1;
					$$->child[1] = newExpNode(ConstK);
					$$->child[1]->attr.val = atoi(savedVal);
					$$->is_array = 1;
				}

type_specifier : INT{
				$$ = newExpNode(TypeK);
				$$->exp = Integer;
			   }
			   | VOID
			   {
				$$ = newExpNode(TypeK);
				$$->exp = Void;
			   }

fun_declaration : type_specifier ID
				{
					savedFuncName = copyString(prevToken);
					savedLineFuncNo = lineno;
				}
				LPAREN params RPAREN compound_stmt
				{
					$$ = newDeclNode(FuncK);
					$$->lineno = savedLineFuncNo;
					$$->attr.name = savedFuncName;
					$$->child[0] = $1;
					$$->child[1] = $5;
					$$->child[2] = $7;
				}
				;

params : param_list
	   {
		$$ = $1;
	   }
	   | VOID
	   {
		$$ = newDeclNode(ParaK);
		$$->child[0] = newExpNode(TypeK);
		$$->child[0]->exp = Void;
		$$->child[0]->is_param_void = TRUE;
	   }
	   ;

param_list : param_list COMMA param
		   { YYSTYPE t = $1;
			if(t!=NULL){ while(t->sibling != NULL)
					t = t->sibling;
				t->sibling = $3;
				$$ = $1;
			}
			else $$ = $3;
		   }
		   | param
		   {
			$$ = $1;
		   }
		   ;

param : type_specifier ID
	  {
		savedName = copyString(prevToken);
		savedLineNo = lineno;
		$$ = newDeclNode(ParaK);
		$$->attr.name = savedName;
		$$->lineno = savedLineNo;
		$$->child[0] = $1;
	  }
	  | type_specifier ID
	  {
		savedName = copyString(prevToken);
		savedLineNo = lineno;
	  }
	  LSQUBRACKET RSQUBRACKET
	  {
		$$ = newDeclNode(ArrayParaK);
		$$->attr.name = savedName;
		$$->lineno = savedLineNo;
		$$->child[0] = $1;
	  }
	  ;

compound_stmt : LCURLYBRACKET local_declarations statement_list RCURLYBRACKET
			  {
				$$ = newStmtNode (CompoundK);
				$$->child[0] = $2;
				$$->child[1] = $3;
			  }
			  ;

local_declarations : local_declarations var_declaration
			  {
				YYSTYPE t = $1;
				if(t!=NULL){
					while(t->sibling != NULL)
						t = t->sibling;
					t->sibling = $2;
					$$ = $1;
				}
				else $$ = $2;
			  }
			  | {$$ = NULL;}
			  ;

statement_list : statement_list statement
			   { YYSTYPE t = $1;
				if(t!=NULL){
					while(t->sibling != NULL)
						t = t->sibling;
					t->sibling = $2;
					$$ = $1;
				}
				else $$ = $2;
			   }
			   | {$$ = NULL;}
			   ;

statement : expression_stmt
		  {
			$$ = $1;
		  }
		  | compound_stmt
		  {
			$$ = $1;
		  }
		  | selection_stmt
		  {
			$$ = $1;
		  }
		  | iteration_stmt
		  {
			$$ = $1;
		  }
		  | return_stmt
		  {
			$$ = $1;
		  }
		  ;

expression_stmt : expression SEMI
				{
					$$ = $1;
				}
				| SEMI
				{
					$$ = NULL;
				}
				;

selection_stmt : IF LPAREN expression RPAREN statement %prec ELSE_NO
			   {
				$$ = newStmtNode(SelectionK);
				$$->sel = IfK;
				$$->child[0] = $3;
				$$->child[1] = $5;
			   }
			   | IF LPAREN expression RPAREN statement ELSE statement
			   {
				$$ = newStmtNode(SelectionK);
				$$->sel = IfElseK;
				$$->child[0] = $3;
				$$->child[1] = $5;
				$$->child[2] = $7;
			   }
			   ;

iteration_stmt : WHILE LPAREN expression RPAREN statement
			   {
				$$ = newStmtNode(IterationK);
				$$-> child[0] = $3;
				$$->child[1] = $5;
			   }
			   ;

return_stmt : RETURN SEMI
			{
				$$ = newStmtNode(ReturnK);
			}
			| RETURN expression SEMI
			{
				$$ = newStmtNode(ReturnK);
				$$->child[0] = $2;
			}
			;

expression : var ASSIGN expression
		   {
			$$ = newExpNode(AssignK);
			$$->child[0] = $1;
			$$->child[1] = $3;
		   }
		   | simple_expression
		   {
			$$ = $1;
		   }
		   ;

var : ID
	{
		savedName = copyString(prevToken);
		savedLineNo = lineno;
		$$ = newExpNode(IdK);
		$$->attr.name = savedName;
		$$->lineno = savedLineNo;
		$$->is_array = 0;
	}
	| ID
	{
		savedArrayName = copyString(prevToken);
		savedLineNo = lineno;
	}
	LSQUBRACKET expression RSQUBRACKET
	{
		$$ = newExpNode(ArrayIdK);
		$$->attr.name = savedArrayName;
		$$->lineno = savedLineNo;
		$$->child[0] = $4;
		$$->is_array = 1;
	}
    ;

simple_expression : additive_expression relop additive_expression
				  {
					$$ = $2;
					$$->child[0] = $1;
					$$->child[1] = $3;
				  }
				  | additive_expression
				  {
					$$ = $1;
				  }
				  ;

relop : LT
	  {
		$$ = newExpNode(OpK);
		$$->attr.op = LT;
	  }
	  | LTE
	  {
		$$ = newExpNode(OpK);
		$$->attr.op = LTE;
	  }
	  | GT
	  {
		$$ = newExpNode(OpK);
		$$->attr.op = GT;
	  }
	  | GTE
	  {
		$$ = newExpNode(OpK);
		$$->attr.op = GTE;
	  }
	  | EQ
	  {
		$$ = newExpNode(OpK);
		$$->attr.op = EQ;
	  }
	  | NEQ
	  {
		$$ = newExpNode(OpK);
		$$->attr.op = NEQ;
	  }


additive_expression : additive_expression addop term
					{
						$$ = $2;
						$$->child[0] = $1;
						$$->child[1] = $3;
					}
					| term
					{
						$$ = $1;
					}
					;

addop : PLUS
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

term : term mulop factor
	 {
		$$ = $2;
		$$->child[0] = $1;
		$$->child[1] = $3;
	 }
	 | factor
	 {
		$$ = $1;
	 }
	 ;

mulop : MUL
	  {
		$$ = newExpNode(OpK);
		$$->attr.op = MUL;
	  }
	  | BACKSLASH
	  {
		$$ = newExpNode(OpK);
		$$->attr.op = BACKSLASH;
	  }
	  ;

factor : LPAREN expression RPAREN
	   {
		$$ = $2;
	   }
	   | var
	   {
		$$ = $1;
	   }
	   | call
	   {
		$$ = $1;
	   }
	   | NUM
	   {
		$$ = newExpNode(ConstK);
		$$->attr.val = atoi(tokenString);
	   }
	   ;

call : ID
	 {
		stackCall[++top] = copyString(prevToken);
		callLineNo[top] = lineno;
	 }
	LPAREN args RPAREN
	{
		$$ = newExpNode(CallK);
		$$->attr.name = stackCall[top];
		$$->lineno = callLineNo[top--];
		$$->child[0] = $4;
	}
	;

args : arg_list
	 {
		$$ = $1;
	 }
	 |{$$ = NULL;}
	 ;

arg_list : arg_list COMMA expression
		 { YYSTYPE t = $1;
			if(t != NULL){ while(t->sibling != NULL)
					t = t -> sibling;
				t->sibling = $3;
				$$ = $1;
			}
			else
				$$ = $3;
		 }
		 | expression
		 {
			$$ = $1;
		 }
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

