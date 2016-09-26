/****************************************************/
/* File: cgen.h                                     */
/* The code generator interface to the TINY compiler*/
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _CGEN_H_
#define _CGEN_H_

enum yytokentype
  {
    PLUS = 275,
    MINUS = 276,
    MUL = 277,
    BACKSLASH = 278,
    LT = 279,
    LTE = 280,
    GT = 281,
    GTE = 282,
    EQ = 283,
    NEQ = 284
  };

/* Procedure codeGen generates code to a code
 * file by traversal of the syntax tree. The
 * second parameter (codefile) is the file name
 * of the code file, and is used to print the
 * file name as a comment in the code file
 */
void codeGen(TreeNode * syntaxTree, char * codefile);
int calBase(char *name, int scope_number_for_type);
#endif
