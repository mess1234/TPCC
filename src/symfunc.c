#include "../inc/symfunc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* PRIVATE FUNCTIONS */

/**
 * @brief Search an item with the given identifier. Return a pointer to the
 * first match, or NULL if not found. Only the first `IDENT_LEN` bytes are used
 * to compare two identifiers.
 *
 * @param fsa pointer to the FnSymArr (non NULL)
 * @param ident a '\0' terminated string (non NULL)
 * @return FnSym* pointer to an entry, or NULL
 */
FnSym *_locate_ident(FnSymArr *fsa, char *id)
{
    FnSym *current;
    size_t i;
    current = fsa->arr;
    for (i = 0; i < fsa->len; i++)
    {
        if (strncmp(current->ident, id, IDENT_LEN) == 0)
            return current;
        current++;
    }
    return NULL;
}

/* PUBLIC FUNCTIONS */

void fs_reset_tables(FnSym *old)
{
    assert(old != NULL);
    old->params = st_create();
    old->locals = st_create();
}

void fs_free(FnSym *fs)
{
    if (fs == NULL)
        return;
    st_free(&(fs->params));
    st_free(&(fs->locals));
}

void fs_print(FnSym *fs)
{
    assert(fs != NULL);
    printf("function '%s', returns %s\n", fs->ident, type_to_str(fs->type));
    printf("* parameters\n");
    st_print(&(fs->params));
    printf("* local variables\n");
    st_print(&(fs->locals));
}

FnSymArr fsa_create(size_t capacity)
{
    FnSymArr new = {NULL, 0, 0};
    new.arr = (FnSym *)malloc(capacity * sizeof(FnSym));
    if (new.arr == NULL)
    {
        fprintf(stderr, "could not create FnSymArr\n");
        exit(-1);
    }
    new.capacity = capacity;
    return new;
}

void fsa_free(FnSymArr *fsa)
{
    size_t i;
    if (fsa == NULL)
        return;
    for (i = 0; i < fsa->len; i++)
        fs_free(fsa->arr + i);
    free(fsa->arr);
}

int fsa_add_if_absent(FnSymArr *fsa, FnSym *fs)
{
    assert(fsa != NULL);
    assert(fs != NULL);
    if (fsa->len >= fsa->capacity)
        return 2;
    if (_locate_ident(fsa, fs->ident) != NULL)
        return 0;
    fsa->arr[fsa->len] = *fs; /* shallow copy, doesn't duplicate tables */
    fsa->len++;
    return 1;
}

FnSym *fsa_get(FnSymArr *fsa, char *ident)
{
    assert(fsa != NULL);
    assert(ident != NULL);
    return _locate_ident(fsa, ident);
}

int fsa_contains(FnSymArr *fsa, char *ident)
{
    assert(fsa != NULL);
    assert(ident != NULL);
    return _locate_ident(fsa, ident) != NULL;
}

void fsa_print(FnSymArr *fsa)
{
    FnSym *current;
    size_t i;
    assert(fsa != NULL);
    current = fsa->arr;
    for (i = 0; i < fsa->len; i++)
    {
        fs_print(current);
        putchar('\n');
        current++;
    }
}