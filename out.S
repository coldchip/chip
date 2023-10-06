global	_start

_start:
	call	chip_func_main
	mov	rax, 60
	xor	rdi, rdi
	syscall


chip_func_main:
	push	rbp
	mov	rbp, rsp
	sub	rsp, 32
	mov rax, 1
	mov [rbp-8], rax
	mov rax, 0
	mov [rbp-16], rax
	mov rax, 1
	mov [rbp-24], rax
	mov rax, [rbp-16]
	push rax
	mov rax, [rbp-24]
	push rax
	pop rbx
	pop rax
	add rax, rbx
	mov [rbp-32], rax
L0:
	mov rax, [rbp-8]
	push rax
	mov rax, 40
	push rax
	pop rbx
	pop rax
	cmp rax, rbx
	setl al
	movzx rax, al
	mov	rbx, 1
	cmp	rbx, rax
	jg	L1
	mov rax, [rbp-24]
	mov [rbp-16], rax
	mov rax, [rbp-32]
	mov [rbp-24], rax
	mov rax, [rbp-16]
	push rax
	mov rax, [rbp-24]
	push rax
	pop rbx
	pop rax
	add rax, rbx
	mov [rbp-32], rax
	mov rax, [rbp-24]
	push	rax
	call	chip_func_itoa
	add rsp, 8
	mov rax, 10
	push	rax
	call	chip_func_printf
	add rsp, 8
	mov rax, [rbp-8]
	push rax
	mov rax, 1
	push rax
	pop rbx
	pop rax
	add rax, rbx
	mov [rbp-8], rax
	jmp	L0
L1:
	mov	rsp, rbp
	pop	rbp
	ret


chip_func_itoa:
	push	rbp
	mov	rbp, rsp
	sub	rsp, 64
	mov [rbp-40], rax
	mov rax, 10
	mov [rbp-48], rax
L2:
	mov rax, [rbp-40]
	push rax
	mov rax, 0
	push rax
	pop rbx
	pop rax
	cmp rax, rbx
	setg al
	movzx rax, al
	mov	rbx, 1
	cmp	rbx, rax
	jg	L3
	mov rax, [rbp-40]
	push rax
	mov rax, [rbp-48]
	push rax
	pop rbx
	pop rax
	cdq
	idiv rbx
	mov rax, rdx
	mov [rbp-56], rax
	mov rax, [rbp-56]
	push rax
	mov rax, 9
	push rax
	pop rbx
	pop rax
	cmp rax, rbx
	setg al
	movzx rax, al
	mov	rbx, 1
	cmp	rbx, rax
	jg	L4
	mov rax, [rbp-56]
	push rax
	mov rax, 7
	push rax
	pop rbx
	pop rax
	add rax, rbx
	mov [rbp-56], rax
L4:
	mov rax, 48
	push rax
	mov rax, [rbp-56]
	push rax
	pop rbx
	pop rax
	add rax, rbx
	mov [rbp-64], rax
	mov rax, [rbp-64]
	push	rax
	call	chip_func_printf
	add rsp, 8
	mov rax, [rbp-40]
	push rax
	mov rax, [rbp-48]
	push rax
	pop rbx
	pop rax
	cdq
	idiv rbx
	mov [rbp-40], rax
	jmp	L2
L3:
	mov rax, 0
	mov	rsp, rbp
	pop	rbp
	ret


chip_func_printf:
	push	rbp
	mov	rbp, rsp
	sub	rsp, 72
	mov [rbp-72], rax
	lea rax, [rbp-72]

		mov 	rsi, rax
		mov 	rdx, 1				; number of bytes
		mov     rax, 1                  ; system call for write
		mov     rdi, 1                  ; file handle 1 is stdout
		syscall                         ; invoke operating system to do the write

		mov rax, 0
	mov	rsp, rbp
	pop	rbp
	ret


