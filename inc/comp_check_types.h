/**
 * This module has to verify :
 * - local variables initialization
 * - assignment (lvalue = exp)
 * - arguments in function calls
 *     + error if incorrect number of args
 *     + error if void expression
 *     + warning if char param is given an int
 * - function return type
 *     + error if a non void function returns void
 *     + error if a void function returns non void
 *     + warning if a char function returns int
 *
 * Notes :
 * a non void function doesn't have to include a return exp, the return value
 * will be whatever value is in rax
 *
 * there's a warning when a non void function includes a `return;` but not when
 * it doesn't inlude any return at all...
 */

#ifndef __COMP_CHECK_TYPES__
#define __COMP_CHECK_TYPES__

#include "symdata.h"
#include "tree.h"

/**
 * @brief Checks that the types are correct for all instructions and all local
 * variable initialization.
 *
 * @param tree abstract syntax tree
 * @param symbols address of a SymData containing the globals variables and
 * every function
 */
void check_types(Node *tree, SymData *symbols);

#endif