#ifndef __COMP_CHECK_DECL__
#define __COMP_CHECK_DECL__

#include "symdata.h"
#include "symfunc.h"
#include "tree.h"

/* doesn't check expressions in local variables initialization, that's done in
 * comp_init_symbols */

/**
 * @brief Check if the symbols used in instructions have been declared. If not,
 * throws a semantic error.
 *
 * @param tree abstract syntax tree
 * @param symbols symbols tables for globals and functions
 */
void check_declarations(Node *tree, SymData *symbols);

/**
 * @brief Check if the symbols used in an expression have been declared. If
 * not, throws a semantic error.
 *
 * @param exp expression node in syntax tree
 * @param symbols symbols tables for globals and functions
 * @param fs the FnSym corresponding to the function surrounding `exp`
 */
void check_exp_decl(Node *exp, SymData *symbols, FnSym *fs);

#endif