#include "../inc/comp_translate.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/macros.h"
#include "../inc/symbol.h"
#include "../inc/symfunc.h"
#include "../inc/symtable.h"
#include "../inc/types.h"

#define NB_REG_ARG 6 // number of args passed in registers

static FILE *out = NULL; // output file

/* location of a symbol */
typedef enum _Location
{
    PARAMS,
    LOCALS,
    GLOBALS
} Location;

/* infos used to access a symbol */
typedef struct _SymAcc
{
    const char *size; // "byte" or "dword"
    const char *reg;  // "r10b" or "r10d"
    const char *base; // "rbp-" or "globals+"
    size_t address;
} SymAcc;

static int _max(int x, int y);
// static void _copy(FILE *src, FILE *dst);
static unsigned int _new_label(void);
static SymAcc _find_symbol(SymData *symbols, FnSym *fs, char *id, int limit);
static void _find_symbol_p1(SymData *symbols, FnSym *fs, char *id, int limit,
                            Symbol *sym, Location *loc);
static SymAcc _find_symbol_p2(FnSym *fs, Symbol *symbol, Location loc);

static void _write_globals(SymData *symbols);
static void _write_text(Node *tree, SymData *symbols);
static void _write_functions(Node *tree, SymData *symbols);
static void _write_func(Node *func, SymData *symbols, FnSym *fs);
static void _copy_params(Node *func, FnSym *fs);
static void _init_locals(SymData *symbols, FnSym *fs, Node *func);
static void _init_local(SymData *symbols, FnSym *fs, Node *var, int index);
static void _write_stmts(Node *list, SymData *symbols, FnSym *fs);
static void _write_stmt(Node *stmt, SymData *symbols, FnSym *fs);
static void _write_if(Node *node, SymData *symbols, FnSym *fs);
static void _write_ifelse(Node *node, SymData *symbols, FnSym *fs);
static void _write_while(Node *node, SymData *symbols, FnSym *fs);
static void _write_assign(Node *node, SymData *symbols, FnSym *fs);
static void _write_ret_exp(Node *ret, SymData *symbols, FnSym *fs);
static void _write_call(Node *call, SymData *symbols, FnSym *fs, int limit);
static void _push_call_args(Node *arg, SymData *symbols, FnSym *fs, int limit);
static void _mov_call_args(int nb_args);

static void _write_exp(Node *exp, SymData *symbols, FnSym *fs, int limit);
static void _write_exp_constant(Node *node);
static void _write_exp_id(Node *node, SymData *symbols, FnSym *fs, int limit);
static void _write_op(Node *op, SymData *symbols, FnSym *fs, int limit);
static void _write_eq(Node *op, SymData *symbols, FnSym *fs, int limit);
static void _write_order(Node *op, SymData *symbols, FnSym *fs, int limit);
static void _write_cmp(char *jmp, SymData *symbols, FnSym *fs);
static void _write_and(Node *op, SymData *symbols, FnSym *fs, int limit);
static void _write_or(Node *op, SymData *symbols, FnSym *fs, int limit);
static void _write_not(Node *op, SymData *symbols, FnSym *fs, int limit);
static void _write_pm(Node *op, SymData *symbols, FnSym *fs, int limit);
static void _write_exp_call(Node *call, SymData *syms, FnSym *fs, int limit);

/* PUBLIC FUNCTIONS */

void translate(Node *tree, SymData *symbols)
{
    assert(tree != NULL);
    assert(symbols != NULL);
    out = fopen("./_anonymous.asm", "w"); // ./asm/_anonymous.asm
    if (out == NULL)
    {
        fprintf(stderr, "could not open _anonymous.asm\n");
        exit(EXIT_IO_ERR);
    }
    _write_globals(symbols);
    _write_text(tree, symbols);
    fclose(out);
}

/* PRIVATE FUNCTIONS */

static int _max(int x, int y)
{
    return (x < y) ? y : x;
}

// /* copy src to dst */
// static void _copy(FILE *src, FILE *dst)
// {
//     int c;
//     while ((c = fgetc(src)) != EOF)
//         if (fputc(c, dst) == EOF)
//         {
//             fprintf(stderr, "error copying file\n");
//             exit(EXIT_IO_ERR);
//         }
// }

