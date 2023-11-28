#ifndef __TYPES__
#define __TYPES__

#include <stddef.h>

typedef enum _Type
{
    T_INVALID, /* useful for ht_get... */
    T_CHAR,
    T_NUM,
    T_VOID
} Type;

/**
 * @brief Returns the amount of bytes required to store a variable of type t.
 * If t is T_INVALID or T_VOID, return 0.
 *
 * @param t type
 * @return size_t size in bytes
 */
size_t type_size(Type t);

/**
 * @brief Get a string reprenting a type.
 *
 * @param t the type
 * @return char* string representing t
 */
char *type_to_str(Type t);

/**
 * @brief Return the type corresponding to s.
 *
 * @param s a string (non NULL)
 * @return Type T_INVALID if s doesn't correspond to any type
 */
Type str_to_type(char *s);

#endif
