#include "../inc/comp_check_decl.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../inc/macros.h"
#include "../inc/symtable.h"

static void _check_function(Node *func, SymData *symbols, FnSym *fs);
static void _check_statements(Node *stmts, SymData *symbols, FnSym *fs);
static void _check_statement(Node *stmt, SymData *symbols, FnSym *fs);
static void _check_exp(Node *exp, SymData *symbols, FnSym *fs);
static void _check_var_par(Node *id_node, SymTable *globals, FnSym *fs);
static void _check_func_call(Node *call_node, SymData *symbols, FnSym *fs);
static void _check_func_id(Node *id_node, FnSymArr *fsa);
static void _check_func_args(Node *args, SymData *symbols, FnSym *fs);

/* PUBLIC FUNCTIONS */

void check_declarations(Node *tree, SymData *symbols)
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

void check_exp_decl(Node *exp, SymData *symbols, FnSym *fs)
{
    assert(exp != NULL);
    assert(symbols != NULL);
    assert(fs != NULL);
    _check_exp(exp, symbols, fs);
}

/* PRIVATE FUNCTIONS */

/* check one function
 * *fs corresponds to the current function */
static void _check_function(Node *func, SymData *symbols, FnSym *fs)
{
    Node *statements = SECONDCHILD(SECONDCHILD(func));
    _check_statements(statements, symbols, fs);
}

/* check a list of statements
 * *fs corresponds to the function surrounding theses statements */
static void _check_statements(Node *stmts, SymData *symbols, FnSym *fs)
{
    Node *stmt = FIRSTCHILD(stmts);
    for (; stmt != NULL; stmt = stmt->nextSibling)
        _check_statement(stmt, symbols, fs);
}

/* check one statement
 * *fs corresponds to the function surrounding this statement */
static void _check_statement(Node *stmt, SymData *symbols, FnSym *fs)
{
    switch (stmt->label)
    {
    case _if_:
    case _while_:
        _check_exp(FIRSTCHILD(stmt), symbols, fs);
        _check_statement(SECONDCHILD(stmt), symbols, fs);
        break;
    case if_else:
        _check_exp(FIRSTCHILD(stmt), symbols, fs);
        _check_statement(THIRDCHILD(stmt), symbols, fs);
        break;
    case binop_assign:
        _check_var_par(FIRSTCHILD(stmt), symbols->globals, fs);
        _check_exp(SECONDCHILD(stmt), symbols, fs);
        break;
    case return_exp:
        _check_exp(FIRSTCHILD(stmt), symbols, fs);
        break;
    case call_func:
        _check_func_call(stmt, symbols, fs);
        break;
    case stmt_sequence:
        _check_statements(stmt, symbols, fs);
        break;
    case return_void:
    case empty_stmt:
        break;
    default:
        fprintf(stderr, "line %d : error : ", stmt->line_nb);
        fprintf(stderr, "unknown statement label : %d\n", stmt->label);
        exit(EXIT_OTHER_ERR);
    }
}

/* check one expression */
static void _check_exp(Node *exp, SymData *symbols, FnSym *fs)
{
    switch (exp->label)
    {
    case character:
    case number:
        break;
    case ident:
        _check_var_par(exp, symbols->globals, fs);
        break;
    case binop_and:
    case binop_or:
    case binop_eq:
    case binop_order:
    case binop_addsub:
    case binop_divstar:
        _check_exp(FIRSTCHILD(exp), symbols, fs);
        _check_exp(SECONDCHILD(exp), symbols, fs);
        break;
    case unop_not:
    case unop_pm:
        _check_exp(FIRSTCHILD(exp), symbols, fs);
        break;
    case call_func:
        _check_func_call(exp, symbols, fs);
        break;
    default:
        fprintf(stderr, "line %d : error : ", exp->line_nb);
        fprintf(stderr, "unknown expression label : %d\n", exp->label);
        exit(EXIT_OTHER_ERR);
    }
}

/* check if an identifier has been declared, either as a parameter, a local
 * variable, or a global variable */
static void _check_var_par(Node *id_node, SymTable *globals, FnSym *fs)
{
    char *id = id_node->val.ident;
    SymTable *tables[3] = {&(fs->params), &(fs->locals), globals};
    for (int i = 0; i < 3; i++)
        if (st_contains(tables[i], id))
            return;
    fprintf(stderr, "line %d : error : ", id_node->line_nb);
    fprintf(stderr, "'%s' undeclared\n", id);
    exit(EXIT_SEM_ERR);
}

/* check if a function has been declared and if the symbols used in its
 * arguments have also been delcared
 * check that the callee doesn't use the ident of a param/local var
 * fs = FnSym of the calling function (not the callee) */
static void _check_func_call(Node *call_node, SymData *symbols, FnSym *fs)
{
    char *calle_id = call_node->firstChild->val.ident;
    if (st_contains(&(fs->params), calle_id) ||
        st_contains(&(fs->locals), calle_id))
    {
        fprintf(stderr, "line %d : error : called object '%s' is a parameter "
                        "or local variable\n", call_node->line_nb, calle_id);
        exit(EXIT_SEM_ERR);
    }
    _check_func_id(FIRSTCHILD(call_node), symbols->fsa);
    _check_func_args(SECONDCHILD(call_node), symbols, fs);
}

static void _check_func_id(Node *id_node, FnSymArr *fsa)
{
    char *id = id_node->val.ident;
    if (fsa_contains(fsa, id))
        return;
    fprintf(stderr, "line %d : error : ", id_node->line_nb);
    fprintf(stderr, "function '%s' undeclared\n", id);
    exit(EXIT_SEM_ERR);
}

static void _check_func_args(Node *args, SymData *symbols, FnSym *fs)
{
    Node *arg = FIRSTCHILD(args);
    for (; arg != NULL; arg = arg->nextSibling)
        _check_exp(arg, symbols, fs);
}
