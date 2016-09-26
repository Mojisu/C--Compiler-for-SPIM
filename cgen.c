/****************************************************/
/* File: cgen.c                                     */
/* The code generator implementation                */
/* for the TINY compiler                            */
/* (generates code for the TM machine)              */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"

/* tmpOffset is the memory offset for temps
   It is decremented each time a temp is
   stored, and incremeted when loaded again
*/
static int label_num = 0;
static int flag = 1;
static int scope_number_for_type = 0;
static int base = 0;
static int param_cnt[100];
static int top_param_cnt = 0;
static int top = 0;
static int return_flag = 0;
static int param_flag = 0;

//PLUS MINUS MUL BACKSLASH
//LT LTE GT GTE EQ NEQ


void pop_stack(int num) {
	char str[5];
	sprintf(str, "$t%d", num);
	emitRM("lw", str, 0, "$sp");
	emitRO("addiu", "$sp", "$sp", "4");
	top--;
}
void push_stack(void) {
	emitRO("subu", "$sp", "$sp", "4");
	emitRM("sw", "$t0", 0, "$sp");
	top++;
}

/* prototype for internal recursive code generator */
static void cGen (TreeNode * tree, int is_addr);

/* Procedure genStmt generates code at a statement node */
static void genStmt( TreeNode * tree, int is_addr)
{ 
	TreeNode * p1, * p2, * p3;
	int savedLoc1,savedLoc2,currentLoc;
	int loc;
	char label1[100], label2[100];
	switch (tree->kind.stmt) {
		case CompoundK:
			if(flag) scope_number_for_type++;
			flag = 1;
			p1 = tree -> child[0];
			p2 = tree -> child[1];
			cGen(p1, is_addr);
			cGen(p2, is_addr);
			break;
		case ExpressionK:
			break;
		case SelectionK:
			p1 = tree -> child[0];
			p2 = tree -> child[1];
			if(tree -> sel == IfK) {
				cGen(p1, is_addr);
				sprintf(label1, "L%d", label_num++);
				pop_stack(0);
				emit2("beqz", "$t0", label1); // if v0 is zero, then jump
				cGen(p2, is_addr);
				fprintf(code, "%s:\n", label1);
			}
			else {
				cGen(p1, is_addr);
				sprintf(label1, "L%d", label_num++);
				pop_stack(0);
				emit2("beqz", "$t0", label1);
				cGen(p2, is_addr);
				sprintf(label2, "L%d", label_num++);
				emit1("b", label2);
				fprintf(code, "%s:\n", label1);
				p3 = tree -> child[2];
				cGen(p3, is_addr);
				fprintf(code, "%s:\n", label2);
			}
			break;
		case IterationK:
			p1 = tree -> child[0];
			p2 = tree -> child[1];
			sprintf(label1, "L%d", label_num++);
			sprintf(label2, "L%d", label_num++);
			fprintf(code, "%s:\n", label1);
			cGen(p1, is_addr);
			pop_stack(0);
			emit2("beqz", "$t0", label2);
			cGen(p2, is_addr);
			fprintf(code, "b %s\n", label1);
			fprintf(code, "%s:\n", label2);
			break;
		case ReturnK:
			p1 = tree -> child[0];
			cGen(p1, is_addr);
			pop_stack(2); // result in t2
			return_flag = 1;
			break;
		default:
			break;
	}
} /* genStmt */
/* Procedure genExp generates code at an expression node */
static void genExp( TreeNode * tree, int is_addr)
{ 
	int loc, cnt;
	TreeNode * p1, * p2;
	BucketList l;
	switch (tree->kind.exp) {
		case OpK:
			cGen(tree -> child[0], is_addr);
			cGen(tree -> child[1], is_addr);
			pop_stack(0); // t0 is right value
			pop_stack(1); // t1 is left value
			switch(tree -> attr.op) {
				case PLUS:
					fprintf(code, "addu $t0, $t0, $t1\n");
					break;
				case MINUS:
					fprintf(code, "subu $t0, $t1, $t0\n");
					break;
				case MUL:
					fprintf(code, "mul $t0, $t0, $t1\n");
					break;
				case BACKSLASH:
					fprintf(code, "div $t0, $t1, $t0\n");
					break;
				case LT:
					fprintf(code, "slt $t0, $t1, $t0\n");
					break;
				case LTE:
					fprintf(code, "sle $t0, $t1, $t0\n");
					break;
				case GT:
					fprintf(code, "sgt $t0, $t1, $t0\n");
					break;
				case GTE:
					fprintf(code, "sge $t0, $t1, $t0\n");
					break;
				case EQ:
					fprintf(code, "seq $t0, $t1, $t0\n");
					break;
				case NEQ:
					fprintf(code, "sne $t0, $t1, $t0\n");
					break;
			}
			push_stack();
			break;
		case TypeK:
			break;
		case ConstK:
			fprintf(code, "li $t0, %d\n", tree -> attr.val); 
			push_stack();
			break;
		case IdK:
			l = getBucket(tree -> attr.name, scope_number_for_type);


			if(l->is_array){
				if(param_flag){
					if(l -> scope == 0){ //global
						fprintf(code, "la $t0, %s\n", tree -> attr.name);
						push_stack();
					}
					else{ //local
						ScopeList *sl = getScopeList(tree -> attr.name, scope_number_for_type);
						base = calBase(tree -> attr.name, sl->scope_for_type);
						loc = l -> memloc - param_cnt[top_param_cnt] + base + 1;
						//첫번째 param 위치를 찾아서 이 idk의 주소 저장
						fprintf(code, "la $t0, %d($fp)\n", -4*(l->array_size + l->memloc + base));
						push_stack();
					}
				}
			}
			else{
			if(is_addr) {
				if(l -> scope == 0) {
					fprintf(code, "la $t0, %s\n", tree -> attr.name);
				}
				else {
					if(l -> kind == 2) { // param
						loc = param_cnt[top_param_cnt-1] - 1 + 4 - l -> memloc;
					}
					else {
						ScopeList * sl = getScopeList(tree -> attr.name, scope_number_for_type);
						base = calBase(tree -> attr.name, sl->scope_for_type);
						loc = l -> memloc - param_cnt[top_param_cnt] + base + 1;
						loc = -loc;
					}
					fprintf(code, "la $t0, %d($fp)\n", loc*4);
				}
			}
			else {
				if(l -> scope == 0) {
					fprintf(code, "lw $t0, %s\n", tree -> attr.name);
				}
				else {
					if(l -> kind == 2) {
						loc = param_cnt[top_param_cnt-1] - 1 + 4 - l -> memloc;
					}
					else {
						ScopeList * sl = getScopeList(tree -> attr.name, scope_number_for_type);
						base = calBase(tree -> attr.name, sl->scope_for_type);
						loc = l -> memloc - param_cnt[top_param_cnt] + base + 1;
						loc = -loc;
					}
					fprintf(code, "lw $t0, %d($fp)\n", loc*4);
				}
			}
			push_stack();
			}
			break;
		case AssignK:
			p1 = tree -> child[0];
			p2 = tree -> child[1];
			cGen(p1, 1);
			cGen(p2, 0);
			pop_stack(0); // result in t0(val)
			pop_stack(1); // result in t1(addr)
			fprintf(code, "sw $t0, 0($t1)\n");
			if(top != 0) {
				push_stack();
			}
			break;
		case CallK:
			param_flag = 1; //이제 부터 들어가는 idk는 파라미터다
			if(tree -> child[0] != NULL) {
				cGen(tree -> child[0], is_addr);
			}
			param_flag = 0; // 파라미터 끝
			emit1("jal", tree -> attr.name);
			l = getBucket(tree -> attr.name, scope_number_for_type);
			if(l -> type == Integer) {
				fprintf(code, "move $t0, $t2\n");
				push_stack();
				return_flag = 0;
			}
			break;
		case ArrayIdK:
			l = getBucket(tree -> attr.name, scope_number_for_type);

			if(is_addr) {
				if(l -> scope == 0) {
					cGen(tree->child[0], 0);
					pop_stack(1);
					fprintf(code, "mul $t1, $t1, 4\n");
					fprintf(code, "la $t0, %s\n", tree -> attr.name);
					fprintf(code, "addu $t0, $t0, $t1\n");
					fprintf(code, "la $t0, 0($t0)\n");
				}
				else {
					if(l -> kind == 2) { // param_cnt
						loc = param_cnt[top_param_cnt-1] - 1 + 4 - l -> memloc;
						cGen(tree -> child[0], 0);
						pop_stack(1);
						fprintf(code, "mul $t1, $t1, 4\n");
						fprintf(code, "lw $t0, %d($fp)\n", 4*loc);
						fprintf(code, "addu $t0, $t0, $t1\n");					
						fprintf(code, "la $t0, 0($t0)\n");	
					}
					else {
						ScopeList *sl = getScopeList(tree -> attr.name, scope_number_for_type);
						base = calBase(tree -> attr.name, sl->scope_for_type);
						loc = l -> memloc - param_cnt[top_param_cnt] + base + 1;
						loc = -loc;
						loc = loc - l->array_size + 1;

						cGen(tree -> child[0], 0);
						pop_stack(1);
						fprintf(code, "mul $t1, $t1, 4\n");
						fprintf(code, "la $t0, %d($fp)\n", loc*4);
						fprintf(code, "addu $t0, $t0, $t1\n");
						fprintf(code, "la $t0, 0($t0)\n");
					}
				}
			}
		else {
				if(l -> scope == 0) {
					cGen(tree->child[0], 0);
					pop_stack(1);
					fprintf(code, "mul $t1, $t1, 4\n");
					fprintf(code, "la $t0, %s\n", tree -> attr.name);
					fprintf(code, "addu $t0, $t0, $t1\n");
					fprintf(code, "lw $t0, 0($t0)\n");
				}
				else {
					if(l -> kind == 2) {
						loc = param_cnt[top_param_cnt-1] - 1 + 4 - l -> memloc;
						cGen(tree -> child[0], 0);
						pop_stack(1);
						fprintf(code, "mul $t1, $t1, 4\n");
						fprintf(code, "lw $t0, %d($fp)\n", 4*loc);
						fprintf(code, "addu $t0, $t0, $t1\n");
						fprintf(code, "lw $t0, 0($t0)\n");
					}
					else {
						ScopeList *sl = getScopeList(tree -> attr.name, scope_number_for_type);
						base = calBase(tree -> attr.name, sl->scope_for_type);
						loc = l -> memloc - param_cnt[top_param_cnt] + base + 1 ; 
						loc = -loc;
						loc = loc - l->array_size + 1;
						cGen(tree -> child[0], 0);
						pop_stack(1);
						fprintf(code, "mul $t1, $t1, 4\n");
						fprintf(code, "la $t0, %d($fp)\n", loc*4);
						fprintf(code, "addu $t0, $t0, $t1\n");
						fprintf(code, "lw $t0, 0($t0)\n");	

					}
				}
			}
			push_stack();
			break;
	}
} /* genExp */

