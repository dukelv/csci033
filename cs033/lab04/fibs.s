.file "fibs.s"					# Linker stuff (you don't need to worry about this)
.text

##################
# fibs_recursive #
##################

.globl fibs_recursive			# Linker stuff
.type	fibs_recursive, @function

fibs_recursive:					# unsigned int fibs_recursive(unsigned int n) {
	pushl	%ebp #save old ebp
	movl	%esp, %ebp #set ebp as frame ptr
	pushl	%ebx #save callee register ebx
	pushl	%edi
	pushl	%esi
	subl	$4, %esp #allocate 4 bytes on stack
	movl	8(%ebp), %ebx #get n
	movl	%ebx, %esi
	
	movl	$0, %eax #set eax=0
	cmpl	%ebx, %eax #see if n==0
	je		L1 #n==0, return
	
	movl	$1, %eax #set eax=1
	cmpl	%ebx, %eax #see if n==1
	je		L1 #n==1, return
	
	movl 	%ebx, %eax
	subl	$1, %eax
	movl	%eax, (%esp)
	call	fibs_recursive
	movl	%eax, %edi

	movl	%esi, %eax
	subl	$2, %eax
	movl	%eax, (%esp)
	call	fibs_recursive

	movl	%eax, %esi
	addl	%edi, %esi
	movl	%esi, %eax
L1:
	addl	$4, %esp
	popl	%esi
	popl	%edi
	popl	%ebx
	popl	%ebp
	# YOUR CODE HERE			#
	ret							#
								# }

.size	fibs_recursive, .-fibs_recursive  # Linker stuff

.section	.note.GNU-stack,"",@progbits
