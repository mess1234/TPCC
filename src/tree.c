#include "../inc/tree.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int line_nb; // from lexer

/* PRIVATE FUNCTIONS */

static const char *StringFromLabel[] = {
    MAP_LABELS(TO_STRING)
    // list all other node labels, if any
    // The list must coincide with the string array in tree.c
    // To avoid listing them twice, see https://stackoverflow.com/a/10966395
};

static void _printChar(char c)
{
    switch (c)
    {
        case '\'':  printf("'\\''");    break;
        case '\\':  printf("'\\\\'");   break;
        case '\n':  printf("'\\n'");    break;
        case '\t':  printf("'\\t'");    break;
        default:    printf("'%c'", c);  break;
    }
}

static void _printNode(Node *node)
{
    label_t label = node->label;
    switch (label)
    {
        // Constant strings
        case _void_:        printf("void");                         break;
        case _if_:          printf("if");                           break;
        case if_else:       printf("if_else");                      break;
        case _while_:       printf("while");                        break;
        case return_exp:
        case return_void:   printf("return");                       break;
        case binop_and:     printf("and");                          break;
        case binop_or:      printf("or");                           break;
        case unop_not:      putchar('!');                           break;
        case binop_assign:  putchar('=');                           break;
        // Variable strings => need more info
        case type:          printf("%s", node->val.type);           break;
        case binop_eq:
        case binop_order:   printf("%s", node->val.comp);           break;
        case unop_pm:
        case binop_addsub:
        case binop_divstar: printf("%c", node->val.byte);           break;
        case number:        printf("%d", node->val.num);            break;
        case character:     _printChar(node->val.byte);             break;
        case ident:         printf("%s", node->val.ident);          break;
        default:            printf("%s", StringFromLabel[label]);   break;
    }
}

/* PUBLIC FUNCTIONS */

Node *makeNode(label_t label)
{
    Node *node = malloc(sizeof(Node));
    if (!node)
    {
        printf("Run out of memory\n");
        exit(-1);
    }
    node->label = label;
    node->firstChild = node->nextSibling = NULL;
    node->line_nb = line_nb;
    return node;
}

void addSibling(Node *node, Node *sibling)
{
    Node *curr = node;
    while (curr->nextSibling != NULL)
        curr = curr->nextSibling;
    curr->nextSibling = sibling;
}

void addChild(Node *parent, Node *child)
{
    if (parent->firstChild == NULL)
        parent->firstChild = child;
    else
        addSibling(parent->firstChild, child);
}

void deleteTree(Node *node)
{
    if (node->firstChild)
        deleteTree(node->firstChild);
    if (node->nextSibling)
        deleteTree(node->nextSibling);
    free(node);
}

void printTree(Node *node)
{
    static bool rightmost[128]; // tells if node is rightmost sibling
    static int dep = 0;         // depth of current node
    Node *child;
    int i;
    // 2502 = vertical line
    for (i = 1; i < dep; i++)
        printf(rightmost[i] ? "    " : "\u2502   ");
    // 2514 = L; 2500 = horizontal; 251c = vertical + right horizontal
    if (dep > 0)
        printf(rightmost[dep] ? "\u2514\u2500\u2500 " : "\u251c\u2500\u2500 ");
    // printf("%s", StringFromLabel[node->label]);
    _printNode(node);
    putchar('\n');
    dep++;
    for (child = node->firstChild; child != NULL; child = child->nextSibling)
    {
        rightmost[dep] = (child->nextSibling) ? false : true;
        printTree(child);
    }
    dep--;
}

int nbChild(Node *node)
{
    Node *curr = node->firstChild;
    int len = 0;
    for (; curr != NULL; curr = curr->nextSibling)
        len++;
    return len;
}

/* helps to build the tree */
void setLineTypes(Node *line, char *type)
{
    for (; line != NULL; line = line->nextSibling)
    {
        if (line->firstChild->val.type[0] != '\0')
            return;
        strncpy(line->firstChild->val.type, type, 5); // type is char[5]
    }
}