static void genDecl( TreeNode *tree, int is_addr)
{
	TreeNode *p1, *p2;
	int loc;
	switch (tree -> kind.decl) {
		case VarK:
			if(tree->is_array == 0){
				emitRO("subu", "$sp", "$sp", "4");
			}
			else{
				cGen(tree->child[1], is_addr);
				char temp[11];
				sprintf(temp, "%d", 4*tree->child[1]->attr.val);
				emitRO("subu", "$sp", "$sp", temp);
			}
			break;
		case FuncK:
			p1 = tree -> child[2];
			scope_number_for_type++;
			flag = 0;

			TreeNode *temp = tree -> child[1];
			int	cnt = 0;
			while(temp != NULL) { // count number of parameter
				cnt++;
				temp = temp -> sibling;
			}
			param_cnt[top_param_cnt++] = cnt;

			// save
			fprintf(code, "%s: \n", tree -> attr.name);
			fprintf(code, "subu $sp, $sp, 16\n");
			fprintf(code, "sw $t1, 12($sp)\n");
			fprintf(code, "sw $t0, 8($sp)\n");
			fprintf(code, "sw $fp, 4($sp)\n"); // control link
			fprintf(code, "sw $ra, 0($sp)\n"); // return address
			fprintf(code, "move $fp, $sp\n");

			cGen(p1, is_addr);

			// restore
			fprintf(code, "lw $ra, 0($fp)\n");
			fprintf(code, "lw $t0, 8($fp)\n");
			fprintf(code, "lw $t1, 12($fp)\n");
			fprintf(code, "move $sp, $fp\n");
			fprintf(code, "lw $fp, 4($fp)\n");
			fprintf(code, "addu $sp, $sp, %d\n", 16+param_cnt[top_param_cnt-1]*4);
			fprintf(code, "jr $ra\n");
			top_param_cnt--;
			break;
		case ParaK:
			break;
		case ArrayParaK:
			break;
	}
}
/* Procedure cGen recursively generates code by
 * tree traversal
 */
