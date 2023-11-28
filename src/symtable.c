#include "../inc/symtable.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../inc/types.h"

SymTable st_create(void)
{
    SymTable new = {NULL, 0, 0};
    return new;
}

void st_free(SymTable *st)
{
    if (st != NULL)
        ht_free(st->table);
}

int st_is_empty(SymTable *st)
{
    assert(st != NULL);
    return st->nb_items == 0;
}

int st_add_if_absent(SymTable *st, Symbol *symbol)
{
    assert(st != NULL);
    assert(symbol != NULL);
    if (!ht_add_if_absent(st->table, symbol))
        return 0;
    st->size += type_size(symbol->type);
    st->nb_items++;
    return 1;
}

Symbol st_get(SymTable *st, char *ident)
{
    assert(st != NULL);
    assert(ident != NULL);
    if (st_is_empty(st))
        /* don't call ht_get when st->table == NULL */
        return symbol_invalid();
    return ht_get(st->table, ident);
}

Symbol st_get_index(SymTable *st, int index)
{
    assert(st != NULL);
    if (st_is_empty(st))
        return symbol_invalid();
    return ht_get_index(st->table, index);
}

int st_contains(SymTable *st, char *ident)
{
    assert(st != NULL);
    assert(ident != NULL);
    if (st_is_empty(st))
        return 0;
    return ht_contains(st->table, ident);
}

/* suppose symbols are in the correct buckets) */
int st_intersect(SymTable *st1, SymTable *st2)
{
    assert(st1 != NULL);
    assert(st2 != NULL);
    if (st_is_empty(st1) || st_is_empty(st2))
        return 0;
    return ht_intersect(st1->table, st2->table);
}

void st_print(SymTable *st)
{
    assert(st != NULL);
    printf("table with %d elements in %lu bytes\n", st->nb_items, st->size);
    if (st->table == NULL)
        printf("null\n");
    else
        ht_print(st->table);
}
