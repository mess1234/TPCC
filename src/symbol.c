#include "../inc/symbol.h"
#include <assert.h>
#include <stdio.h>

Symbol symbol_invalid(void)
{
    Symbol res = {"", 0, T_INVALID, -1};
    return res;
}

int symbol_is_valid(Symbol *symbol)
{
    assert(symbol != NULL);
    return symbol->type != T_INVALID;
}

void symbol_print(Symbol *symbol)
{
    assert(symbol != NULL);
    printf("(%2d, %02lu, %s, %s)",
           symbol->index,
           symbol->address,
           type_to_str(symbol->type),
           symbol->ident);
}

#if 0
/**
 * @brief Compare two symbol's identifiers.
 *
 * @param s1 address of a symbol to compare
 * @param s2 address of a symbol to compare
 * @return int 1 if *s1 and *s2 share the same identifiers, 0 otherwise
 */
int comp_ident(Symbol *s1, Symbol *s2)
{
    assert(s1 != NULL);
    assert(s2 != NULL);
    return strncmp(s1->ident, s2->ident, IDENT_LEN) == 0;
}
#endif
