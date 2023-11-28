#ifndef __COMP_TRANSLATE__
#define __COMP_TRANSLATE__

#include "symdata.h"
#include "tree.h"

/**
 * @brief Translate the abstract syntax tree to nasm code. Write result to
 * ./asm/_anonymous.asm.
 *
 * @param tree abstract syntax tree
 * @param symbols address to use for the globals and functions (non NULL)
 */
void translate(Node *tree, SymData *symbols);

#endif
