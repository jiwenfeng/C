.section .text

.type display, @function
fmt:
	.string "%d "

newline:
	.string "\n"

display:
	pushl %ebp
	movl %esp, %ebp

	movl $1, %ebx

	LDISPLAY1:
	cmpl %ebx, 12(%ebp)
	jl RETURN

	movl 8(%ebp), %esi
	pushl (%esi, %ebx, 4)
	pushl $fmt
	call printf
	incl %ebx
	jmp LDISPLAY1

	RETURN:
	pushl $newline
	call printf

	movl %ebp, %esp
	popl %ebp
	ret

.type bubble_sort, @function
bubble_sort:
	pushl %ebp
	movl %esp, %ebp

	movl $1, %eax
	movl 12(%ebp), %edi

	L1:
	cmpl %eax, 12(%ebp)
	jae L2
	movl %ebp, %esp
	popl %ebp
	ret

	L2:
	movl %eax, %ebx
	addl $1, %ebx

	L3:
	cmpl %ebx, 12(%ebp)
	jae COMPARE
	incl %eax
	jmp L1

	COMPARE:
	movl 8(%ebp), %esi
	movl (%esi, %eax, 4), %ecx
	movl (%esi, %ebx, 4), %edx
	cmpl %ecx, %edx

	jl L4
	incl %ebx
	jmp L3

	L4:
	movl %edx, (%esi, %eax, 4)
	movl %ecx, (%esi, %ebx, 4)
	incl %ebx
	jmp L3


.global _start

_start:
	pushl %ebp
	movl %esp, %ebp
	subl $40, %esp
	movl $8, -40(%ebp)
	movl $3, -36(%ebp)
	movl $12, -32(%ebp)
	movl $32, -28(%ebp)
	movl $43, -24(%ebp)
	movl $9, -20(%ebp)
	movl $7, -16(%ebp)
	movl $20, -12(%ebp)
	movl $19, -8(%ebp)
	movl $70, -4(%ebp)
	pushl -40(%ebp)


	leal -44(%ebp), %eax
	pushl $10
	pushl %eax
	call display
	addl $8, %esp

	leal -44(%ebp), %eax
	pushl $10
	pushl %eax
	call bubble_sort
	addl $8, %esp

	leal -44(%ebp), %eax
	pushl $10
	pushl %eax
	call display

	movl %ebp, %esp
	popl %ebp

	movl $1, %eax
	movl $0, %ebx
	int $0x80
