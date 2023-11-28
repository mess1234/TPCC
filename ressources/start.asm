section .text
global _start
_start:
mov r11, rsp
sub rsp, 8
and rsp, -16
mov qword [rsp], r11
call func_main
pop rsp
mov rdi, rax
mov rax, 60
syscall
func_putchar:
push rbp
mov rbp, rsp
push rdi
mov rax, 1
mov rdi, 1
mov rsi, rsp
mov rdx, 1
syscall
mov rsp, rbp
pop rbp
ret
func_putint:
push rbp
mov rbp, rsp
mov  r9, 10
xor  rdx, rdx
mov  rax, rdi
idiv r9
add rdx, '0'
push rdx
test rax, rax
jz labelEnd_putint
mov rdi, rax
mov r11, rsp
sub rsp, 8
and rsp, -16
mov qword [rsp], r11
call func_putint
pop rsp
labelEnd_putint:
mov rax, 1
mov rdi, 1
mov rsi, rsp
mov rdx, 1
syscall
mov rsp, rbp
pop rbp
ret
func_getchar:
push rbp
mov rbp, rsp
push 0
mov rax, 0
mov rdi, 0
mov rsi, rsp
mov rdx, 1
syscall
pop rax
mov rsp, rbp
pop rbp
ret
func_getint:
push rbp
mov rbp, rsp
push 0
mov rax, 0
mov rdi, 0
mov rsi, rsp
mov rdx, 1
syscall
mov r10, qword [rsp]
cmp r10, '0'
jl labelErr_getint
cmp r10, '9'
jg labelErr_getint
sub r10, '0'
mov r9, r10
jmp labelLoop_getint
labelErr_getint:
mov rax, 60
mov rdi, 5
syscall
labelLoop_getint:
mov rax, 0
mov rdi, 0
mov rsi, rsp
mov rdx, 1
syscall
mov r10, qword [rsp]
cmp r10, '0'
jl labelRet_getint
cmp r10, '9'
jg labelRet_getint
sub r10, '0'
imul r9, 10
add r9, r10
jmp labelLoop_getint
labelRet_getint:
mov rax, r9
mov rsp, rbp
pop rbp
ret
