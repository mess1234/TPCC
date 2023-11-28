#ifndef __TREE__
#define __TREE__

#define TO_ENUM(ENUM) ENUM,
#define TO_STRING(STR) #STR,
#define MAP_LABELS(INSTR) \
    INSTR(start)          \
    INSTR(global_vars)    \
    INSTR(local_vars)     \
    INSTR(var_init)       \
    INSTR(var_no_init)    \
    INSTR(functions)      \
    INSTR(function)       \
    INSTR(prototype)      \
    INSTR(params)         \
    INSTR(param)          \
    INSTR(body)           \
    INSTR(stmt_sequence)  \
    INSTR(empty_stmt)     \
    INSTR(call_func)      \
    INSTR(args)           \
    INSTR(type)           \
    INSTR(_void_)         \
    INSTR(_if_)           \
    INSTR(if_else)        \
    INSTR(_while_)        \
    INSTR(return_exp)     \
    INSTR(return_void)    \
    INSTR(unop_pm)        \
    INSTR(unop_not)       \
    INSTR(binop_and)      \
    INSTR(binop_or)       \
    INSTR(binop_eq)       \
    INSTR(binop_order)    \
    INSTR(binop_addsub)   \
    INSTR(binop_divstar)  \
    INSTR(binop_assign)   \
    INSTR(number)         \
    INSTR(character)      \
    INSTR(ident)

typedef enum
{
    MAP_LABELS(TO_ENUM)
    // list all other node labels, if any
    // The list must coincide with the string array in tree.c
    // To avoid listing them twice, see https://stackoverflow.com/a/10966395
} label_t;

typedef union _Value
{
    int num;
    char byte;
    char type[5];
    char comp[3];
    char ident[64];
} Value;

typedef struct _Node
{
    label_t label;
    Value val;
    struct _Node *firstChild, *nextSibling;
    int line_nb;
} Node;

Node *makeNode(label_t label);
void addSibling(Node *node, Node *sibling);
void addChild(Node *parent, Node *child);
void deleteTree(Node *node);
void printTree(Node *node);

/**
 * @brief Count the number of childrens of a node.
 *
 * @param node the node
 * @return int the number of childrens
 */
int nbChild(Node *node);

/**
 * @brief Set a type to each variable node of a line. Stops when encountering a
 * variable whose type is already set. The type of a variable node `v` is
 * considered set when v->firstChild->val.type[0] != '\0'.
 *
 * @param line first variable of the line, the others ones are its siblings
 * @param type a char[5] containing "int" or "char"
 */
void setLineTypes(Node *line, char *type);

#define FIRSTCHILD(node) ((node)->firstChild)
#define SECONDCHILD(node) ((node)->firstChild->nextSibling)
#define THIRDCHILD(node) ((node)->firstChild->nextSibling->nextSibling)

#endif