/* returns a different number each time */
static unsigned int _new_label(void)
{
    static unsigned int label = 0;
    label++;
    return label;
}

/* search a symbol in a function's tables or in globals and return a
 * corresponding SymAcc */
static SymAcc _find_symbol(SymData *symbols, FnSym *fs, char *id, int limit)
{
    Symbol symbol;
    Location loc;
    _find_symbol_p1(symbols, fs, id, limit, &symbol, &loc);
    return _find_symbol_p2(fs, &symbol, loc);
}

/* search a symbol in a function's tables or in globals (error if not found)
 * local var index must be smaller that limit (if limit != -1)
 * write result in *sym and location in *loc */
static void _find_symbol_p1(SymData *symbols, FnSym *fs, char *id, int limit,
                            Symbol *sym, Location *loc)
{
    SymTable *tables[3] = {&(fs->params), &(fs->locals), symbols->globals};
    int i;
    for (i = 0; i < 3; i++)
    {
        *sym = st_get(tables[i], id);
        if (!symbol_is_valid(sym))
            continue;
        if (i == 1 && limit != -1 && sym->index >= limit)
            continue; // symbol found but it's an undeclared local variable
        break;
    }
    switch (i)
    {
        case 0: *loc = PARAMS;  break;
        case 1: *loc = LOCALS;  break;
        case 2: *loc = GLOBALS; break;
        default:
            fprintf(stderr, "translate : symbol '%s' not found\n", id);
            exit(EXIT_OTHER_ERR);
    }
}

/* return SymAcc corresponding to a symbol found at given location */
static SymAcc _find_symbol_p2(FnSym *fs, Symbol *symbol, Location loc)
{
    SymAcc sl;
    switch (loc)
    {
    case PARAMS:
        sl.base = "rbp-";
        sl.address = fs->params.size - symbol->address;
        break;
    case LOCALS:
        sl.base = "rbp-";
        sl.address = fs->params.size + fs->locals.size - symbol->address;
        break;
    case GLOBALS:
        sl.base = "globals+";
        sl.address = symbol->address;
        break;
    }
    sl.size = (symbol->type == T_CHAR) ? "byte" : "dword";
    sl.reg = (symbol->type == T_CHAR) ? "r10b" : "r10d";
    return sl;
}

/* allocate memory for globals, if any */
static void _write_globals(SymData *symbols)
{
    if (st_is_empty(symbols->globals))
        return;
    fprintf(out, "section .bss\n");
    fprintf(out, "globals: resb %lu\n", symbols->globals->size);
}

/* write text section */
static void _write_text(Node *tree, SymData *symbols)
{
    // FILE *start = fopen("./asm/start.asm", "r");
    // if (start == NULL)
    // {
    //     fprintf(stderr, "could not open start.asm\n");
    //     exit(EXIT_IO_ERR);
    // }
    // _copy(start, out);
    // fclose(start);
    fprintf(out, "%s", ASM_START_STRING);
    _write_functions(tree, symbols);
}

static void _write_functions(Node *tree, SymData *symbols)
{
    Node *func = FIRSTCHILD(SECONDCHILD(tree));
    FnSym *fs = symbols->fsa->arr + NB_PREDEF; // skip predefined functions
    for (; func != NULL; func = func->nextSibling)
    {
        _write_func(func, symbols, fs);
        fs++;
    }
}

/* fs = function to write */
static void _write_func(Node *func, SymData *symbols, FnSym *fs)
{
    Node *body = SECONDCHILD(SECONDCHILD(func));
    size_t data_size = fs->params.size + fs->locals.size;
    fprintf(out, "func_%s:\n", fs->ident);
    fprintf(out, "push rbp\n");
    fprintf(out, "mov rbp, rsp\n");
    fprintf(out, "sub rsp, %lu\n", data_size); // alloc params + locals
    _copy_params(func, fs);
    _init_locals(symbols, fs, func);
    _write_stmts(body, symbols, fs);
    fprintf(out, "labelRet_%s:\n", fs->ident);
    fprintf(out, "mov rsp, rbp\n");
    fprintf(out, "pop rbp\n");
    fprintf(out, "ret\n");
}

