#include "../inc/comp_init_symbols.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/comp_check_decl.h" // for local vars
#include "../inc/hashtable.h"
#include "../inc/macros.h"
#include "../inc/symbol.h"
#include "../inc/symfunc.h"
#include "../inc/types.h"

static int _nb_funcs(Node *tree);
static Type _get_valid_type(Node *node);
static Type _get_valid_symbol_type(Node *node);
static void _add_varpar(Node *var, SymTable *st);

static void _make_globals(Node *tree, SymData *symbols);
static void _fill_globals(Node *list, SymTable *st);
static int _uses_predef_ident(Node *var);

static void _make_fsa(Node *tree, SymData *symbols);
static void _add_predefined(SymData *symbols);
static SymTable _putchar_params(void);
static SymTable _putint_params(void);
static void _fill_fsa_idents(Node *tree, SymData *symbols);
static void _add_fs(Node *func, SymData *symbols);
static void _check_main(Node *func, FnSym *fs);
static void _fill_fsa_tables(Node *tree, SymData *symbols);
static void _fill_fs_tables(Node *func, FnSym *fs, SymData *symbols);
static void _fill_params(Node *list, FnSym *fs);
static void _fill_locals(Node *list, FnSym *fs, SymData *symbols);
static int _var_in_params(Node *var, FnSym *fs);

/* PUBLIC FUNCTIONS */

void make_symbol_tables(Node *tree, SymData *symbols)
{
    assert(tree != NULL);
    assert(symbols != NULL);
    _make_globals(tree, symbols);
    _make_fsa(tree, symbols);
}

/* PRIVATE FUNCTIONS */

/**
 * @brief Return the number of function declarations in an AST.
 *
 * @param tree abstract syntax tree
 * @return int number of functions declared in tree
 */
static int _nb_funcs(Node *tree)
{
    return nbChild(SECONDCHILD(tree));
}

/**
 * @brief Get the type represented by a node. Throws an error if the type is
 * not valid.
 *
 * @param node should have a `_void_` or `type` label
 * @return Type T_CHAR, T_NUM or T_VOID
 */
static Type _get_valid_type(Node *node)
{
    Type res;
    switch (node->label)
    {
        case _void_:    res = T_VOID;                       break;
        case type:      res = str_to_type(node->val.type);  break;
        default:        res = T_INVALID;                    break;
    }
    if (res != T_INVALID)
        return res;
    // we're here => this node isn't neither a _void_ node, neither a type node
    // holding a correct value. That's a bug from the lexer or parser
    // => programming error
    fprintf(stderr, "line %d : error : ", node->line_nb);
    fprintf(stderr, "incorrect syntax tree (invalid type)\n");
    exit(EXIT_OTHER_ERR);
}

/**
 * @brief Get the type represented by a node. Throws an error if the type is
 * void or isn't valid.
 *
 * @param node should have a `type` label
 * @return Type T_CHAR or T_NUM
 */
static Type _get_valid_symbol_type(Node *node)
{
    Type res = _get_valid_type(node);
    if (res != T_VOID)
        return res;
    fprintf(stderr, "line %d : error : ", node->line_nb);
    fprintf(stderr, "incorrect syntax tree (void symbol)\n");
    exit(EXIT_OTHER_ERR);
}

/**
 * @brief Add one variable or parameter to a symbol table. Throws a semantic
 * error if the identifier is already used.
 *
 * @param var var_init, var_no_init or param node
 * @param st symbol table
 */
static void _add_varpar(Node *var, SymTable *st)
{
    Symbol curr_sym;
    // Get info
    curr_sym.type = _get_valid_symbol_type(FIRSTCHILD(var));
    strncpy(curr_sym.ident, SECONDCHILD(var)->val.ident, IDENT_LEN);
    curr_sym.address = st->size;
    curr_sym.index = st->nb_items;
    // Add to table
    if (!st_add_if_absent(st, &curr_sym))
    {
        fprintf(stderr, "line %d : error : ", var->line_nb);
        fprintf(stderr, "redeclaration of '%s'\n", curr_sym.ident);
        exit(EXIT_SEM_ERR);
    }
}

/* add globals to symdata */
static void _make_globals(Node *tree, SymData *symbols)
{
    Node *globals = tree->firstChild;
    *(symbols->globals) = st_create();
    if (globals->firstChild != NULL)
        _fill_globals(globals, symbols->globals);
}

