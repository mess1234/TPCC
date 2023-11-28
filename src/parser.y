%{  /* parser.y */
    #include <stdio.h>
    #include <string.h>
    #include "../inc/tree.h"

    extern int yylex(void);
    extern Node *syntax_tree;
    extern int line_nb;
    extern int char_nb;

    void yyerror(const char *msg);
%}

%union {
    Node *node;
    int num;
    char byte;
    char type[5];
    char comp[3];
    char ident[64];
}

%token <type> TYPE
%token VOID
%token IF ELSE WHILE
%token RETURN
%token AND OR
%token <comp> EQ ORDER
%token <byte> ADDSUB DIVSTAR
%token <num> NUM
%token <byte> CHARACTER
%token <ident> IDENT

%type <node> Prog
%type <node> GlobVars GlobVarsLine
%type <node> LocVars LocVarsLine LocVar
%type <node> DeclFoncts DeclFonct
%type <node> EnTeteFonct Parametres ListTypVar
%type <node> Corps SuiteInstr Instr InstrOpen InstrClosed InstrSimple
%type <node> Exp TB FB M E T F LValue
%type <node> Arguments ListExp

%%

Prog:  GlobVars DeclFoncts {
        Node *glob_node = makeNode(global_vars);
        addChild(glob_node, $1);
        addSibling(glob_node, $2);
        addChild(syntax_tree, glob_node); }
    ;
GlobVars:
       GlobVars TYPE GlobVarsLine ';' {
            Node *new_vars = $3;
            setLineTypes(new_vars, $2);
            $$ = $1;
            if ($$ != NULL)
                addSibling($$, new_vars);
            else
                $$ = new_vars;
            }
    |  { $$ = NULL; }
    ;
GlobVarsLine:
       GlobVarsLine ',' IDENT {
            Node *var_node = makeNode(var_no_init);
            addChild(var_node, makeNode(type));
            addChild(var_node, makeNode(ident));
            /* set values */
            FIRSTCHILD(var_node)->val.type[0] = '\0'; /* undefined type */
            strcpy(SECONDCHILD(var_node)->val.ident, $3);
            $$ = $1;
            addSibling($$, var_node); }
    |  IDENT {
            $$ = makeNode(var_no_init);
            addChild($$, makeNode(type));
            addChild($$, makeNode(ident));
            /* set values */
            FIRSTCHILD($$)->val.type[0] = '\0'; /* undefined type */
            strcpy(SECONDCHILD($$)->val.ident, $1); }
    ;
LocVars:
       LocVars TYPE LocVarsLine ';' {
            Node *new_vars = $3;
            setLineTypes(new_vars, $2);
            $$ = $1;
            if ($$ != NULL)
                addSibling($$, new_vars);
            else
                $$ = new_vars;
            }
    | { $$ = NULL; }
    ;
LocVarsLine:
       LocVarsLine ',' LocVar   { $$ = $1; addSibling($$, $3); }
    |  LocVar                   { $$ = $1; }
    ;
LocVar:
       IDENT '=' Exp {
            $$ = makeNode(var_init);
            addChild($$, makeNode(type));
            addChild($$, makeNode(ident));
            addChild($$, $3);
            /* set values */
            FIRSTCHILD($$)->val.type[0] = '\0'; /* undefined type */
            strcpy(SECONDCHILD($$)->val.ident, $1); }
    |  IDENT {
            $$ = makeNode(var_no_init);
            addChild($$, makeNode(type));
            addChild($$, makeNode(ident));
            /* set values */
            FIRSTCHILD($$)->val.type[0] = '\0'; /* undefined type */
            strcpy(SECONDCHILD($$)->val.ident, $1); }
    ;
DeclFoncts:
       DeclFoncts DeclFonct { $$ = $1; addChild($$, $2); }
    |  DeclFonct            { $$ = makeNode(functions); addChild($$, $1); }
    ;
DeclFonct:
       EnTeteFonct Corps {
            $$ = makeNode(function);
            addChild($$, $1);
            addChild($$, $2); }
    ;
EnTeteFonct:
       TYPE IDENT '(' Parametres ')' {
            Node *id_node = makeNode(ident);
            strcpy(id_node->val.ident, $2);
            Node *type_node = makeNode(type);
            strcpy(type_node->val.type, $1);
            $$ = makeNode(prototype);
            addChild($$, id_node);
            addChild($$, type_node);
            addChild($$, $4); }
    |  VOID IDENT '(' Parametres ')' {
            Node *id_node = makeNode(ident);
            strcpy(id_node->val.ident, $2);
            $$ = makeNode(prototype);
            addChild($$, id_node);
            addChild($$, makeNode(_void_));
            addChild($$, $4); }
    ;
Parametres:
       VOID         { $$ = makeNode(_void_); }
    |  ListTypVar   { $$ = makeNode(params); addChild($$, $1); }
    ;