/* fs = current function */
static void _copy_params(Node *func, FnSym *fs)
{
    Node *params = THIRDCHILD(FIRSTCHILD(func));
    Node *par = params->firstChild; // current param
    Symbol sym;                     // current param's symbol
    SymAcc sl;                      // current param's access info
    int index = 0;                  // current param's index in table
    const char *reg1[NB_REG_ARG] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
    const char *reg4[NB_REG_ARG] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
    // iterate over params, in order
    for (; par != NULL; par = par->nextSibling)
    {
        sym = st_get(&(fs->params), SECONDCHILD(par)->val.ident);
        if (!symbol_is_valid(&sym))
        {
            fprintf(stderr, "translate : couldn't copy parameter\n");
            exit(EXIT_OTHER_ERR);
        }
        sl.address = fs->params.size - sym.address;
        sl.size = (sym.type == T_CHAR) ? "byte" : "dword";
        if (index < NB_REG_ARG)
            sl.reg = (sym.type == T_CHAR) ? reg1[index] : reg4[index];
        else
        {
            sl.reg = (sym.type == T_CHAR) ? "r10b" : "r10d";
            fprintf(out, "mov r10, qword [rbp+%d]\n",
                    8 * (2 + index - NB_REG_ARG));
        }
        fprintf(out, "mov %s [rbp-%lu], %s\n", sl.size, sl.address, sl.reg);
        index++;
    }
}

/* write code to init func/fs local variables */
static void _init_locals(SymData *symbols, FnSym *fs, Node *func)
{
    Node *locals = FIRSTCHILD(SECONDCHILD(func)), *var;
    int index = 0; // current var's index in table
    for (var = locals->firstChild; var != NULL; var = var->nextSibling)
    {
        if (var->label == var_init)
            _init_local(symbols, fs, var, index);
        index++;
    }
}

/* write code to init one local variable; var->label == var_init */
static void _init_local(SymData *symbols, FnSym *fs, Node *var, int index)
{
    Symbol sym; // current var's symbol
    SymAcc sl;  // current var's access info
    sym = st_get(&(fs->locals), SECONDCHILD(var)->val.ident);
    if (!symbol_is_valid(&sym))
    {
        fprintf(stderr, "translate : couldn't initialize local var\n");
        exit(EXIT_OTHER_ERR);
    }
    sl.address = fs->params.size + fs->locals.size - sym.address;
    sl.size = (sym.type == T_CHAR) ? "byte" : "dword";
    sl.reg = (sym.type == T_CHAR) ? "r10b" : "r10d";
    _write_exp(THIRDCHILD(var), symbols, fs, index);
    fprintf(out, "pop r10\n");
    fprintf(out, "mov %s [rbp-%lu], %s\n", sl.size, sl.address, sl.reg);
}

static void _write_stmts(Node *list, SymData *symbols, FnSym *fs)
{
    Node *s = list->firstChild; // s = current statement
    for (; s != NULL; s = s->nextSibling)
        _write_stmt(s, symbols, fs);
}

static void _write_stmt(Node *stmt, SymData *symbols, FnSym *fs)
{
    switch (stmt->label)
    {
    case _if_:          _write_if(stmt, symbols, fs);                   break;
    case if_else:       _write_ifelse(stmt, symbols, fs);               break;
    case _while_:       _write_while(stmt, symbols, fs);                break;
    case binop_assign:  _write_assign(stmt, symbols, fs);               break;
    case return_void:   fprintf(out, "jmp labelRet_%s\n", fs->ident);   break;
    case return_exp:    _write_ret_exp(stmt, symbols, fs);              break;
    case call_func:     _write_call(stmt, symbols, fs, -1);             break;
    case stmt_sequence: _write_stmts(stmt, symbols, fs);                break;
    case empty_stmt:                                                    break;
    default:
        fprintf(stderr, "line %d : error : ", stmt->line_nb);
        fprintf(stderr, "unknown statement label : %d\n", stmt->label);
        exit(EXIT_OTHER_ERR);
    }
}

