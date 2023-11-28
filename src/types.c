#include "../inc/types.h"
#include <assert.h>
#include <string.h>

#define _SIZE_NUM 4
#define _SIZE_CHAR 1

size_t type_size(Type t)
{
    switch (t)
    {
        case T_NUM:     return _SIZE_NUM;
        case T_CHAR:    return _SIZE_CHAR;
        default:        return 0; /* error ? */
    }
}

Type str_to_type(char *s)
{
    assert(s != NULL);
    if (strcmp(s, "char") == 0)
        return T_CHAR;
    if (strcmp(s, "int") == 0)
        return T_NUM;
    if (strcmp(s, "void") == 0)
        return T_VOID;
    return T_INVALID;
}

char *type_to_str(Type t)
{
    switch (t)
    {
        case T_CHAR:    return "char";
        case T_NUM:     return "int";
        case T_VOID:    return "void";
        default:        return "?";
    }
}
