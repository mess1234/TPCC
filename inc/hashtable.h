#ifndef __HASHTABLE__
#define __HASHTABLE__

#include "linkedlist.h"
#include "symbol.h"

/**
 * Hash table containing symbols.
 * Value = a symbol
 * Key   = the symbol's identifier
 * Linked lists are used to handle collisions => table = array of linked lists.
 * The length of the array is fixed.
 */
typedef List *HashTable;

/**
 * @brief Create and init a new hash table. The table has been dynamically
 * allocated and must be freed with ht_free. Interrupts program if memory
 * allocation fails.
 *
 * @return HashTable empty hash table
 */
HashTable ht_create(void);

/**
 * @brief Free memory allocated for a hash table.
 * If table is NULL, do nothing.
 *
 * @param table table to free
 */
void ht_free(HashTable table);

/**
 * @brief Add a symbol to the table if no already present symbol share the same
 * identifier. Only the first `IDENT_LEN` bytes are used to compare two
 * identifiers. Interrupts program if memory allocation fails.
 *
 * @param table the table (non NULL)
 * @param symbol symbol to add
 * @return int 1 if the symbol has been added, 0 otherwise
 */
int ht_add_if_absent(HashTable table, Symbol *symbol);

/**
 * @brief Find a symbol with the given identifier in the table. If no match is
 * found then return an invalid symbol. This does not remove the result from
 * the table. Only the first `IDENT_LEN` bytes are used to compare two
 * identifiers.
 *
 * @param table the table (non NULL)
 * @param ident a '\0' terminated string (non NULL)
 * @return Symbol a match or an invalid symbol
 */
Symbol ht_get(HashTable table, char *ident);

/**
 * @brief Find a symbol with the given index in the table. If no match is found
 * then return an invalid symbol. This does not remove the result from the
 * table.
 *
 * @param table the table (non NULL)
 * @param index index to find
 * @return Symbol a match or an invalid symbol
 */
Symbol ht_get_index(HashTable table, int index);

/**
 * @brief Check if any symbol in the table already uses the given identifier.
 * Only the first `IDENT_LEN` bytes are used to compare two identifiers.
 *
 * @param table the table (non NULL)
 * @param ident a '\0' terminated string (non NULL)
 * @return int 1 if the identifier is already used, 0 otherwise
 */
int ht_contains(HashTable table, char *ident);

/**
 * @brief Check if there's a symbol in t1 and a symbol in t2 that use the same
 * identifier. Only the first `IDENT_LEN` bytes are used to compare two
 * identifiers. t1 and t2 are supposed to have been completed using
 * `ht_add_if_absent`, so that symbols are in the buckets corresponding to
 * their hash code.
 *
 * @param t1 a table (non NULL)
 * @param t2 a table (non NULL)
 * @return int 1 if t1 and t2 intersect, 0 otherwise
 */
int ht_intersect(HashTable t1, HashTable t2);

/**
 * @brief Print all items of a hash table, one line per item.
 *
 * @param table hash table to print (non NULL)
 */
void ht_print(HashTable table);

#endif