static void cGen( TreeNode * tree, int is_addr)
{ 
	if (tree != NULL) { 
		switch (tree->nodekind) {
			case StmtK:
				genStmt(tree, is_addr);
				break;
			case ExpK:
				genExp(tree, is_addr);
				break;
			case DeclK:
				genDecl(tree, is_addr);
				break;
		}
		cGen(tree->sibling, is_addr);
	}
}

/**********************************************/
/* the primary function of the code generator */
/**********************************************/
/* Procedure codeGen generates code to a code
 * file by traversal of the syntax tree. The
 * second parameter (codefile) is the file name
 * of the code file, and is used to print the
 * file name as a comment in the code file
 */
void codeGen(TreeNode * syntaxTree, char * codefile)
{  char * s = malloc(strlen(codefile)+7);
	int i;
	strcpy(s,"File: ");
	strcat(s,codefile);
	fprintf(code, "\t.data\n");
	fprintf(code, "new: .asciiz \"\\n\"\n");

	ScopeList *target = getScopeTypeHead()->next;

	//0번째 즉 맨끝 찾기

	while(target->scope != 0){
		target = target ->next;
	}

	//hash 순회 하며 넣기
	for(i=0; i < SIZE ; i++){
		if(target -> hashTable[i] != NULL)	{
			BucketList l = target -> hashTable[i];
			while(l != NULL){
				if(l->kind == 0){ //vark
					if(l->is_array == 0){ //not array
						fprintf(code, "%s:  .word 0\n", l->name);
					}
					else{ //array
						fprintf(code, "%s:  .word 0:%d\n", l->name, l->array_size);	
					}	
				}
				l = l->next;
			}
		}
	}
	
	

	fprintf(code, "\t.text\n");
	fprintf(code, "\t.globl main\n");


	// output
	fprintf(code, "output:\n");
	fprintf(code, "subu $sp, $sp, 16\n");
	fprintf(code, "sw $t1, 12($sp)\n");
	fprintf(code, "sw $t0, 8($sp)\n");
	fprintf(code, "sw $fp, 4($sp)\n"); // control link
	fprintf(code, "sw $ra, 0($sp)\n"); // return address
	fprintf(code, "move $fp, $sp\n");

	fprintf(code, "li $v0, 1\n");
	fprintf(code, "lw $t0, 16($fp)\n");
	fprintf(code, "move $a0, $t0\n");
	fprintf(code, "syscall\n");
	fprintf(code, "li $v0, 4\n");
	fprintf(code, "la $a0, new\n");
	fprintf(code, "syscall\n");
	
	// restore
	fprintf(code, "move $sp, $fp\n");
	fprintf(code, "addu $sp, $sp, %d\n", 20);
	fprintf(code, "lw $ra, 0($fp)\n");
	fprintf(code, "lw $t0, 8($fp)\n");
	fprintf(code, "lw $t1, 12($fp)\n");
	fprintf(code, "lw $fp, 4($fp)\n");
	fprintf(code, "jr $ra\n");


	//input
	fprintf(code, "input:\n");
	fprintf(code, "subu $sp, $sp, 16\n");
	fprintf(code, "sw $t1, 12($sp)\n");
	fprintf(code, "sw $t0, 8($sp)\n");
	fprintf(code, "sw $fp, 4($sp)\n"); // control link
	fprintf(code, "sw $ra, 0($sp)\n"); // return address
	fprintf(code, "move $fp, $sp\n");

	//process
	fprintf(code, "li $v0, 5\n");
	fprintf(code, "syscall\n");
	fprintf(code, "move $t2, $v0\n");
	
	//restore
	fprintf(code, "move $sp, $fp\n");
	fprintf(code, "addu $sp, $sp, %d\n", 16);
	fprintf(code, "lw $ra, 0($fp)\n");
	fprintf(code, "lw $t0, 8($fp)\n");
	fprintf(code, "lw $t1, 12($fp)\n");
	fprintf(code, "lw $fp, 4($fp)\n");
	fprintf(code, "jr $ra\n");

	cGen(syntaxTree, 0);
}

int calBase(char *name, int scope_number_for_type)
{
	ScopeList *target = getScopeTypeHead() -> next;
	BucketList l;
	int i, base = 0;

	while(target!=NULL && target -> scope_for_type != scope_number_for_type){
		target = target -> next;
	}

	int scope = target -> scope;

	scope--;

	while(scope > 0){
		while(target != NULL && target -> scope != scope){
			target = target -> next;
		}

		for(i=0;i<SIZE;i++){
			if(target -> hashTable[i] != NULL){
				l = target -> hashTable[i];
				while(l != NULL){
					if(l -> is_array == 0)
						base++;
					else{
						base += l -> array_size;
					}
					l = l->next;
				}
			}
		}
		scope--;
	}
	return base;
}
