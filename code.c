/****************************************************/
/* File: code.c                                     */
/* TM Code emitting utilities                       */
/* implementation for the TINY compiler             */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "code.h"

/* TM location number for current instruction emission */
static int emitLoc = 0 ;

/* Highest TM location emitted so far
   For use in conjunction with emitSkip,
   emitBackup, and emitRestore */
static int highEmitLoc = 0;



/* Procedure emitComment prints a comment line 
 * with comment c in the code file
 */
void emitComment( char * c )
{ 
	if (TraceCode) fprintf(code,"* %s\n",c);
}

/* Register only
 * ex) add $t0, $t0, $t1
 */
void emitRO(char *op, char *dest, char *src1, char *src2)
{ 
	fprintf(code, "%s %s, %s, %s\n", op, dest, src1, src2);
} /* emitRO */

/* Register and Memory
 * ex) sw $31, 20($29)
 */
void emitRM(char * op, char *dest, int base, char *src)
{
	fprintf(code, "%s %s, %d(%s)\n", op, dest, base, src);
}
void emitLabel(char *label) 
{
	fprintf(code, "%s:\n", label);
}
void emit1(char *op, char *a)
{
	fprintf(code, "%s %s\n", op, a);
}
void emit2(char *op, char *a1, char *a2) 
{
	fprintf(code, "%s %s, %s\n", op, a1, a2);
}


/* Function emitSkip skips "howMany" code
 * locations for later backpatch. It also
 * returns the current code position
 */
int emitSkip( int howMany)
{  
	int i = emitLoc;
	emitLoc += howMany ;
	if (highEmitLoc < emitLoc)  highEmitLoc = emitLoc ;
	return i;
} /* emitSkip */

/* Procedure emitBackup backs up to 
 * loc = a previously skipped location
 */
void emitBackup( int loc)
{ 
	if (loc > highEmitLoc) emitComment("BUG in emitBackup");
	emitLoc = loc ;
} /* emitBackup */

/* Procedure emitRestore restores the current 
 * code position to the highest previously
 * unemitted position
 */
void emitRestore(void)
{ 
	emitLoc = highEmitLoc;
}

/* Procedure emitRM_Abs converts an absolute reference 
 * to a pc-relative reference when emitting a
 * register-to-memory TM instruction
 * op = the opcode
 * r = target register
 * a = the absolute location in memory
 * c = a comment to be printed if TraceCode is TRUE
 */
void emitRM_Abs( char *op, int r, int a, char * c)
{ 
	fprintf(code,"%3d:  %5s  %d,%d(%d) ", emitLoc,op,r,a-(emitLoc+1),pc);
	++emitLoc;
	if (TraceCode) fprintf(code,"\t%s",c) ;
	fprintf(code,"\n") ;
	if (highEmitLoc < emitLoc) highEmitLoc = emitLoc ;
} /* emitRM_Abs */
