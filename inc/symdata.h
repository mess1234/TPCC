#ifndef __SYMDATA__
#define __SYMDATA__

#include "symfunc.h"
#include "symtable.h"

typedef struct _SymData
{
    SymTable *globals;
    FnSymArr *fsa;
} SymData;

/* sd_print ? */

#endif