/* node->label == _if_ */
static void _write_if(Node *node, SymData *symbols, FnSym *fs)
{
    unsigned int label = _new_label();
    _write_exp(FIRSTCHILD(node), symbols, fs, -1);
    fprintf(out, "pop r10\n");
    fprintf(out, "test r10, r10\n");
    fprintf(out, "jz labelAfter_%X\n", label);
    _write_stmt(SECONDCHILD(node), symbols, fs);
    fprintf(out, "labelAfter_%X:\n", label);
}

/* node->label == if_else */
static void _write_ifelse(Node *node, SymData *symbols, FnSym *fs)
{
    unsigned int label = _new_label();
    _write_exp(FIRSTCHILD(node), symbols, fs, -1);
    fprintf(out, "pop r10\n");
    fprintf(out, "test r10, r10\n");
    fprintf(out, "jz labelElse_%X\n", label);
    _write_stmt(SECONDCHILD(node), symbols, fs);
    fprintf(out, "jmp labelAfter_%X\n", label);
    fprintf(out, "labelElse_%X:\n", label);
    _write_stmt(THIRDCHILD(node), symbols, fs);
    fprintf(out, "labelAfter_%X:\n", label);
}

/* node->label == _while_ */
static void _write_while(Node *node, SymData *symbols, FnSym *fs)
{
    unsigned int label = _new_label();

    fprintf(out, "labelStart_%X:\n", label);
    _write_exp(FIRSTCHILD(node), symbols, fs, -1);
    fprintf(out, "pop r10\n");
    fprintf(out, "test r10, r10\n");
    fprintf(out, "jz labelAfter_%X\n", label);
    _write_stmt(SECONDCHILD(node), symbols, fs);
    fprintf(out, "jmp labelStart_%X\n", label);
    fprintf(out, "labelAfter_%X:\n", label);
}

/* node->label == binop_assign */
static void _write_assign(Node *node, SymData *symbols, FnSym *fs)
{
    SymAcc sl = _find_symbol(symbols, fs, node->firstChild->val.ident, -1);
    _write_exp(SECONDCHILD(node), symbols, fs, -1);
    fprintf(out, "pop r10\n");
    fprintf(out, "mov %s [%s%lu], %s\n", sl.size, sl.base, sl.address, sl.reg);
}

/* node->label == _write_ret_exp */
static void _write_ret_exp(Node *ret, SymData *symbols, FnSym *fs)
{
    // return (void exp) is allowed to we need to check fs's return type
    if (fs->type != T_VOID)
    {
        _write_exp(ret->firstChild, symbols, fs, -1);
        fprintf(out, "pop rax\n");
    }
    fprintf(out, "jmp labelRet_%s\n", fs->ident);
}

/* node->label == call_func; fs = caller*/
static void _write_call(Node *call, SymData *symbols, FnSym *fs, int limit)
{
    char *callee_id = call->firstChild->val.ident;
    Node *args = SECONDCHILD(call);
    int nb_args = nbChild(args);
    int nb_args_stack = _max(0, nb_args - NB_REG_ARG);
    // Align stack
    fprintf(out, "mov r11, rsp\n");
    fprintf(out, "sub rsp, 8\n");
    fprintf(out, "and rsp, -16\n");
    fprintf(out, "mov qword [rsp], r11\n");
    // Push dummy value to preserve stack alignment
    if (nb_args > NB_REG_ARG && (nb_args & 1) == 1)
    {
        fprintf(out, "push 0\n");
        nb_args_stack++;
    }
    // Push args from last to first
    _push_call_args(args->firstChild, symbols, fs, limit);
    // Move 6 first args to registers
    _mov_call_args(nb_args);
    fprintf(out, "call func_%s\n", callee_id);
    // Restore stack
    if (nb_args_stack != 0)
        fprintf(out, "add rsp, %d\n", 8 * nb_args_stack);
    fprintf(out, "pop rsp\n");
}

/* push the arg and its siblings, from last to first */
static void _push_call_args(Node *arg, SymData *symbols, FnSym *fs, int limit)
{
    if (arg == NULL)
        return;
    _push_call_args(arg->nextSibling, symbols, fs, limit);
    _write_exp(arg, symbols, fs, limit);
}

