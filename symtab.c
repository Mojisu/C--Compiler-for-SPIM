

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
#include "symtab.h"


/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

FILE* listing;

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

int scope_number = -1;
int scope_number_for_type = -1;

ScopeList* getScopeTypeHead()
{
	return ScopeTypeHead;
}

void st_init(){
	ScopeHead = (ScopeList*)malloc(sizeof(ScopeList));
	ScopeHead -> next = NULL;
	ScopeTypeHead = (ScopeList*)malloc(sizeof(ScopeList));
	ScopeTypeHead -> next = NULL;
	scope_insert();
	st_insert("input", 0, 0, 1, FALSE, 0, 1, -1, NULL);
	st_insert("output", 0, 1, 1, FALSE, 0, 1, -1, NULL);
}

//함수진입 or 왼쪽 중괄호 만났을때 호출
void scope_insert(){
	ScopeList *temp;
	temp = (ScopeList*)malloc(sizeof(ScopeList));
	temp -> next = ScopeHead -> next;
	temp -> scope = ++scope_number; //가장 최근일수록 숫자가 높음
	temp -> scope_for_type = ++scope_number_for_type;
	ScopeHead -> next = temp;
}

//오른쪽 중괄호 만났을 때 호출
void scope_delete(){
	ScopeList *temp, *move;
	temp = ScopeHead -> next;
	printSymTab(listing);
	ScopeHead -> next = temp -> next;
	scope_number--;

	move = ScopeTypeHead;
	while(move -> next != NULL) {
		move = move -> next;
	}
	move -> next = temp;
	temp -> next = NULL;
//	free(temp);
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, int lineno, int loc, int kind, int is_array, int array_size, int type, int scope, char *func_name)
{ 
	int h = hash(name);	
	ScopeList *target = ScopeHead -> next;

	if(scope!=-1){
		while(target->scope != scope){
			target = target->next;
		}
	}

	BucketList l = target->hashTable[h];

	while ((l != NULL) && (strcmp(name,l->name) != 0))
		l = l->next;
	if (l == NULL) /* variable not yet in table */
	{
		l = (BucketList) malloc(sizeof(struct BucketListRec));
		l->name = name;
		l->lines = (LineList) malloc(sizeof(struct LineListRec));
		l->lines->lineno = lineno;
		l->memloc = loc;
		l->kind = kind;
		l->is_array = is_array;
		l->array_size = array_size;
		l->type = type;
		l->scope = target->scope;
		l->lines->next = NULL;
		l->next = target -> hashTable[h];
		l->func_name = func_name;
		target -> hashTable[h] = l;
	}
	else /* found in table, so just add line number */
	{
		LineList t = l->lines;
		while (t->next != NULL) t = t->next;
		t->next = (LineList) malloc(sizeof(struct LineListRec));
		t->next->lineno = lineno;
		t->next->next = NULL;
	}
} /* st_insert */

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int st_lookup ( char * name )
{ 
  int h = hash(name);
  ScopeList *target = ScopeHead -> next;
  BucketList l =  target -> hashTable[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) return -1;
  else return l->memloc;
}

