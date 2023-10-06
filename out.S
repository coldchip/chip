global	_start

_start:
	call	chip_func_main
	mov	rax, 60
	xor	rdi, rdi
	syscall


chip_func_main:
	push	rbp
	mov	rbp, rsp
	sub	rsp, 296
	mov [rbp-256], rax
	mov rax, 72
	push rax
	xor rdi, rdi
	mov rax, 0
	mov rdi, rax
	pop rax
	mov [rbp-256+(rdi*8)], rax
	mov rax, 101
	push rax
	xor rdi, rdi
	mov rax, 1
	mov rdi, rax
	pop rax
	mov [rbp-256+(rdi*8)], rax
	mov rax, 108
	push rax
	xor rdi, rdi
	mov rax, 2
	mov rdi, rax
	pop rax
	mov [rbp-256+(rdi*8)], rax
	mov rax, 108
	push rax
	xor rdi, rdi
	mov rax, 3
	mov rdi, rax
	pop rax
	mov [rbp-256+(rdi*8)], rax
	mov rax, 111
	push rax
	xor rdi, rdi
	mov rax, 4
	mov rdi, rax
	pop rax
	mov [rbp-256+(rdi*8)], rax
	mov rax, 32
	push rax
	xor rdi, rdi
	mov rax, 5
	mov rdi, rax
	pop rax
	mov [rbp-256+(rdi*8)], rax
	mov rax, 87
	push rax
	xor rdi, rdi
	mov rax, 6
	mov rdi, rax
	pop rax
	mov [rbp-256+(rdi*8)], rax
	mov rax, 111
	push rax
	xor rdi, rdi
	mov rax, 7
	mov rdi, rax
	pop rax
	mov [rbp-256+(rdi*8)], rax
	mov rax, 114
	push rax
	xor rdi, rdi
	mov rax, 8
	mov rdi, rax
	pop rax
	mov [rbp-256+(rdi*8)], rax
	mov rax, 108
	push rax
	xor rdi, rdi
	mov rax, 9
	mov rdi, rax
	pop rax
	mov [rbp-256+(rdi*8)], rax
	mov rax, 100
	push rax
	xor rdi, rdi
	mov rax, 10
	mov rdi, rax
	pop rax
	mov [rbp-256+(rdi*8)], rax
	mov rax, 0
	mov [rbp-264], rax
L0:
	mov rax, [rbp-264]
	push rax
	mov rax, 11
	push rax
	pop rbx
	pop rax
	cmp rax, rbx
	setl al
	movzx rax, al
	mov	rbx, 1
	cmp	rbx, rax
	jg	L1
	xor rdi, rdi
	mov rax, [rbp-264]
	mov rdi, rax
	mov rax, [rbp-256+(rdi*8)]
	push	rax
	call	chip_func_printf
	add rsp, 8
	mov rax, [rbp-264]
	push rax
	mov rax, 1
	push rax
	pop rbx
	pop rax
	add rax, rbx
	mov [rbp-264], rax
	jmp	L0
L1:
	mov rax, 10
	push	rax
	call	chip_func_printf
	add rsp, 8
	mov rax, 1
	mov [rbp-272], rax
	mov rax, 0
	mov [rbp-280], rax
	mov rax, 1
	mov [rbp-288], rax
	mov rax, [rbp-280]
	push rax
	mov rax, [rbp-288]
	push rax
	pop rbx
	pop rax
	add rax, rbx
	mov [rbp-296], rax
L2:
	mov rax, [rbp-272]
	push rax
	mov rax, 46
	push rax
	pop rbx
	pop rax
	cmp rax, rbx
	setl al
	movzx rax, al
	mov	rbx, 1
	cmp	rbx, rax
	jg	L3
	mov rax, [rbp-288]
	mov [rbp-280], rax
	mov rax, [rbp-296]
	mov [rbp-288], rax
	mov rax, [rbp-280]
	push rax
	mov rax, [rbp-288]
	push rax
	pop rbx
	pop rax
	add rax, rbx
	mov [rbp-296], rax
	mov rax, [rbp-288]
	push	rax
	call	chip_func_itoa
	add rsp, 8
	mov rax, 10
	push	rax
	call	chip_func_printf
	add rsp, 8
	mov rax, [rbp-272]
	push rax
	mov rax, 1
	push rax
	pop rbx
	pop rax
	add rax, rbx
	mov [rbp-272], rax
	jmp	L2
L3:
	mov	rsp, rbp
	pop	rbp
	ret


chip_func_itoa:
	push	rbp
	mov	rbp, rsp
	sub	rsp, 592
	mov [rbp-304], rax
	mov [rbp-560], rax
	mov rax, 16
	mov [rbp-568], rax
	mov rax, 0
	mov [rbp-576], rax
L4:
	mov rax, [rbp-304]
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
	jg	L5
	mov rax, [rbp-304]
	push rax
	mov rax, [rbp-568]
	push rax
	pop rbx
	pop rax
	cdq
	idiv rbx
	mov rax, rdx
	mov [rbp-584], rax
	mov rax, [rbp-584]
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
	jg	L6
	mov rax, [rbp-584]
	push rax
	mov rax, 7
	push rax
	pop rbx
	pop rax
	add rax, rbx
	mov [rbp-584], rax
L6:
	mov rax, 48
	push rax
	mov rax, [rbp-584]
	push rax
	pop rbx
	pop rax
	add rax, rbx
	mov [rbp-592], rax
	mov rax, [rbp-592]
	push rax
	xor rdi, rdi
	mov rax, [rbp-576]
	mov rdi, rax
	pop rax
	mov [rbp-560+(rdi*8)], rax
	mov rax, [rbp-304]
	push rax
	mov rax, [rbp-568]
	push rax
	pop rbx
	pop rax
	cdq
	idiv rbx
	mov [rbp-304], rax
	mov rax, [rbp-576]
	push rax
	mov rax, 1
	push rax
	pop rbx
	pop rax
	add rax, rbx
	mov [rbp-576], rax
	jmp	L4
L5:
L7:
	mov rax, [rbp-576]
	push rax
	mov rax, 0
	push rax
	mov rax, 1
	push rax
	pop rbx
	pop rax
	sub rax, rbx
	push rax
	pop rbx
	pop rax
	cmp rax, rbx
	setg al
	movzx rax, al
	mov	rbx, 1
	cmp	rbx, rax
	jg	L8
	xor rdi, rdi
	mov rax, [rbp-576]
	mov rdi, rax
	mov rax, [rbp-560+(rdi*8)]
	push	rax
	call	chip_func_printf
	add rsp, 8
	mov rax, [rbp-576]
	push rax
	mov rax, 1
	push rax
	pop rbx
	pop rax
	sub rax, rbx
	mov [rbp-576], rax
	jmp	L7
L8:
	mov rax, 0
	mov	rsp, rbp
	pop	rbp
	ret


chip_func_printf:
	push	rbp
	mov	rbp, rsp
	sub	rsp, 600
	mov [rbp-600], rax
	lea rax, [rbp-600]

		mov 	rsi, rax
		mov 	rdx, 1				; number of bytes
		mov     rax, 1                  ; system call for write
		mov     rdi, 1                  ; file handle 1 is stdout
		syscall                         ; invoke operating system to do the write

		mov rax, 0
	mov	rsp, rbp
	pop	rbp
	ret


