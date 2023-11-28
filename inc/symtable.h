#ifndef __SYMTABLE__
#define __SYMTABLE__

#include <stddef.h>
#include "hashtable.h"
#include "symbol.h"

typedef struct _SymTable
{
    HashTable table;
    size_t size;  /* total size in bytes */
    int nb_items; /* number of elements in table */
} SymTable;

/* Theses functions assume that the SymTable is in a coherent state
 * size and nb_items should be correct
 * table == NULL should imply nb_items == 0 */

/**
 * @brief Return an empty SymTable, containing a NULL HashTable.
 *
 * @return SymTable an empty symbol table
 */
SymTable st_create(void);

/**
 * @brief Free the HashTable contained in *st. If st is NULL, do nothing.
 * Doesn't free *st itself.
 *
 * @param st address of a symbol table
 */
void st_free(SymTable *st);

/**
 * @brief Returns 1 if *st is empty, 0 otherwise.
 *
 * @param st address of a symbol table (non NULL)
 * @return int 1 if *st is empty, 0 otherwise
 */
int st_is_empty(SymTable *st);

/**
 * @brief Add a symbol to *st if no already present symbol share the same
 * identifier. Only the first `IDENT_LEN` bytes are used to compare two
 * identifiers. Doesn't change the symbol's address or index. Interrupts
 * program if memory allocation fails.
 *
 * @param st address of a symbol table, st and st->table must be non NULL
 * @param symbol symbol to add
 * @return int int 1 if the symbol has been added, 0 otherwise
 */
int st_add_if_absent(SymTable *st, Symbol *symbol);

/**
 * @brief Find a symbol with the given identifier in *st. If no match is found
 * then return an invalid symbol. This does not remove the result from *st.
 * Only the first `IDENT_LEN` bytes are used to compare two identifiers.
 *
 * @param st address of a symbol table (non NULL)
 * @param ident a '\0' terminated string (non NULL)
 * @return Symbol a match or an invalid symbol
 */
Symbol st_get(SymTable *st, char *ident);

/**
 * @brief Find a symbol with the given index in *st. If no match is found then
 * return an invalid symbol. This does not remove the result from *st.
 *
 * @param st address of a symbol table (non NULL)
 * @param index index to find
 * @return Symbol a match or an invalid symbol
 */
Symbol st_get_index(SymTable *st, int index);

/**
 * @brief Check if any symbol in *st already uses the given identifier. Only
 * the first `IDENT_LEN` bytes are used to compare two identifiers.
 *
 * @param st address of a symbol table (non NULL)
 * @param ident a '\0' terminated string (non NULL)
 * @return int 1 if the identifier is already used, 0 otherwise
 */
int st_contains(SymTable *st, char *ident);

/**
 * @brief Check if there's a symbol in *st1 and a symbol in *st2 that use the
 * same identifier. Only the first `IDENT_LEN` bytes are used to compare two
 * identifiers. *st1 and *st2 are supposed to have been completed using
 * `st_add_if_absent`, so that symbols are in the buckets corresponding to
 * their hash code.
 *
 * @param st1 address of a symbol table (non NULL)
 * @param st2 address of a symbol table (non NULL)
 * @return int 1 if st1 and st2 intersect, 0 otherwise
 */
int st_intersect(SymTable *st1, SymTable *st2);

/**
 * @brief Print all items of a symbol table, one line per item.
 *
 * @param st address of the symbol table to print (non NULL)
 */
void st_print(SymTable *st);

#endif