/* move args from stack to rdi, rsi, rdx, rcx, r8, r9 */
static void _mov_call_args(int nb_args)
{
    const char *reg[NB_REG_ARG] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    for (int i = 0; i < nb_args && i < NB_REG_ARG; i++)
        fprintf(out, "pop %s\n", reg[i]);
}

static void _write_exp(Node *exp, SymData *symbols, FnSym *fs, int limit)
{
    switch (exp->label)
    {
    case character:
    case number:        _write_exp_constant(exp);                   break;
    case ident:         _write_exp_id(exp, symbols, fs, limit);     break;
    case binop_and:     _write_and(exp, symbols, fs, limit);        break;
    case binop_or:      _write_or(exp, symbols, fs, limit);         break;
    case binop_eq:      _write_eq(exp, symbols, fs, limit);         break;
    case binop_order:   _write_order(exp, symbols, fs, limit);      break;
    case binop_addsub:
    case binop_divstar: _write_op(exp, symbols, fs, limit);         break;
    case unop_not:      _write_not(exp, symbols, fs, limit);        break;
    case unop_pm:       _write_pm(exp, symbols, fs, limit);         break;
    case call_func:     _write_exp_call(exp, symbols, fs, limit);   break;
    default:
        fprintf(stderr, "line %d : error : ", exp->line_nb);
        fprintf(stderr, "unknown expression label : %d\n", exp->label);
        exit(EXIT_OTHER_ERR);
    }
}

/* node->label is character or number */
static void _write_exp_constant(Node *node)
{
    int constant;
    switch (node->label)
    {
        case character: constant = node->val.byte;  break;
        case number:    constant = node->val.num;   break;
        default:
            fprintf(stderr, "translate : impossible switch case\n");
            exit(EXIT_OTHER_ERR);
    }
    fprintf(out, "push 0x%X\n", constant);
}

/* node->label == ident */
static void _write_exp_id(Node *node, SymData *symbols, FnSym *fs, int limit)
{
    // write 4 bytes to register => erase all 8 bytes
    // write 1 byte => erase only 1 byte => need to reset register before mov
    SymAcc sl = _find_symbol(symbols, fs, node->val.ident, limit);
    if (strcmp(sl.size, "byte") == 0)
        fprintf(out, "xor r10, r10\n");
    fprintf(out, "mov %s, %s [%s%lu]\n", sl.reg, sl.size, sl.base, sl.address);
    fprintf(out, "push r10\n");
}

/* op->label is binop_addsub or binop_divstar */
static void _write_op(Node *op, SymData *symbols, FnSym *fs, int limit)
{
    _write_exp(FIRSTCHILD(op), symbols, fs, limit);
    _write_exp(SECONDCHILD(op), symbols, fs, limit);
    fprintf(out, "pop r11\n"); // second operand
    fprintf(out, "pop rax\n"); // first operand
    switch (op->val.byte)
    {
    case '+':
        fprintf(out, "add rax, r11\n");
        break;
    case '-':
        fprintf(out, "sub rax, r11\n");
        break;
    case '*':
        fprintf(out, "imul rax, r11\n");
        break;
    case '/':
        fprintf(out, "xor rdx, rdx\n"); // rdx = 0
        fprintf(out, "idiv r11\n");     // rax = (rdx:rax) / r11
        break;
    case '%':
        fprintf(out, "xor rdx, rdx\n");
        fprintf(out, "idiv r11\n");
        fprintf(out, "push rdx\n");
        return;
    default:
        fprintf(stderr, "translate : unknown operator %d\n", op->val.byte);
        exit(EXIT_OTHER_ERR);
    }
    fprintf(out, "push rax\n");
}

/* op->label == binop_eq */
static void _write_eq(Node *op, SymData *symbols, FnSym *fs, int limit)
{
    char *jmp_type;
    // this assumes that op->val is == or != (implied by lexer)
    jmp_type = (op->val.comp[0] == '=') ? "je" : "jne";
    _write_exp(FIRSTCHILD(op), symbols, fs, limit);
    _write_exp(SECONDCHILD(op), symbols, fs, limit);
    _write_cmp(jmp_type, symbols, fs);
}