/* fill globals symbol table, useless if there is zero global variable
 * st = symbol table for globals (should be empty) */
static void _fill_globals(Node *list, SymTable *st)
{
    Node *var = list->firstChild; // curent variable
    st->table = ht_create();
    for (; var != NULL; var = var->nextSibling)
    {
        if (_uses_predef_ident(var))
        {
            fprintf(stderr, "line %d : error : ", var->line_nb);
            fprintf(stderr, "global variable '%s' uses a predefined function "
                            "identifier\n",
                    SECONDCHILD(var)->val.ident);
            exit(EXIT_SEM_ERR);
        }
        _add_varpar(var, st);
    }
}

/* return 1 if this var uses the same identifier as a predefined function */
static int _uses_predef_ident(Node *var)
{
    char *id = SECONDCHILD(var)->val.ident;
    char *names[NB_PREDEF] = {"putchar", "putint", "getchar", "getint"};
    for (int i = 0; i < NB_PREDEF; i++)
        if (strcmp(id, names[i]) == 0)
            return 1;
    return 0;
}

/**
 * @brief Add all the functions (with their symbol tables) to the SymData,
 * including the predefined functions.
 *
 * @param tree abstract syntax tree
 * @param symbols must contain the globals variables
 */
static void _make_fsa(Node *tree, SymData *symbols)
{
    *(symbols->fsa) = fsa_create(NB_PREDEF + _nb_funcs(tree));
    _add_predefined(symbols);
    _fill_fsa_idents(tree, symbols);
    // _check_main(symbols->fsa);
    if (!fsa_contains(symbols->fsa, "main"))
    {
        fprintf(stderr, "error : missing main function\n");
        exit(EXIT_SEM_ERR);
    }
    _fill_fsa_tables(tree, symbols);
}

/**
 * @brief Add the predefined function to a SymData. There must be room for at
 * least `NB_PREDEF` functions.
 *
 * @param symbols pointer to the SymData
 */
static void _add_predefined(SymData *symbols)
{
    char *names[NB_PREDEF] = {"putchar", "putint", "getchar", "getint"};
    Type ret_type[NB_PREDEF] = {T_VOID, T_VOID, T_CHAR, T_NUM};
    FnSym item;
    // add functions
    for (int i = 0; i < NB_PREDEF; i++)
    {
        // need params to make sure function is correctly used
        fs_reset_tables(&item);
        strcpy(item.ident, names[i]);
        item.type = ret_type[i];
        if (i == 0)
            item.params = _putchar_params();
        else if (i == 1)
            item.params = _putint_params();
        fsa_add_if_absent(symbols->fsa, &item);
    }
}

/* create and return a hashtable for putchar's parameters */
static SymTable _putchar_params(void)
{
    Symbol param = {"c", 0, T_CHAR, 0};
    SymTable st = st_create();
    st.table = ht_create();
    st_add_if_absent(&st, &param);
    return st;
}

/* create and return a hashtable for putint's parameters */
static SymTable _putint_params(void)
{
    Symbol param = {"i", 0, T_NUM, 0};
    SymTable st = st_create();
    st.table = ht_create();
    st_add_if_absent(&st, &param);
    return st;
}

/**
 * @brief Add every declared function (with empty tables) to the SymData.
 *
 * @param tree abstract syntax tree
 * @param symbols pointer to the SymData
 */
static void _fill_fsa_idents(Node *tree, SymData *symbols)
{
    Node *func = FIRSTCHILD(SECONDCHILD(tree));
    for (; func != NULL; func = func->nextSibling)
        _add_fs(func, symbols);
}

/**
 * @brief Create the FnSym corresponding a function and add it to a SymData.
 * The FnSym has empty symbol tables (we'll fill them later).
 *
 * @param func node for the function
 * @param symbols pointer to the SymData
 */
