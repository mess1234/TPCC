#ifndef __SYMFUNC__
#define __SYMFUNC__

#include <stddef.h>
#include "symtable.h"
#include "types.h"

/* Holds a function's
 * - identifier
 * - return type
 * - parameters symbol table
 * - local variables symbol table */
typedef struct _FnSym
{
    SymTable params;
    SymTable locals;
    char ident[IDENT_LEN]; /* should end in '\0' */
    Type type;             /* return type of the function */
} FnSym;

/* just an array of FnSym */
typedef struct _FnSymArr
{
    FnSym *arr;
    size_t len;      /* used length */
    size_t capacity; /* max length, don't change it */
} FnSymArr;

/**
 * @brief Reset a FnSym's tables but doesn't free them. Doesn't reset the
 * identifier and type.
 *
 * @param old a FnSym (non NULL)
 */
void fs_reset_tables(FnSym *old);

/**
 * @brief Free the symbol tables contained in fs. Doesn't free *fs itself.
 * If fs is NULL, do nothing.
 *
 * @param fs address of a FnSym
 */
void fs_free(FnSym *fs);

/**
 * @brief Print information about a function.
 *
 * @param fs address of the function's FnSym (non NULL)
 */
void fs_print(FnSym *fs);

/**
 * @brief Create and return a new FnSymArr fsa. Memory has been allocated
 * dynamically for fsa.arr, so you'll have to use fsa_free.
 *
 * @param capacity maximum capacity of the new array
 * @return FnSymArr the new array
 */
FnSymArr fsa_create(size_t capacity);

/**
 * @brief Free the symbol table of every item in *fsa, and free fsa->arr.
 * Doesn't free *fsa itself. If fsa is NULL, do nothing.
 *
 * @param fsa address of a FnSymArr
 */
void fsa_free(FnSymArr *fsa);

/**
 * @brief Add an item to the array, if no already present item share the same
 * identifier. When adding the item, do a shallow copy. Only the first
 * `IDENT_LEN` bytes are used to compare two identifiers.
 *
 * @param fsa pointer to the FnSymArr (non NULL)
 * @param fs pointer to the item (non NULL)
 * @return int 1 if the item has been added, 2 if not enough room, 0 if there
 * was enough room but the item's identifier was already used
 */
int fsa_add_if_absent(FnSymArr *fsa, FnSym *fs);

/**
 * @brief Search an item with the given identifier. Return a pointer to the
 * first match, or NULL if not found. Only the first `IDENT_LEN` bytes are used
 * to compare two identifiers.
 *
 * @param fsa pointer to the FnSymArr (non NULL)
 * @param ident a '\0' terminated string (non NULL)
 * @return FnSym* pointer to an entry, or NULL
 */
FnSym *fsa_get(FnSymArr *fsa, char *ident);

/**
 * @brief Check if any item in the array already uses the given identifier.
 * Only the first `IDENT_LEN` bytes are used to compare two identifiers.
 *
 * @param fsa pointer to the FnSymArr (non NULL)
 * @param ident a '\0' terminated string (non NULL)
 * @return int 1 if the identifier is already used, 0 otherwise
 */
int fsa_contains(FnSymArr *fsa, char *ident);

/**
 * @brief Print information about every function in *fsa.
 *
 * @param fsa pointer to a FnSymArr (non NULL)
 */
void fsa_print(FnSymArr *fsa);

#endif