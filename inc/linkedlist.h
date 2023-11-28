#ifndef __LINKEDLIST__
#define __LINKEDLIST__

#include "symbol.h"

/* A linked list node */
typedef struct _ListNode
{
    Symbol value;
    struct _ListNode *next;
} ListNode;

typedef ListNode *List;

/**
 * @brief Free all nodes of the list.
 *
 * @param l list to free
 */
void list_free(List l);

/**
 * @brief Add a symbol to the head of the list. Interrupt program if memory
 * allocation fails.
 *
 * @param lp pointer to the list (non NULL)
 * @param symbol pointer to the symbol to add
 */
void list_add(List *lp, Symbol *symbol);

/**
 * @brief Find a symbol with the given identifier in the list. Return the first
 * match. If no match is found then return an invalid symbol. This does not
 * remove the result from the list. Only the first `IDENT_LEN` bytes are used
 * to compare two identifiers.
 *
 * @param l list to check
 * @param ident a '\0' terminated string (non NULL)
 * @return Symbol first match found, or an invalid symbol
 */
Symbol list_get(List l, char *ident);

/**
 * @brief Check if the list contains a symbol with the given identifier.
 * Only the first `IDENT_LEN` bytes are used to compare two identifiers.
 *
 * @param l list to check
 * @param ident a '\0' terminated string (non NULL)
 * @return int 1 if there's a match, 0 otherwise
 */
int list_contains(List l, char *ident);

/**
 * @brief Check if there's a symbol in l1 and a symbol in l2 that use the same
 * identifier. Only the first `IDENT_LEN` bytes are used to compare two
 * identifiers.
 *
 * @param l1 a list
 * @param l2 a list
 * @return int 1 if l1 and l2 intersect, 0 otherwise
 */
int list_intersect(List l1, List l2);

/**
 * @brief Print all items in the list.
 *
 * @param l list to print
 */
void list_print(List l);

#endif