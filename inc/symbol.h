#ifndef __SYMBOL__
#define __SYMBOL__

#include <stddef.h>
#include "types.h"

#define IDENT_LEN 64

typedef struct _Symbol
{
    char ident[IDENT_LEN]; /* should end in '\0' */
    size_t address;        /* relative address */
    Type type;             /* shouldn't be void... */
    int index;             /* order of insertion in table */
} Symbol;

/**
 * @brief Return an invalid symbol (meaning a symbol whose type is T_INVALID).
 * Used in HashTable and SymTable getters.
 *
 * @return Symbol an invalid symbol
 */
Symbol symbol_invalid(void);

/**
 * @brief Check symbol validity.
 *
 * @param symbol pointer to the symbol to check (non NULL)
 * @return int 1 if valid, 0 if invalid
 */
int symbol_is_valid(Symbol *symbol);

/**
 * @brief Print a symbol.
 *
 * @param symbol pointer to the symbol (non NULL)
 */
void symbol_print(Symbol *symbol);

#endif