static void _add_fs(Node *func, SymData *symbols)
{
    Node *type = SECONDCHILD(FIRSTCHILD(func));
    FnSym item;
    char *name = func->firstChild->firstChild->val.ident;
    fs_reset_tables(&item);
    strncpy(item.ident, name, IDENT_LEN);
    item.type = _get_valid_type(type);
    _check_main(func, &item);
    // check globals
    if (st_contains(symbols->globals, item.ident))
    {
        fprintf(stderr, "line %d : error : ", func->line_nb);
        fprintf(stderr, "'%s' declared both as a variable and as a function\n",
                item.ident);
        exit(EXIT_SEM_ERR);
    }
    // check other function names
    switch (fsa_add_if_absent(symbols->fsa, &(item)))
    {
    case 0:
        fprintf(stderr, "line %d : error : ", func->line_nb);
        fprintf(stderr, "function '%s' redefined\n", item.ident);
        exit(EXIT_SEM_ERR);
    case 2:
        fprintf(stderr, "error : not enough room for functions\n");
        exit(EXIT_OTHER_ERR);
    }
}

/**
 * @brief Throws an error if the main function has a bad prototype. It should
 * be ̀̀`int main(void)`. If func isn't the main function, do nothing.
 *
 * @param func a function node
 * @param fs a FnSym corresponding to func, with ident and type
 */
static void _check_main(Node *func, FnSym *fs)
{
    Node *type = SECONDCHILD(FIRSTCHILD(func));
    Node *params = THIRDCHILD(FIRSTCHILD(func));
    if (strcmp(fs->ident, "main") != 0)
        return;
    if (fs->type != T_NUM)
    {
        fprintf(stderr, "line %d : error : ", type->line_nb);
        fprintf(stderr, "return type of 'main' is not int\n");
        exit(EXIT_SEM_ERR);
    }
    if (params->label != _void_)
    {
        fprintf(stderr, "line %d : error : ", params->line_nb);
        fprintf(stderr, "'main' must take zero argument\n");
        exit(EXIT_SEM_ERR);
    }
}

/**
 * @brief Fills the symbol tables of every function declared in the AST. Throws
 * a semantic error if a local variable reuses a parameter name.
 *
 * @param tree abstract syntax tree
 * @param symbols must contain the predefined functions + every function
 * declared in the AST
 */
static void _fill_fsa_tables(Node *tree, SymData *symbols)
{
    FnSym *fs = symbols->fsa->arr + NB_PREDEF; // predefined functions => +4
    Node *func = FIRSTCHILD(SECONDCHILD(tree));
    for (; func != NULL; func = func->nextSibling)
    {
        _fill_fs_tables(func, fs, symbols);
        fs++;
    }
}

/* fills the symbol tables of one function */
static void _fill_fs_tables(Node *func, FnSym *fs, SymData *symbols)
{
    Node *params = THIRDCHILD(FIRSTCHILD(func));
    Node *locals = FIRSTCHILD(SECONDCHILD(func));
    // fs_reset_tables(fs); // tables are already empty
    if (params->firstChild != NULL)
        _fill_params(params, fs);
    if (locals->firstChild != NULL)
        _fill_locals(locals, fs, symbols);
}

/* fills the parameters symbol tables of one function
 * useless if there is zero param */
static void _fill_params(Node *list, FnSym *fs)
{
    Node *par = list->firstChild; // curent param
    fs->params.table = ht_create();
    for (; par != NULL; par = par->nextSibling)
        _add_varpar(par, &(fs->params));
}

/* fills the local variables symbol tables of one function
 * useless if there is zero local variable
 * params should have been filled before
 * check if a variable reuses a parameter identifier
 * check if initializations are ok */
static void _fill_locals(Node *list, FnSym *fs, SymData *symbols)
{
    Node *var = list->firstChild; // curent variable
    fs->locals.table = ht_create();
    for (; var != NULL; var = var->nextSibling)
    {
        if (_var_in_params(var, fs))
        {
            fprintf(stderr, "line %d : error : ", var->line_nb);
            fprintf(stderr, "local variable reuses a parameter identifier\n");
            exit(EXIT_SEM_ERR);
        }
        if (var->label == var_init)
            check_exp_decl(THIRDCHILD(var), symbols, fs);
        _add_varpar(var, &(fs->locals));
    }
}

/* check if a variable reuses a parameter identifier
 * var should have label var_init or var_no_init */
static int _var_in_params(Node *var, FnSym *fs)
{
    char *ident = SECONDCHILD(var)->val.ident;
    return st_contains(&(fs->params), ident);
}
