#include "../inc/linkedlist.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* PRIVATE FUNCTIONS */

/**
 * @brief Find a node whose value's identifier matches ident. Only the first
 * `IDENT_LEN` bytes are used to compare two identifiers.
 *
 * @param l list to search
 * @param ident a '\0' terminated string (non NULL)
 * @return ListNode* adress of the first matching node, or NULL if not found
 */
static ListNode *_locate_ident(List l, char *ident)
{
    ListNode *curr;
    for (curr = l; curr != NULL; curr = curr->next)
        if (strncmp(curr->value.ident, ident, IDENT_LEN) == 0)
            break; /* found */
    return curr;
}

/* PUBLIC FUNCTIONS */

void list_free(List l)
{
    ListNode *tmp;
    while (l != NULL)
    {
        tmp = l->next;
        free(l);
        l = tmp;
    }
}

void list_add(List *lp, Symbol *symbol)
{
    ListNode *new_node;
    assert(lp != NULL);
    /* Allocate memory for new node */
    new_node = (ListNode *)malloc(1 * sizeof(ListNode));
    if (new_node == NULL)
    {
        fprintf(stderr, "could not create node\n");
        exit(-1);
    }
    /* Set node */
    /* copy works because symbol fields are statically allocated */
    new_node->value = *symbol;
    new_node->next = *lp;
    /* Add it to the head of the list */
    *lp = new_node;
}

Symbol list_get(List l, char *ident)
{
    ListNode *location;
    assert(ident != NULL);
    location = _locate_ident(l, ident);
    if (location == NULL)
        return symbol_invalid();
    return location->value;
}

int list_contains(List l, char *ident)
{
    assert(ident != NULL);
    return _locate_ident(l, ident) != NULL;
}

int list_intersect(List l1, List l2)
{
    ListNode *curr;
    /* for all idents in l1, check if it's l2 */
    for (curr = l1; curr != NULL; curr = curr->next)
        if (list_contains(l2, curr->value.ident))
            return 1;
    return 0;
}

void list_print(List l)
{
    ListNode *curr;
    for (curr = l; curr != NULL; curr = curr->next)
    {
        symbol_print(&(curr->value));
        putchar('\n');
    }
}