int scope_check(char *name, int lineno)
{
	ScopeList *target = ScopeHead -> next;
	while(target != NULL) {
		int h = hash(name);
		BucketList l = target -> hashTable[h];
		while(l != NULL) {			 
			if(strcmp(name, l -> name) == 0) {
				return target -> scope;
			}
			l = l->next;
		}
		target = target -> next;
	}
	return -1;
}
int isArray(char *name, int scope_number_for_type) {
	ScopeList *target = ScopeTypeHead -> next;
	BucketList l;
	int h = hash(name), scope;

	while(target != NULL && target -> scope_for_type != scope_number_for_type) {
		target = target -> next;
	}
	scope = target -> scope;
	l = target -> hashTable[h];
	while(l != NULL) {
		if(strcmp(name, l -> name) == 0) {
			return l -> is_array;
		}
		l = l -> next;
	}

	target = target -> next;
	scope--;
	while(scope >= 0) {
		while(target != NULL && target -> scope != scope) {
			target = target -> next;
		}
		if(target == NULL) {
			printf("target is NULL\n");
		}
		l = target -> hashTable[h];
		while(l != NULL) {
			if(strcmp(name, l -> name) == 0) {
				return l -> is_array;
			}
			l = l -> next;
		}
		scope--;
	}
	return 0;
}
int isFunc(char *name, int scope_number_for_type, int *type) {
    ScopeList *target = ScopeTypeHead -> next;
    BucketList l;
    int h = hash(name), scope;

    while(target != NULL && target -> scope_for_type != scope_number_for_type) {
        target = target -> next;
    }
    scope = target -> scope;
    l = target -> hashTable[h];
    while(l != NULL) {
        if(strcmp(name, l -> name) == 0) {
			*type = l -> type; 
            return l -> kind;
        }
        l = l -> next;
    }

    target = target -> next;
    scope--;
    while(scope >= 0) {
        while(target != NULL && target -> scope != scope) {
            target = target -> next;
        }
        if(target == NULL) {
            printf("target is NULL\n");
        }
        l = target -> hashTable[h];
        while(l != NULL) {
            if(strcmp(name, l -> name) == 0) {
				*type = l -> type;
                return l -> kind;
            }
            l = l -> next;
        }
        scope--;
    }
    return 0;
}
int getType(char *name, int scope_number_for_type) {
	ScopeList *target = ScopeTypeHead -> next;
	BucketList l;
	int h = hash(name), scope;

	while(target != NULL && target -> scope_for_type != scope_number_for_type) {
		target = target -> next;
	}
	scope = target -> scope;
	l = target -> hashTable[h];
	while(l != NULL) {
		if(strcmp(name, l -> name) == 0) {
			return l -> type;
		}
		l = l -> next;
	}

	target = target -> next;
    scope--;
    while(scope >= 0) {
        while(target != NULL && target -> scope != scope) {
            target = target -> next;
        }
        if(target == NULL) {
            printf("target is NULL\n");
        }
        l = target -> hashTable[h];
        while(l != NULL) {
            if(strcmp(name, l -> name) == 0) {
                return l -> type;
            }
            l = l -> next;
        }
        scope--;
    }
}
BucketList getBucket(char *name, int scope_number_for_type) {
	ScopeList *target = ScopeTypeHead -> next;
	BucketList l;
	int h = hash(name), scope;

	while(target != NULL && target -> scope_for_type != scope_number_for_type) {
		target = target -> next;
	}
	scope = target -> scope;
	l = target -> hashTable[h];
	while(l != NULL) {
		if(strcmp(name, l -> name) == 0) {
			return l;
		}
		l = l -> next;
	}

	target = target -> next;
    scope--;
    while(scope >= 0) {
        while(target != NULL && target -> scope != scope) {
            target = target -> next;
        }
        if(target == NULL) {
            printf("target is NULL\n");
        }
        l = target -> hashTable[h];
        while(l != NULL) {
            if(strcmp(name, l -> name) == 0) {
                return l;
            }
            l = l -> next;
        }
        scope--;
    }
}

ScopeList* getScopeList(char *name, int scope_number_for_type){
	ScopeList *target = ScopeTypeHead -> next;
	BucketList l;
	int h = hash(name), scope;

	while(target != NULL && target -> scope_for_type != scope_number_for_type) {
		target = target -> next;
	}
	scope = target -> scope;
	l = target -> hashTable[h];
	while(l != NULL) {
		if(strcmp(name, l -> name) == 0) {
			return target;
		}
		l = l -> next;
	}

	target = target -> next;
    scope--;
    while(scope >= 0) {
        while(target != NULL && target -> scope != scope) {
            target = target -> next;
        }
        if(target == NULL) {
            printf("target is NULL\n");
        }
        l = target -> hashTable[h];
        while(l != NULL) {
            if(strcmp(name, l -> name) == 0) {
                return target;
            }
            l = l -> next;
        }
        scope--;
    }

}
/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing)
{ 
	int i;
	char temp[10];
	ScopeList *target = ScopeHead -> next;
	if(target == NULL) return;
	//BucketList l =  target->hashTable[h];
	while(target != NULL) {
		for(i=0; i<SIZE; i++) {
			if(target -> hashTable[i] != NULL) break;
		}
		if(i == SIZE) return;

		fprintf(listing,"Variable Name  Scope  Location	 Kind   IsArray  ArraySize  Type  Line Numbers\n");
		fprintf(listing,"-------------  -----  -------- ------  -------  ---------  ----  ------------\n");
		for(i=0; i<SIZE; i++) {
			if(target -> hashTable[i] != NULL) { 
				BucketList l = target -> hashTable[i];
				while(l != NULL) {
					LineList t = l -> lines;
					fprintf(listing, "%-13s  ", l -> name); // name
					fprintf(listing, "%-5d  ", l -> scope); // scope
					fprintf(listing, "%-8d  ", l -> memloc); // location

					// kind
					if(l -> kind == TYPE_VAR) strcpy(temp, "VAR");
					else if(l -> kind == TYPE_PARAM) strcpy(temp, "PARAM");
					else strcpy(temp, "FUNC");
					fprintf(listing, "%-6s  ", temp);

					// IsArray
					if(l -> is_array == TRUE) strcpy(temp, "TRUE");
					else strcpy(temp, "FALSE");
					fprintf(listing, "%-7s  ", temp);

					// ArraySize
					fprintf(listing, "%-9d  ", l -> array_size);

					// Type
					if(l -> type == 0) strcpy(temp, "VOID");
					else strcpy(temp, "INT");
					fprintf(listing, "%-4s  ", temp);

					while (t != NULL) {
						fprintf(listing,"%d ",t -> lineno);
						t = t -> next;
					}
					fprintf(listing,"\n");
					l = l -> next;
				}
			}
		}
		target = target -> next;
	}
	fprintf(listing, "\n\n");
} /* printSymTab */
