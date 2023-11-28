#include "../inc/comp_check_types.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../inc/macros.h"
#include "../inc/symfunc.h"
#include "../inc/types.h"

static void _check_function(Node *func, SymData *symbols, FnSym *fs);
static void _check_locals(Node *func, SymData *symbols, FnSym *fs);
static void _check_stmts(Node *list, SymData *symbols, FnSym *fs);
static void _check_stmt(Node *stmt, SymData *symbols, FnSym *fs);
static void _check_ret_exp(Node *n, SymData *symbols, FnSym *fs);
static void _check_ret_void(FnSym *fs, int line);
static void _check_assign(Node *id, Node *exp, SymData *symbols, FnSym *fs);
static void _check_assign_types(Type var_type, Type exp_type, int line);
static void _prevent_void(Node *exp, SymData *symbols, FnSym *fs);
static Type _get_exp_type(Node *exp, SymData *symbols, FnSym *fs);
static Type _get_symbol_type(Node *id, SymTable *globals, FnSym *fs);
static Type _get_call_type(Node *call, SymData *symbols, FnSym *fs);
static void _check_call_args(Node *call, SymData *syms, FnSym *fs, FnSym *gs);
static void _check_arg_type(int index, Node *arg, SymData *symbols,
                            FnSym *fs, FnSym *gs);

/* PUBLIC FUNCTIONS */

void check_types(Node *tree, SymData *symbols)
{
    Node *func;
    FnSym *fs;
    assert(tree != NULL);
    assert(symbols != NULL);
    func = FIRSTCHILD(SECONDCHILD(tree));
    fs = symbols->fsa->arr + NB_PREDEF; // skip predefined functions
    for (; func != NULL; func = func->nextSibling)
    {
        _check_function(func, symbols, fs);
        fs++;
    }
}

/* PRIVATE FUNCTIONS */

/* fs corresponds to the function being treated */
static void _check_function(Node *func, SymData *symbols, FnSym *fs)
{
    Node *body = SECONDCHILD(SECONDCHILD(func));
    _check_locals(func, symbols, fs);
    _check_stmts(body, symbols, fs); // check body
}

/* check every local variable initialization in a function */
static void _check_locals(Node *func, SymData *symbols, FnSym *fs)
{
    // v = 1st local variable
    Node *v = FIRSTCHILD(FIRSTCHILD(SECONDCHILD(func)));
    for (; v != NULL; v = v->nextSibling)
        if (v->label == var_init)
            _check_assign(SECONDCHILD(v), THIRDCHILD(v), symbols, fs);
}

/* check a list of statements */
static void _check_stmts(Node *list, SymData *symbols, FnSym *fs)
{
    Node *s = list->firstChild; // s = current statement
    for (; s != NULL; s = s->nextSibling)
        _check_stmt(s, symbols, fs);
}

/* check one statement */
static void _check_stmt(Node *stmt, SymData *symbols, FnSym *fs)
{
    switch (stmt->label)
    {
    case _if_:
    case if_else:
    case _while_:
        _prevent_void(FIRSTCHILD(stmt), symbols, fs);
        break;
    case binop_assign:
        _check_assign(FIRSTCHILD(stmt), SECONDCHILD(stmt), symbols, fs);
        break;
    case return_void:
        _check_ret_void(fs, stmt->line_nb);
        break;
    case return_exp:
        _check_ret_exp(stmt, symbols, fs);
        break;
    case call_func:
        _get_call_type(stmt, symbols, fs); // will check call
        break;
    case stmt_sequence:
        _check_stmts(stmt, symbols, fs);
        break;
    case empty_stmt:
        break;
    default:
        fprintf(stderr, "line %d : error : ", stmt->line_nb);
        fprintf(stderr, "unknown statement label : %d\n", stmt->label);
        exit(EXIT_OTHER_ERR);
    }
}

/* n->label must be return_exp */
static void _check_ret_exp(Node *n, SymData *symbols, FnSym *fs)
{
    Type exp_type = _get_exp_type(n->firstChild, symbols, fs);
    if (fs->type == exp_type)
        return;
    if (fs->type == T_VOID || exp_type == T_VOID)
    {
        fprintf(stderr, "line %d : error : ", n->firstChild->line_nb);
        fprintf(stderr, "incorrect type in return statement ");
        fprintf(stderr, "(got '%s', expected '%s')\n",
                type_to_str(exp_type), type_to_str(fs->type));
        exit(EXIT_SEM_ERR);
    }
    if (exp_type == T_CHAR)
        return;
    fprintf(stderr, "line %d : warning : ", n->firstChild->line_nb);
    fprintf(stderr, "conversion from 'int' to 'char' may change value\n");
}

/* ret->label must be return_void
 * used used only for warning */
static void _check_ret_void(FnSym *fs, int line)
{
    if (fs->type == T_VOID)
        return;
    fprintf(stderr, "line %d : error : ", line);
    fprintf(stderr, "'return' with no value, in function returning '%s'\n",
            type_to_str(fs->type));
    exit(EXIT_SEM_ERR);
}

static void _check_assign(Node *id, Node *exp, SymData *symbols, FnSym *fs)
{
    Type var_type = _get_symbol_type(id, symbols->globals, fs);
    Type exp_type = _get_exp_type(exp, symbols, fs);
    _check_assign_types(var_type, exp_type, exp->line_nb);
    // comp_init_symbols module guarantees that var_type is num or char
    // _get_exp_type guarantees that exp_type is num or char or void
}

