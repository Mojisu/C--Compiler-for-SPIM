/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#define TYPE_VAR 0
#define TYPE_FUNC 1
#define TYPE_PARAM 2

#define TRUE 1
#define FALSE 0

#define SIZE 211
/* SIZE is the size of the hash table */

typedef struct LineListRec
   { int lineno;
     struct LineListRec * next;
   } * LineList;

typedef struct BucketListRec
   { char * name;
     LineList lines;
     int memloc ; /* memory location for variable */
	 int kind; /*변수인지 0 함수인지 1 파라미터인지 2 */
	 int is_array;
	 int array_size;
	 int type;
	 int scope;
	 char *func_name;
     struct BucketListRec *next;
   } * BucketList;


typedef struct ScopeListRec
{
	int scope;
	int scope_for_type;
	BucketList hashTable[SIZE];
	struct ScopeListRec *next;
} ScopeList;

/* the hash table */
//static BucketList *hashTable[SIZE];
static ScopeList *ScopeHead, *ScopeTypeHead;


/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, int lineno, int loc, int kind, int is_array, int array_size, int type, int scope, char *func_name);
/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int st_lookup ( char * name );

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing);

void st_init();

void scope_insert();
void scope_delete();
int scope_check(char* name, int lineno); //0리턴ㅎ면 에러
int isArray(char *name, int scope_number_for_type);
int isFunc(char *name, int scope_number_for_type, int *f_kind);
int getType(char *name, int scope_number_for_type);
BucketList getBucket(char *name, int scope_number_for_type);
ScopeList* getScopeTypeHead();
ScopeList* getScopeList(char *name, int scope_number_for_type);
#endif
