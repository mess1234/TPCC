#define _GNU_SOURCE // for getopt_long
#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "../inc/comp_check_decl.h"
#include "../inc/comp_check_types.h"
#include "../inc/comp_init_symbols.h"
#include "../inc/comp_translate.h"
#include "../inc/macros.h"
#include "../inc/symfunc.h"
#include "../inc/symtable.h"
#include "../inc/tree.h"
#include "../obj/parser.h" // for yyparse

#define FLAG_HELP 1    // 0b0001
#define FLAG_TREE 2    // 0b0010
#define FLAG_SYMBOLS 4 // 0b0100

Node *syntax_tree; // global for parser

void compile(Node *tree, int show_tables);
void set_flags(int argc, char *const argv[], int *flags);
void help(void);

int main(int argc, char **argv)
{
    int parse_result, flags = 0; // flags = options flags, bitwise-or'd
    // Read options
    set_flags(argc, argv, &flags);
    if (flags & FLAG_HELP)
        help();
    // Parse input and build tree
    syntax_tree = makeNode(start);
    parse_result = yyparse();
    switch (parse_result)
    {
        case 1:     exit(EXIT_SYN_ERR);     // invalid input
        case 2:     exit(EXIT_OTHER_ERR);   // memory exhaustion
        default:    break;                  // parsing was successful
    }
    // Do stuff with tree (if parsing succeeded)
    if (flags & FLAG_TREE)
        printTree(syntax_tree);
    compile(syntax_tree, flags & FLAG_SYMBOLS);
    deleteTree(syntax_tree);
    return EXIT_NO_ERR;
}

/**
 * @brief Check for semantic errors in AST and translate it to nasm
 *
 * @param tree the AST
 * @param show_tables if non zero, print all symbol tables to stdout
 */
void compile(Node *tree, int show_tables)
{
    SymTable globals;
    FnSymArr functions;
    SymData symbols = {&globals, &functions};
    assert(tree != NULL);
    make_symbol_tables(tree, &symbols);
    check_declarations(tree, &symbols);
    check_types(tree, &symbols);
    translate(tree, &symbols);
    if (show_tables)
    {
        printf("Global variables :\n");
        st_print(&globals);
        putchar('\n');
        fsa_print(&functions);
    }
    st_free(&globals);
    fsa_free(&functions);
}

/**
 * @brief Parse the command line arguments stored in argv. Set flags for
 * command line options. Throw an error if an option is not recognized.
 *
 * @param argc argument count
 * @param argv argument vector
 * @param flags pointer to the int holding the options flags (bitwise-or'd)
 */
void set_flags(int argc, char *const argv[], int *flags)
{
    int optval, last_opt;
    struct option longopts[] = {
        {"help", no_argument, &last_opt, FLAG_HELP},
        {"tree", no_argument, &last_opt, FLAG_TREE},
        {"symtabs", no_argument, &last_opt, FLAG_SYMBOLS},
        {0, 0, 0, 0}
    };
    while ((optval = getopt_long(argc, argv, "hts", longopts, NULL)) != -1)
        switch (optval)
        {
            case 0:     *flags |= last_opt;     break;
            case 'h':   *flags |= FLAG_HELP;    break;
            case 't':   *flags |= FLAG_TREE;    break;
            case 's':   *flags |= FLAG_SYMBOLS; break;
            default:    exit(EXIT_OTHER_ERR);
        }
}

/**
 * @brief Print tpcc help and exit
 *
 */
void help(void)
{
    printf("%s", HELP_STRING);
    exit(EXIT_NO_ERR);
}