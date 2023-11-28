; print 1 character
; 1 param in rdi, no return value
putchar:
	; save rbp, rsp & realign stack
    push rbp
    mov rbp, rsp
    ; print char
	push rdi
	mov rax, 1		; write
	mov rdi, 1		; to stdout
	mov rsi, rsp	; data stored at [rsp]
	mov rdx, 1		; nb of bytes
	syscall
    ; restore rsp and return
    mov rsp, rbp
    pop rbp
	ret

; print 1 integer
; 1 param in rdi, no return value
putint:
	; save rbp, rsp & realign stack
    push rbp
    mov rbp, rsp
	; do
	; 	q, r = rdi / 10
	; 	push r
	; while (q != 0)
	labelDiv_putint:
	mov  r9, 10		; copy divisor to scratch register
	xor  rdx, rdx   ; clear high bits of dividend
	mov  rax, rdi	; copy rdi into dividend
	idiv r9         ; compute rdi / 10 => quotient in rax, remainder in rdx
	add rdx, '0'	; turn digit into a char
	push rdx		; save to stack
	mov rdi, rax
	test rax, rax
    jnz labelDiv_putint
	; do
	; 	print digit
	; 	pop _
	; while (rsp != rbp) // while some digits remain in stack
	labelPrint_putint:
	; print digit on top of stack
	mov rax, 1		; write
	mov rdi, 1		; to stdout
	mov rsi, rsp	; digit is on top of stack
	mov rdx, 1		; nb of bytes
	syscall
	pop r9	; remove digit
	cmp rsp, rbp
	jne labelPrint_putint
    ; restore rsp and return
    mov rsp, rbp
    pop rbp
	ret

; read 1 character
; no param, one return value in rax
getchar:
	; save rbp, rsp & realign stack
    push rbp
    mov rbp, rsp
    ; read input
	push 0			; allocate 8 bytes for the char
	mov rax, 0		; read
	mov rdi, 0		; from stdin
	mov rsi, rsp	; write result at [rsp]
	mov rdx, 1		; nb of bytes
	syscall
	pop rax
    ; restore rsp and return
    mov rsp, rbp
    pop rbp
	ret

; read 1 integer
; no param, one return value in rax
getint:
	; save rbp, rsp & realign stack
    push rbp
    mov rbp, rsp
	; init
	push 0		; allocate 8 bytes for current digit
    ; read input
	mov rax, 0		; read
	mov rdi, 0		; from stdin
	mov rsi, rsp	; write result at [rsp]
	mov rdx, 1		; nb of bytes
	syscall
	mov r10, qword [rsp]	; digit in r10
	; first char is not a digit => exit(5)
	cmp r10, '0'
	jl labelErr_getint
	cmp r10, '9'
	jg labelErr_getint
	; first char is a digit => add it to result
	sub r10, '0'
	mov r9, r10	; result will be stored in r9
	jmp labelLoop_getint
	labelErr_getint:
	mov rax, 60
	mov rdi, 5
	syscall
	labelLoop_getint:	; continue while reading a digit
    ; read input
	mov rax, 0		; read
	mov rdi, 0		; from stdin
	mov rsi, rsp	; write result at [rsp]
	mov rdx, 1		; nb of bytes
	syscall
	mov r10, qword [rsp]	; digit in r10
	; if not a digit, return
	cmp r10, '0'
	jl labelRet_getint
	cmp r10, '9'
	jg labelRet_getint
	; add digit to result
	sub r10, '0'
	imul r9, 10
	add r9, r10
	jmp labelLoop_getint
	labelRet_getint:
	mov rax, r9
    ; restore rsp, rbp
    mov rsp, rbp
    pop rbp
	ret

; == PUTINT REC ==

; ; print 1 integer
; ; 1 param in rdi, no return value
; putint:
; 	; save rbp, rsp & realign stack
;     push rbp
;     mov rbp, rsp
; 	; compute rdi / 10, quotient in rax, remainder in rdx
; 	mov  r9, 10		; copy divisor to scratch register
; 	xor  rdx, rdx   ; clear high bits of dividend
; 	mov  rax, rdi	; copy dividend into rax
; 	idiv r9         ; divide
; 	; save remainder char in stack
; 	add rdx, '0'
; 	push rdx
; 	; if quotient != 0 print it
; 	test rax, rax
;     jz labelEnd_putint
; 	; call putint
; 	mov rdi, rax
; 	mov r11, rsp
; 	sub rsp, 8
; 	and rsp, -16
; 	mov qword [rsp], r11
; 	call putint
; 	pop rsp
; 	labelEnd_putint:
; 	; print remainder
; 	mov rax, 1		; write
; 	mov rdi, 1		; to stdout
; 	mov rsi, rsp	; digit is on top of stack
; 	mov rdx, 1		; nb of bytes
; 	syscall
;     ; restore rsp and return
;     mov rsp, rbp
;     pop rbp
; 	ret