/* var_type = num or char
 * exp_type = num or char or void
 * line = line of exp */
static void _check_assign_types(Type var_type, Type exp_type, int line)
{
    if (exp_type == T_VOID)
    {
        fprintf(stderr, "line %d : error : ", line);
        fprintf(stderr, "void value in assignment\n");
        exit(EXIT_SEM_ERR);
    }
    if (var_type != T_CHAR || exp_type != T_NUM)
        return;
    fprintf(stderr, "line %d : warning : ", line);
    fprintf(stderr, "conversion from 'int' to 'char' may change value\n");
}

/* check expression validity and compute its type
 * if void, throws a semantic error */
static void _prevent_void(Node *exp, SymData *symbols, FnSym *fs)
{
    if (_get_exp_type(exp, symbols, fs) != T_VOID)
        return; // it must be num or char
    fprintf(stderr, "line %d : error : ", exp->line_nb);
    fprintf(stderr, "void value in forbidden place\n");
    exit(EXIT_SEM_ERR);
}

/* check expression validity and return its type
 * throws error if invalid expression
 * return type is always char or num or void, thanks to comp_init_symbols */
static Type _get_exp_type(Node *exp, SymData *symbols, FnSym *fs)
{
    switch (exp->label)
    {
    case character:
        return T_CHAR;
    case number:
        return T_NUM;
    case ident:
        return _get_symbol_type(exp, symbols->globals, fs);
    case binop_and:
    case binop_or:
    case binop_eq:
    case binop_order:
    case binop_addsub:
    case binop_divstar:
        _prevent_void(FIRSTCHILD(exp), symbols, fs);
        _prevent_void(SECONDCHILD(exp), symbols, fs);
        return T_NUM;
    case unop_not:
    case unop_pm:
        _prevent_void(FIRSTCHILD(exp), symbols, fs);
        return T_NUM;
    case call_func:
        return _get_call_type(exp, symbols, fs);
    default:
        fprintf(stderr, "line %d : error : ", exp->line_nb);
        fprintf(stderr, "unknown expression label : %d\n", exp->label);
        exit(EXIT_OTHER_ERR);
    }
}

/* *node is an ident node
 * search globals last because parameters and local variable can hide a global
 * variable */
static Type _get_symbol_type(Node *node, SymTable *globals, FnSym *fs)
{
    SymTable *tables[3] = {&(fs->params), &(fs->locals), globals};
    Type res;
    char *id = node->val.ident;
    for (int i = 0; i < 3; i++)
    {
        res = st_get(tables[i], id).type;
        // item in table => res == int or char, else res == invalid
        if (res != T_INVALID)
            return res;
    }
    // not found ! should be impossible if comp_check_declarations didn't
    // stop us before
    fprintf(stderr, "line %d : error : ", node->line_nb);
    fprintf(stderr, "symbol '%s' not found\n", id);
    exit(EXIT_OTHER_ERR);
}

/* g is called by f
 * check that g is called with correct args and return its type
 * fs = f's symbols
 * gs = g's symbols */
static Type _get_call_type(Node *call, SymData *symbols, FnSym *fs)
{
    char *id = call->firstChild->val.ident;
    FnSym *gs = fsa_get(symbols->fsa, id);
    if (gs == NULL)
    {
        // not found ! should be impossible if comp_check_declarations didn't
        // stop us before
        fprintf(stderr, "line %d : error : ", call->line_nb);
        fprintf(stderr, "function '%s' not found\n", id);
        exit(EXIT_OTHER_ERR);
    }
    _check_call_args(call, symbols, fs, gs);
    return gs->type;
}

/* fs = caller
 * gs = callee */
static void _check_call_args(Node *call, SymData *syms, FnSym *fs, FnSym *gs)
{
    Node *arg;
    int nb_params, nb_args, i;
    nb_params = gs->params.nb_items;
    nb_args = nbChild(SECONDCHILD(call));
    if (nb_args != nb_params)
    {
        fprintf(stderr, "line %d : error : ", call->line_nb);
        fprintf(stderr, "bad number of arguments (expected %d, got %d)\n",
                nb_params, nb_args);
        exit(EXIT_SEM_ERR);
    }
    arg = FIRSTCHILD(SECONDCHILD(call)); // first arg
    for (i = 0; i < nb_args; i++)
    {
        _check_arg_type(i, arg, syms, fs, gs);
        arg = arg->nextSibling;
    }
}

/* fs = caller
 * gs = callee */
static void _check_arg_type(int index, Node *arg, SymData *symbols,
                            FnSym *fs, FnSym *gs)
{
    Type param_type, arg_type;
    param_type = st_get_index(&(gs->params), index).type;
    if (param_type == T_INVALID)
    {
        fprintf(stderr, "line %d : error : ", arg->line_nb);
        fprintf(stderr, "could not find param of index %d for function '%s'\n",
                index, gs->ident);
        exit(EXIT_OTHER_ERR);
    }
    arg_type = _get_exp_type(arg, symbols, fs);
    _check_assign_types(param_type, arg_type, arg->line_nb);
}
