/****************************************************/
/* File: util_cminus.c								*/
/* Utility function implementation                  */
/* for the C- compiler                              */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"
#include "cminus.tab.h"

/* Procedure printToken prints a token 
 * and its lexeme to the listing file
 */
void printToken( TokenType token, const char* tokenString )
{ switch (token)
  { case IF: fprintf(listing, "IF			if\n"); break;
    case ELSE: fprintf(listing,"ELSE			else\n"); break;
	case INT: fprintf(listing, "INT			int\n"); break;
	case RETURN: fprintf(listing, "RETURN			return\n"); break;
	case VOID: fprintf(listing, "VOID			void \n"); break;
	case WHILE: fprintf(listing, "WHILE			while \n"); break;
	case PLUS: fprintf(listing, "PLUS\n"); break;
	case MINUS: fprintf(listing, "MINUS\n"); break;
	case MUL: fprintf(listing, "MUL\n"); break;
	case BACKSLASH: fprintf(listing, "BACKSLASH\n"); break;
	case LT: fprintf(listing, "LT\n"); break;
	case LTE: fprintf(listing, "LTE\n"); break;
	case GT: fprintf(listing, "GT\n"); break;
	case GTE: fprintf(listing, "GTE\n"); break;
	case EQ: fprintf(listing, "EQ\n"); break;
	case NEQ: fprintf(listing, "NEQ\n"); break;
	case ASSIGN: fprintf(listing, "=			=\n"); break;
	case SEMI: fprintf(listing, ";			;\n"); break;
	case COMMA: fprintf(listing, ",			,\n"); break;
	case LPAREN: fprintf(listing, "(			(\n"); break;
	case RPAREN: fprintf(listing, ")			)\n"); break;
    case LSQUBRACKET: fprintf(listing,"[			[\n"); break;
    case RSQUBRACKET: fprintf(listing,"]			]\n"); break;
    case LCURLYBRACKET: fprintf(listing,"{			{\n"); break;
    case RCURLYBRACKET: fprintf(listing,"}			}\n"); break;
    case ENDFILE: fprintf(listing,"EOF\n"); break;
    case NUM: fprintf(listing,"NUM			%s\n",tokenString); break;
    case ID: fprintf(listing,"ID			%s\n",tokenString); break;
    case ERROR: fprintf(listing,"ERROR:			%s\n", tokenString); break;
    default: /* should never happen */ 
      fprintf(listing,"Unknown token: %d\n",token);
  }
}

TreeNode* newDeclNode(DeclKind kind){
	TreeNode *t = (TreeNode*)malloc(sizeof(TreeNode));
	int i;
	if(t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else{
		for(i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = DeclK;
		t->kind.decl = kind;
		t->lineno = lineno;
	}
	return t;
}

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode * newStmtNode(StmtKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = StmtK;
    t->kind.stmt = kind;
    t->lineno = lineno;
  }
  return t;
}

/* Function newExpNode creates a new expression 
 * node for syntax tree construction
 */
TreeNode * newExpNode(ExpKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ExpK;
    t->kind.exp = kind;
    t->lineno = lineno;
    t->exp = Void;
  }
  return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char * copyString(char * s)
{ int n;
  char * t;
  if (s==NULL) return NULL;
  n = strlen(s)+1;
  t = malloc(n);
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else strcpy(t,s);
  return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno+=2
#define UNINDENT indentno-=2

/* printSpaces indents by printing spaces */
static void printSpaces(void)
{ int i;
  for (i=0;i<indentno;i++)
    fprintf(listing," ");
}

/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 */
void printTree( TreeNode * tree )
{ int i;
  INDENT;
  while (tree != NULL) {
    printSpaces();
    if (tree->nodekind==StmtK)
    { 
		switch (tree->kind.stmt) 
		{
			case ExpressionK: fprintf(listing, "Expression\n"); break;
			case SelectionK:if(tree->sel == IfK) fprintf(listing, "If\n");
								else fprintf(listing, "If-else\n"); break;
			case IterationK: fprintf(listing, "Iteration\n"); break;
			case CompoundK: fprintf(listing, "Compound statement\n"); break;
			case ReturnK: fprintf(listing, "Return\n"); break;
        default:
          fprintf(listing,"Unknown ExpNode kind\n");
          break;
      }
    }
    else if (tree->nodekind==ExpK)
    { 
		switch (tree->kind.exp) 
		{
			case OpK:
				fprintf(listing, "Op: ");
					printToken(tree->attr.op, "\0");
				break;
			case ConstK: fprintf(listing, "Const : %d\n", tree->attr.val); break;
			case IdK: fprintf(listing, "Id : %s\n", tree->attr.name); break;
			case ArrayIdK: fprintf(listing, "Array Id: %s\n", tree->attr.name); break;
			case TypeK: fprintf(listing, "Type: "); if(tree->exp == Void) fprintf(listing, "void\n"); else fprintf(listing,  "int\n"); break;
			case CallK: fprintf(listing, "Call : %s\n", tree->attr.name); break;
			case AssignK: fprintf(listing, "Assign to %s\n", tree->child[0]->attr.name); break;
        default:
          fprintf(listing,"Unknown ExpNode kind\n");
          break;
      }
    }
	else if(tree -> nodekind == DeclK){
		switch(tree->kind.exp){
			case VarK: fprintf(listing, "Variable Id : %s", tree->attr.name); break;
			case FuncK: fprintf(listing, "Function Id: %s", tree->attr.name); break;
			case ParaK: if(tree->child[0]->exp == Integer) fprintf(listing, "Parameter : %s", tree->attr.name);
						else fprintf(listing, "Parameter "); 
							break;
			case ArrayParaK: fprintf(listing, "Array Parameter : %s", tree->attr.name); break;
			default: fprintf(listing, "Unknown DeclNode kind\n"); break;
		}
	}
    else fprintf(listing,"Unknown node kind\n");
    for (i=0;i<MAXCHILDREN;i++)
         printTree(tree->child[i]);
    tree = tree->sibling;
  }
  UNINDENT;
}