ListTypVar:
       ListTypVar ',' TYPE IDENT {
            Node *type_node = makeNode(type);
            strcpy(type_node->val.type, $3);
            Node *id_node = makeNode(ident);
            strcpy(id_node->val.ident, $4);
            Node *sib = makeNode(param);
            addChild(sib, type_node);
            addChild(sib, id_node);
            $$ = $1;
            addSibling($$, sib); }
    |  TYPE IDENT {
            Node *type_node = makeNode(type);
            strcpy(type_node->val.type, $1);
            Node *id_node = makeNode(ident);
            strcpy(id_node->val.ident, $2);
            $$ = makeNode(param);
            addChild($$, type_node);
            addChild($$, id_node); }
    ;
Corps: '{' LocVars SuiteInstr '}' {
        Node *loc_node = makeNode(local_vars);
        addChild(loc_node, $2);
        addSibling(loc_node, $3);
        $$ = makeNode(body);
        addChild($$, loc_node); }
    ;
SuiteInstr:
       SuiteInstr Instr { $$ = $1; addChild($$, $2); }
    |                   { $$ = makeNode(stmt_sequence); }
    ;
Instr:
       InstrOpen    { $$ = $1; }
    |  InstrClosed  { $$ = $1; }
    ;
InstrOpen:
       IF '(' Exp ')' Instr {
            $$ = makeNode(_if_);
            addChild($$, $3);
            addChild($$, $5); }
    |  IF '(' Exp ')' InstrClosed ELSE InstrOpen {
            $$ = makeNode(if_else);
            addChild($$, $3);
            addChild($$, $5);
            addChild($$, $7); }
    |  WHILE '(' Exp ')' InstrOpen {
            $$ = makeNode(_while_);
            addChild($$, $3);
            addChild($$, $5); }
    ;
InstrClosed:
       InstrSimple { $$ = $1; }
    |  IF '(' Exp ')' InstrClosed ELSE InstrClosed {
            $$ = makeNode(if_else);
            addChild($$, $3);
            addChild($$, $5);
            addChild($$, $7); }
    |  WHILE '(' Exp ')' InstrClosed {
            $$ = makeNode(_while_);
            addChild($$, $3);
            addChild($$, $5); }
    ;
InstrSimple:
       LValue '=' Exp ';' {
            $$ = makeNode(binop_assign);
            addChild($$, $1);
            addChild($$, $3); }
    |  IDENT '(' Arguments  ')' ';' {
            Node *id_node = makeNode(ident);
            strcpy(id_node->val.ident, $1);
            $$ = makeNode(call_func);
            addChild($$, id_node);
            addChild($$, $3); }
    |  RETURN Exp ';'       { $$ = makeNode(return_exp); addChild($$, $2); }
    |  RETURN ';'           { $$ = makeNode(return_void); }
    |  '{' SuiteInstr '}'   { $$ = $2; }
    |  ';'                  { $$ = makeNode(empty_stmt); }
    ;
Exp :  Exp OR TB    {   $$ = makeNode(binop_or);
                        addChild($$, $1);
                        addChild($$, $3); }
    |  TB           {   $$ = $1; }
    ;
TB  :  TB AND FB    {   $$ = makeNode(binop_and);
                        addChild($$, $1);
                        addChild($$, $3); }
    |  FB           {   $$ = $1; }
    ;
FB  :  FB EQ M      {   $$ = makeNode(binop_eq);
                        strcpy($$->val.comp, $2);
                        addChild($$, $1);
                        addChild($$, $3); }
    |  M            {   $$ = $1; }
    ;
M   :  M ORDER E    {   $$ = makeNode(binop_order);
                        strcpy($$->val.comp, $2);
                        addChild($$, $1);
                        addChild($$, $3); }
    |  E            {   $$ = $1; }
    ;
E   :  E ADDSUB T   {   $$ = makeNode(binop_addsub);
                        $$->val.byte = $2;
                        addChild($$, $1);
                        addChild($$, $3); }
    |  T            {   $$ = $1; }
    ;
T   :  T DIVSTAR F  {   $$ = makeNode(binop_divstar);
                        $$->val.byte = $2;
                        addChild($$, $1);
                        addChild($$, $3); }
    |  F            {   $$ = $1; }
    ;
F   :  ADDSUB F {
            $$ = makeNode(unop_pm);
            $$->val.byte = $1;
            addChild($$, $2); }
    |  '!' F        {$$ = makeNode(unop_not); addChild($$, $2); }
    |  '(' Exp ')'  { $$ = $2; }
    |  NUM          { $$ = makeNode(number); $$->val.num = $1; }
    |  CHARACTER    { $$ = makeNode(character); $$->val.byte = $1; }
    |  LValue       { $$ = $1; }
    |  IDENT '(' Arguments  ')' {
            Node *id_node = makeNode(ident);
            strcpy(id_node->val.ident, $1);
            $$ = makeNode(call_func);
            addChild($$, id_node);
            addChild($$, $3); }
    ;
LValue:
       IDENT    { $$ = makeNode(ident); strcpy($$->val.ident, $1); }
    ;
Arguments:
       ListExp  { $$ = makeNode(args); addChild($$, $1); }
    |           { $$ = makeNode(args); }
    ;
ListExp:
       ListExp ',' Exp  { $$ = $1; addSibling($$, $3); }
    |  Exp              { $$ = $1; }
    ;

%%

/* Print error message */
void yyerror(const char *msg)
{
    fprintf(stderr, "line %d, char %d : %s\n", line_nb, char_nb, msg);
}