/* op->label == binop_order */
static void _write_order(Node *op, SymData *symbols, FnSym *fs, int limit)
{
    char jmp_type[4] = "j\0\0\0";
    // this assumes that op->val is <, >, <= or >= (implied by lexer)
    jmp_type[1] = (op->val.comp[0] == '<') ? 'l' : 'g';
    jmp_type[2] = (op->val.comp[1] == '=') ? 'e' : '\0';
    _write_exp(FIRSTCHILD(op), symbols, fs, limit);
    _write_exp(SECONDCHILD(op), symbols, fs, limit);
    _write_cmp(jmp_type, symbols, fs);
}

/* compare 2 top values in stack
 * used by eq and order
 * jmp = je, jg, jle... */
static void _write_cmp(char *jmp, SymData *symbols, FnSym *fs)
{
    unsigned int label = _new_label();
    fprintf(out, "pop r11\n"); // second operand
    fprintf(out, "pop r10\n"); // first operand
    fprintf(out, "cmp r10, r11\n");
    fprintf(out, "%s labelYes_%X\n", jmp, label);
    fprintf(out, "push 0\n");
    fprintf(out, "jmp labelAfter_%X\n", label);
    fprintf(out, "labelYes_%X:\n", label);
    fprintf(out, "push 1\n");
    fprintf(out, "labelAfter_%X:\n", label);
}

/* op->label == binop_and, lazy evaluation */
static void _write_and(Node *op, SymData *symbols, FnSym *fs, int limit)
{
    unsigned int label = _new_label();
    _write_exp(FIRSTCHILD(op), symbols, fs, limit);
    fprintf(out, "pop r10\n");
    fprintf(out, "test r10, r10\n");
    fprintf(out, "jz labelElse_%X\n", label);
    _write_exp(SECONDCHILD(op), symbols, fs, limit);
    fprintf(out, "jmp labelAfter_%X\n", label);
    fprintf(out, "labelElse_%X:\n", label);
    fprintf(out, "push 0\n");
    fprintf(out, "labelAfter_%X:\n", label);
}

/* op->label == binop_or, lazy evaluation */
static void _write_or(Node *op, SymData *symbols, FnSym *fs, int limit)
{
    unsigned int label = _new_label();
    _write_exp(FIRSTCHILD(op), symbols, fs, limit);
    fprintf(out, "pop r10\n");
    fprintf(out, "test r10, r10\n");
    fprintf(out, "jz labelElse_%X\n", label);
    fprintf(out, "push 1\n");
    fprintf(out, "jmp labelAfter_%X\n", label);
    fprintf(out, "labelElse_%X:\n", label);
    _write_exp(SECONDCHILD(op), symbols, fs, limit);
    fprintf(out, "labelAfter_%X:\n", label);
}

/* op->label == unop_not */
static void _write_not(Node *op, SymData *symbols, FnSym *fs, int limit)
{
    _write_exp(FIRSTCHILD(op), symbols, fs, limit);
    fprintf(out, "pop r10\n");
    fprintf(out, "xor r11, r11\n");  // mov r11, 0
    fprintf(out, "test r10, r10\n"); // cmp r10, 0
    fprintf(out, "setz r11b\n");     // r11b = (1 if r10 == 0 else 0)
    fprintf(out, "push r11\n");
}

/* op->label == unop_pm */
static void _write_pm(Node *op, SymData *symbols, FnSym *fs, int limit)
{
    if (op->val.byte == '+') // convert char to int ?
        return;
    _write_exp(FIRSTCHILD(op), symbols, fs, limit);
    fprintf(out, "pop r10\n");
    fprintf(out, "neg r10\n");
    fprintf(out, "push r10\n");
}

/* call->label == call_func; fs = caller */
static void _write_exp_call(Node *call, SymData *syms, FnSym *fs, int limit)
{
    char *callee_id = call->firstChild->val.ident;
    FnSym *callee = fsa_get(syms->fsa, callee_id);
    if (callee == NULL)
    {
        // check_decl should already have thrown an error
        fprintf(stderr, "translate : function '%s' not found\n", callee_id);
        exit(EXIT_OTHER_ERR);
    }
    _write_call(call, syms, fs, limit);
    // callee is void => don't push
    if (callee->type != T_VOID)
        fprintf(out, "push rax\n");
}
