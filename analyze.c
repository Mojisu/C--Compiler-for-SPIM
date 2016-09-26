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

/* counter for variable memory locations */
static int location = 2;
static int prev_loc = 0;
static int temp_flag = 1;
static int pass_flag = 1;
static int main_flag = 0;
static int expr_flag = 0;
static int scope_number_for_type = 0;
static char *func_name = NULL;
static int return_flag = 0;
static int int_but_not = 0;
static unsigned char params[100][100]; //i번째 함수(params[i][0]은 함수 식별자)에 j번째 파라미터 0이면 int, 1이면 배열
static char* funcs[100];
static int ptr = -1;
static int ptr2[100];
static int cur[100];
static int prev_lineno;
static int array_flag = 0;
static int assign_flag = 0;
static int operator_cnt[100];
static int sp = -1;	//ptr2 stack pointer
static int is_pass1_error = 0; //1이면 에러 0이면 노에러
static int is_return = 0;
static int is_param = 0 ;

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
		void (* preProc) (TreeNode *),
		void (* postProc) (TreeNode *) )
{ 
	if (t != NULL) { 
		preProc(t);
		{ 
			int i;
			int flag = 0;
			if(t->nodekind == ExpK){
				if(t->kind.exp == AssignK){
					flag = 1;
				}
			}

			//+
			if(t->nodekind == ExpK){
				if(t->kind.exp == CallK){
					is_param = 1;
				}
			}


			for (i=0; i < MAXCHILDREN; i++)
				traverse(t->child[i],preProc,postProc);
			if(flag == 1)
				assign_flag = 0;
			if(array_flag == 1){
				array_flag = 0;
			}
		}

		if(pass_flag == 1){
			if(t->nodekind == StmtK){
				if(t->kind.stmt == CompoundK){
					scope_delete();
					location = prev_loc;
				}
			}
		}
		else {
			if(t->nodekind == StmtK){
				if(t->kind.stmt == ReturnK){
					is_return = 0;
				}
			}

			if(t->nodekind == ExpK){
				if(t->kind.exp == CallK){
					is_param = 0;
				}
			}
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


/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{ 
	int kind, array_size = 0;
	int s;

	switch (t->nodekind) { 
		case StmtK:
			switch (t -> kind.stmt) { 
				case CompoundK:
					if(temp_flag == 1){
						scope_insert();
						location = 0;
					}
					temp_flag = 1;
					break;
				case ExpressionK:
					break;
				case SelectionK:
					break;
				case IterationK:
					break;
				case ReturnK:
					break;
				default:
					break;
			}
			break;
		case ExpK:
			switch (t -> kind.exp) { 
				case OpK:
					break;
				case TypeK:
					break;
				case ConstK:
					break;
				case IdK:
					s = scope_check(t->attr.name, t->lineno);
					if(s == -1) {
						printf("SCOPE ERROR : It is not declared -----  name : %s, line number : %d\n\n", t->attr.name, t->lineno);
						is_pass1_error = 1;
					}
					else{
						st_insert(t->attr.name, t->lineno, 0, 0, 0, 0, 0, s, NULL);
					}
					break;
				case AssignK:
					break;
				case CallK:
					s = scope_check(t->attr.name, t->lineno);
					if(s == -1) {
						printf("SCOPE ERROR : It is not declared -----  name : %s, line number : %d\n\n", t->attr.name, t->lineno);
						is_pass1_error = 1;
					}
					else{
						st_insert(t->attr.name, t->lineno, 0, 0, 0, 0, 0, s, NULL);
					}


					break;
				case ArrayIdK:
					s = scope_check(t->attr.name, t->lineno);
					if(s == -1) {
						printf("SCOPE ERROR : It is not declared -----  name : %s, line number : %d\n\n", t->attr.name, t->lineno);
						is_pass1_error = 1;
					}
					else{
						st_insert(t->attr.name, t->lineno, 0, 0, 0, 0, 0, s, NULL);
					}
					break;
				default:
					break;
			}
			break;
		case DeclK:
			switch (t -> kind.decl) {
				case VarK:
					if(st_lookup(t -> attr.name) == -1) {
						if(t -> child[1] != NULL) array_size = t -> child[1] -> attr.val;
						st_insert(t -> attr.name, t -> lineno, location++, TYPE_VAR, t -> is_array, array_size, t -> child[0] -> exp, -1, func_name);
					}
					else {
						printf("SCOPE ERROR : It is already defined variable. ----- name : %s, lineno : %d\n\n", t->attr.name, t->lineno);
						is_pass1_error = 1;
					}
					break;
				case FuncK:
					if(st_lookup(t -> attr.name) == -1) {
						st_insert(t -> attr.name, t -> lineno, location++, TYPE_FUNC, FALSE, 0, t -> child[0] -> exp, -1, func_name);
					}
					else {	
						printf("SCOPE ERROR : It is already defined function. ----- name : %s, lineno : %d\n\n", t->attr.name, t->lineno);
						is_pass1_error = 1;
					}
					scope_insert();
					prev_loc = location;
					location = 0;
					temp_flag = 0;
					break;
				case ParaK:
					if(t -> child[0] -> exp == Void) {
						break;
					}
					if(st_lookup(t -> attr.name) == -1) {
						st_insert(t -> attr.name, t -> lineno, location++, TYPE_PARAM, FALSE, 0, t -> child[0] -> exp, -1, func_name);
					}
					else {
						printf("SCOPE ERROR : It is already defined parameter. ----- name : %s, lineno : %d\n\n", t->attr.name, t->lineno);
						is_pass1_error = 1;
					}
					break;
				case ArrayParaK:
					if(st_lookup(t -> attr.name) == -1) {
						if(t -> child[1] != NULL) array_size = t -> child[1] -> attr.val;
						st_insert(t -> attr.name, t -> lineno, location++, TYPE_PARAM, TRUE, array_size, t -> child[0] -> exp, -1, func_name);
					}
					else {
						printf("SCOPE ERROR : It is already defined parameter. ----- name : %s, lineno : %d\n\n", t->attr.name, t->lineno);
						is_pass1_error = 1;
					}
					break;
			}
			break;
		default:
			break;
	}
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree) 
{
	st_init();
	location = 2;

	fprintf(listing,"\nSymbol table:\n\n");
	traverse(syntaxTree,insertNode,nullProc);
	scope_delete();
	if (TraceAnalyze) { 
		printSymTab(listing);
	}

}

static void typeError(TreeNode * t, char * message)
{ 
	fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
	Error = TRUE;
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{
	int s, i, f_kind;
	char *l_name, *r_name;
	TreeNode *temp_node;

	if(sp!= -1 && cur[sp] != -1){
		if(!(t->nodekind == ExpK && (t->kind.exp == IdK || t->kind.exp == ArrayIdK || t->kind.exp == ConstK || t->kind.exp == OpK || t->kind.exp == CallK))){
			if(params[cur[sp]][ptr2[sp]] != 0){
	//			printf("SEMANTIC ERROR : There are too few arguments. ----- name : %s, line number : %d\n", funcs[cur[sp]], prev_lineno);
			}
			cur[sp] = -1;
			sp--;
		}
	}

	switch (t->nodekind) {
		case StmtK:
			assign_flag = 0;
			switch (t -> kind.stmt) {
				case CompoundK:
					expr_flag = 0;
					if(temp_flag != 0) {
						scope_number_for_type++;
					}
					temp_flag = 1;
					break;
				case ExpressionK:
					expr_flag = 0;
					break;
				case SelectionK:
					operator_cnt[sp] = 0;
					expr_flag = 1;
					break;
				case IterationK:
					operator_cnt[sp] = 0;
					expr_flag = 1;
					break;
				case ReturnK:
					if(t->child[0] == NULL && return_flag == 1) {
	//					printf("SEMANTIC ERROR : Return should exist. ----- name : %s, line number : %d\n", func_name, t -> lineno);
					}
					else if(return_flag == 0) {
	//					printf("SEMANTIC ERROR : Return should not exist. ----- name : %s, line number : %d\n", func_name, t -> lineno);
					}
					int_but_not = 0;
					expr_flag = 0;
					is_return = 1;
					break;
				default:
					break;
			}
			break;
		case ExpK:
			switch (t -> kind.exp) {
				case OpK:
					if(cur[sp]!=-1){
						operator_cnt[sp]++;
					}
					break;
				case TypeK:
					break;
				case ConstK:
					if(sp!= -1 && cur[sp] != -1){
						int flag = 0;
						int one_more = 0;
						while(1){
							if(params[cur[sp]][ptr2[sp]] == 0){
								flag = 1;
								if(sp==0){
									if(operator_cnt[-1] <= 1){
	//									printf("SEMANTIC ERROR : There are too many arguments. ----- name : %s, line number : %d\n", funcs[cur[sp]], t -> lineno);
										one_more = 0;
									}
								}
								else
									one_more = 1;
							}
							if(operator_cnt[sp] == 0){
								ptr2[sp]++;
							}
							else{
								operator_cnt[sp]--;
							}

							if(flag == 1) {
								sp--;
								ptr2[sp]++;
							}
							if(one_more != 1)
								break;
						}
					}
					prev_lineno = t -> lineno;
					break;
				case IdK:
					if(assign_flag && getType(t -> attr.name, scope_number_for_type) == Void) {
						if(is_param == 0){
	//						printf("SEMANTIC ERROR : Left and right side's type of assign must be interger. ----- name : %s, line number : %d\n", t -> attr.name, t -> lineno);
							}
					}
					if(array_flag == 0){
						if(sp!=-1 && cur[sp] != -1){
							while(1){
								int one_more = 0;
								int flag = 0;

								if(params[cur[sp]][ptr2[sp]] == 0){
									flag = 1;
									if(sp==0){
	//									printf("SEMANTIC ERROR : There are too many arguments. ----- name : %s, line number : %d\n", funcs[cur[sp]], t -> lineno);
										one_more = 0;
									}
									else
										one_more = 1;
								}

								if(flag == 0 && isArray(t -> attr.name, scope_number_for_type) == 0 && params[cur[sp]][ptr2[sp]] == 2){	
	//								printf("SEMANTIC ERROR : Function : %s argument %d must be an integer array. ----- name : %s, line number : %d\n", funcs[cur[sp]], ptr2[sp]+1, t->attr.name, t -> lineno);
									one_more = 0;
								}
								if(flag == 0 && isArray(t -> attr.name, scope_number_for_type) == 1 && params[cur[sp]][ptr2[sp]]==1){
	//								printf("SEMANTIC ERROR : Function : %s argument %d must be an integer variable. ----- name : %s, line number : %d\n", funcs[cur[sp]], ptr2[sp]+1, t->attr.name, t -> lineno);
									one_more = 0;
								}
								if(operator_cnt[sp]==0)
									ptr2[sp]++;
								else
									operator_cnt[sp]--;
								if(flag == 1){
									sp--;
									ptr2[sp]++;

								}
								if(one_more == 0) break;

							}
						}
					}
					else{
						if(sp!=-1 && cur[sp] != -1){
							if(operator_cnt[sp] == 0)
								ptr2[sp]++;
							else
								operator_cnt[sp]--;
						}
						if(isArray(t->attr.name, scope_number_for_type) == 1){

	//						printf("SEMANTIC ERROR : Array index must be integer. ----- name : %s, lineno : %d\n", t->attr.name, t->lineno);
						}

						if(operator_cnt[sp] == 0){
							array_flag = 0;
						}
						else
							operator_cnt[sp]--;
					}
					prev_lineno = t -> lineno;

					break;
				case AssignK:
					assign_flag = 1;
					if(getType(t -> child[0] -> attr.name, scope_number_for_type) == Void) {
						if(is_param == 0){
	//						printf("SEMANTIC ERROR : Left and right side's type of assign must be interger. ----- name : %s, line number : %d\n", t -> attr.name, t -> lineno);
							}
					}
					break;
				case CallK:
					if(expr_flag)
						if(isFunc(t -> attr.name, scope_number_for_type, &f_kind) == 1 && f_kind == Void)
	//						printf("SEMANTIC ERROR : Void value not ignored as it ought to be. ----- line number : %d\n", t -> lineno);

					if(!array_flag && sp == -1 && assign_flag && getType(t -> attr.name, scope_number_for_type) == Void) {
						if(is_param == 0){
	//						printf("SEMANTIC ERROR : Left and right side's type of assign must be interger. ----- name : %s, line number : %d\n", t -> attr.name, t -> lineno);
							}
					}
					if(isFunc(t -> attr.name, scope_number_for_type, &f_kind) != 1) {
	//					printf("SEMANTIC ERROR : It is not function. ----- name : %s, line number : %d\n", t -> attr.name, t -> lineno);
					}
					TreeNode *temp = t -> child[0];
					if(temp != NULL) {
						while(temp -> sibling != NULL) {
							temp = temp -> sibling;
						}
					}

					if(is_return == 1 || ((sp!= -1 && cur[sp]!=-1) && is_param == 1)) {
						if(isFunc(t->attr.name, scope_number_for_type, &f_kind) == 1 && f_kind == Void) {
	//						printf("SEMANTIC ERROR : Function : %s is not return Integer. ----- line number : %d\n", t->attr.name, t->lineno);
						}
						is_return = 0;
					}

					for(i=0;i<ptr;i++){
						if(strcmp(t->attr.name, funcs[i]) == 0){
							cur[++sp] = i;
						}
					}

					ptr2[sp] = 0;
					operator_cnt[sp] = 0;

					if(array_flag == 1){
						if(isFunc(t->attr.name, scope_number_for_type, &f_kind) == 1 && f_kind == Void){
	//						printf("SEMANTIC ERROR : Array index must be integer. ----- name : %s, lineno : %d\n", t->attr.name, t->lineno);
						}
					}
					break;
				case ArrayIdK:
					if(isArray(t -> attr.name, scope_number_for_type) == 0){
	//					printf("SEMANTIC ERROR : It is not array. ----- name : %s, line number : %d\n", t -> attr.name, t -> lineno);
					}
					else
						array_flag = 1;
					break;
				default:
					break;
			}
			break;
		case DeclK:
			assign_flag = 0;
			switch (t -> kind.decl) {
				case VarK:
					if(t -> child[0] -> exp == Void) {
	//					printf("SEMENTIC ERROR : Type of variable must be integer. ----- name : %s, lineno : %d\n", t->attr.name, t->lineno);
					}
					break;
				case FuncK:
					++ptr;
					ptr2[ptr] = 0;
					funcs[ptr] = t->attr.name;
					scope_number_for_type++;
					temp_flag = 0;
					if(strcmp(t -> attr.name, "main") == 0) { // for main
						main_flag = 1;
						if(t -> child[0] -> exp != Void) {
	//						printf("SEMANTIC ERROR : Return type of main must be void. ----- name : %s, line number : %d\n", t->attr.name, t -> lineno);
						}
						if(t -> child[1] -> child[0] -> is_param_void != TRUE) {
	//						printf("SEMANTIC ERROR : main must don't have parameter. ----- name : %s, line number : %d\n", t->attr.name, t -> lineno);
						}
					}
					else if(main_flag) {
	//					printf("SEMANTIC ERROR : main function must be declared in last. ----- name : %s, line number : %d\n", t-> attr.name, t -> lineno);
					}
					if(int_but_not == 1){
	//					printf("SEMANTIC ERROR : Return should exist. ----- name : %s\n", func_name);
						int_but_not = 0;
					}

					if(t -> child[0] -> exp == Integer) {
						return_flag = 1;
						int_but_not = 1;
					}
					else {
						return_flag = 0;
					}
					func_name = t -> attr.name;
					break;
				case ParaK:
					params[ptr][ptr2[ptr]++] = 1;
					if(t -> child[0] -> exp == Void && t -> child[0] -> is_param_void != TRUE) {
	//					printf("SEMENTIC ERROR : Type of variable must be integer. ----- name : %s, lineno : %d\n", t->attr.name, t->lineno);
					}
					break;
				case ArrayParaK:
					params[ptr][ptr2[ptr]++] = 2;
					if(t -> child[0] -> exp == Void && t -> child[0] -> is_param_void != TRUE) {
	//					printf("SEMENTIC ERROR : Type of variable must be integer. ----- name : %s, lineno : %d\n", t->attr.name, t->lineno);
					}
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
{
	pass_flag = 0;
	temp_flag = 1;
	int i=0;
	for(i=0;i<100;i++)
		cur[i] = -1;

	if(!is_pass1_error){
		traverse(syntaxTree, checkNode, nullProc);
		if(int_but_not == 1){
	//		printf("SEMANTIC ERROR : Return should exist. ----- name : %s\n", func_name);
		}
	}
}
