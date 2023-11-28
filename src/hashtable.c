#include "../inc/hashtable.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define _LEN 1008
#define _FACTOR 613
#define _MASK 0X3FFFFFFF /* used for x -> x mod 2^30, _MASK is 2^30 - 1 */

/* PRIVATE FUNCTIONS */

/**
 * @brief Return the index corresponding to a key.
 *
 * @param key a '\0' terminated string (non NULL)
 * @return int index in the hash table's array
 */
static int _hash(char *key)
{
    int i, res = 0;
    for (i = 0; key[i] != '\0'; i++)
        res = (_FACTOR * res + key[i]) & _MASK;
    return res % _LEN;
}

/**
 * @brief Initialize all the linked lists inside a hash table.
 *
 * @param table the table (non NULL)
 */
static void _init_table(HashTable table)
{
    int i;
    for (i = 0; i < _LEN; i++)
        table[i] = NULL;
}

/* PUBLIC FUNCTIONS */

HashTable ht_create(void)
{
    HashTable table;
    table = (HashTable)malloc(_LEN * sizeof(List));
    if (table == NULL)
    {
        fprintf(stderr, "could not create table\n");
        exit(-1);
    }
    _init_table(table);
    return table;
}

void ht_free(HashTable table)
{
    int i;
    if (table == NULL)
        return;
    for (i = 0; i < _LEN; i++)
        list_free(table[i]);
    free(table);
}

int ht_add_if_absent(HashTable table, Symbol *symbol)
{
    List *bucket_p; /* address of symbol's bucket */
    int bucket_idx; /* index of symbol's bucket */
    assert(table != NULL);
    assert(symbol != NULL);
    bucket_idx = _hash(symbol->ident); /* key = symbol's identifier */
    bucket_p = &(table[bucket_idx]);
    if (list_contains(*bucket_p, symbol->ident))
        return 0;
    list_add(bucket_p, symbol);
    return 1;
}

Symbol ht_get(HashTable table, char *ident)
{
    List bucket;
    assert(table != NULL);
    assert(ident != NULL);
    bucket = table[_hash(ident)];
    return list_get(bucket, ident);
}

Symbol ht_get_index(HashTable table, int index)
{
    ListNode *n;
    int i;
    assert(table != NULL);
    for (i = 0; i < _LEN; i++)                     /* iterate table lists */
        for (n = table[i]; n != NULL; n = n->next) /* iterate current list */
            if (n->value.index == index)
                return n->value;
    return symbol_invalid();
}

int ht_contains(HashTable table, char *ident)
{
    List *bucket_p; /* address of ident's bucket */
    int bucket_idx;
    assert(table != NULL);
    assert(ident != NULL);
    bucket_idx = _hash(ident);
    bucket_p = &(table[bucket_idx]);
    return list_contains(*bucket_p, ident);
}

/* suppose that t1 and t2 has been completed using ht_add_if_absent, (so that
symbols are in the correct buckets) */
int ht_intersect(HashTable t1, HashTable t2)
{
    int i;
    assert(t1 != NULL);
    assert(t2 != NULL);
    for (i = 0; i < _LEN; i++)
        if (list_intersect(t1[i], t2[i]))
            return 1;
    return 0;
}

void ht_print(HashTable table)
{
    int i;
    assert(table != NULL);
    for (i = 0; i < _LEN; i++)
        list_print(table[i]);
}

#if 0
/**
 * @brief Add a value to the table. Interrupt program if memory allocation
 * fails.
 *
 * @param table the table (non NULL)
 * @param key a '\0' terminated string (non NULL)
 * @param value value to add
 */
void add(HashTable table, char *key, Symbol value)
{
    int index;
    assert(table != NULL);
    assert(key != NULL);
    index = _hash(key);
    list_add(&(table[index]), value);
}
#endif
