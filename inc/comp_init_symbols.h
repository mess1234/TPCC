#ifndef __COMP_INIT_SYMBOLS__
#define __COMP_INIT_SYMBOLS__

#include "symdata.h"
#include "tree.h"

/**
 * @brief Make symbol tables for globals and functions. Interrupt program if a
 * semantic error is found :
 * - name collision within the same symbol table
 * - name collision between globals and functions
 * - name collision between parameters and local variables of a function
 * - local variable initialized with unknown symbols
 * - no `main` function
 * - `main` prototype isn't `int main(void)`
 *
 * @param tree abstract syntax tree
 * @param symbols address to use for the globals and functions (non NULL)
 */
void make_symbol_tables(Node *tree, SymData *symbols);

#endif