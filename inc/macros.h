/* Define useful macros in one single place */

#ifndef __MACROS__
#define __MACROS__

#define NB_PREDEF 4 // number of predefined functions

#define EXIT_NO_ERR 0
#define EXIT_SYN_ERR 1
#define EXIT_SEM_ERR 2
#define EXIT_IO_ERR 5
#define EXIT_OTHER_ERR 6

/* hardcode help and asm skeleton instead of loading text files => tpcc can be
 * moved anywhere and executed from anywhere */

#define HELP_STRING                               \
    "NAME\n"                                      \
    "\ttpcc - TPC Compiler\n"                     \
    "USAGE\n"                                     \
    "\ttpcc [OPTIONS]\n"                          \
    "DESCRIPTION\n"                               \
    "\tCompile standard input from TPC to nasm\n" \
    "OPTIONS\n"                                   \
    "\t-h  --help\tPrint this help and quit\n"    \
    "\t-t  --tree\tPrint syntax tree\n"           \
    "\t-s  --symtabs\tPrint all symbol tables\n"  \
    "NOTES\n"                                     \
    "\tIf lanched with the help option, doesn't compile input and returns 0\n"

#define ASM_START_STRING     \
    "section .text\n"        \
    "global _start\n"        \
    "_start:\n"              \
    "mov r11, rsp\n"         \
    "sub rsp, 8\n"           \
    "and rsp, -16\n"         \
    "mov qword [rsp], r11\n" \
    "call func_main\n"       \
    "pop rsp\n"              \
    "mov rdi, rax\n"         \
    "mov rax, 60\n"          \
    "syscall\n"              \
    "func_putchar:\n"        \
    "push rbp\n"             \
    "mov rbp, rsp\n"         \
    "push rdi\n"             \
    "mov rax, 1\n"           \
    "mov rdi, 1\n"           \
    "mov rsi, rsp\n"         \
    "mov rdx, 1\n"           \
    "syscall\n"              \
    "mov rsp, rbp\n"         \
    "pop rbp\n"              \
    "ret\n"                  \
    "func_putint:\n"         \
    "push rbp\n"             \
    "mov rbp, rsp\n"         \
    "mov  r9, 10\n"          \
    "xor  rdx, rdx\n"        \
    "mov  rax, rdi\n"        \
    "idiv r9\n"              \
    "add rdx, '0'\n"         \
    "push rdx\n"             \
    "test rax, rax\n"        \
    "jz labelEnd_putint\n"   \
    "mov rdi, rax\n"         \
    "mov r11, rsp\n"         \
    "sub rsp, 8\n"           \
    "and rsp, -16\n"         \
    "mov qword [rsp], r11\n" \
    "call func_putint\n"     \
    "pop rsp\n"              \
    "labelEnd_putint:\n"     \
    "mov rax, 1\n"           \
    "mov rdi, 1\n"           \
    "mov rsi, rsp\n"         \
    "mov rdx, 1\n"           \
    "syscall\n"              \
    "mov rsp, rbp\n"         \
    "pop rbp\n"              \
    "ret\n"                  \
    "func_getchar:\n"        \
    "push rbp\n"             \
    "mov rbp, rsp\n"         \
    "push 0\n"               \
    "mov rax, 0\n"           \
    "mov rdi, 0\n"           \
    "mov rsi, rsp\n"         \
    "mov rdx, 1\n"           \
    "syscall\n"              \
    "pop rax\n"              \
    "mov rsp, rbp\n"         \
    "pop rbp\n"              \
    "ret\n"                  \
    "func_getint:\n"         \
    "push rbp\n"             \
    "mov rbp, rsp\n"         \
    "push 0\n"               \
    "mov rax, 0\n"           \
    "mov rdi, 0\n"           \
    "mov rsi, rsp\n"         \
    "mov rdx, 1\n"           \
    "syscall\n"              \
    "mov r10, qword [rsp]\n" \
    "cmp r10, '0'\n"         \
    "jl labelErr_getint\n"   \
    "cmp r10, '9'\n"         \
    "jg labelErr_getint\n"   \
    "sub r10, '0'\n"         \
    "mov r9, r10\n"          \
    "jmp labelLoop_getint\n" \
    "labelErr_getint:\n"     \
    "mov rax, 60\n"          \
    "mov rdi, 5\n"           \
    "syscall\n"              \
    "labelLoop_getint:\n"    \
    "mov rax, 0\n"           \
    "mov rdi, 0\n"           \
    "mov rsi, rsp\n"         \
    "mov rdx, 1\n"           \
    "syscall\n"              \
    "mov r10, qword [rsp]\n" \
    "cmp r10, '0'\n"         \
    "jl labelRet_getint\n"   \
    "cmp r10, '9'\n"         \
    "jg labelRet_getint\n"   \
    "sub r10, '0'\n"         \
    "imul r9, 10\n"          \
    "add r9, r10\n"          \
    "jmp labelLoop_getint\n" \
    "labelRet_getint:\n"     \
    "mov rax, r9\n"          \
    "mov rsp, rbp\n"         \
    "pop rbp\n"              \
    "ret\n"

#endif
