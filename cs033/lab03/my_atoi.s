	.file	"my_atoi.c"
	.text
.globl my_atoi
	.type	my_atoi, @function
my_atoi:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	pushl	%edi
	pushl	%esi
##############################
# DON'T EDIT ABOVE THIS LINE #
##############################
##########################################################################
# atoi                                                                   #
# 8(%ebp) currently holds a pointer to the array of characters (string). #
# Make sure your return value is in the %eax register at the end!        #
##########################################################################

# Write your code here...
	movl	$0, %eax # initializes return var. to 0
	movl	8(%ebp), %edx # edx takes pointer to char *
L1:
	movzbl	(%edx), %ecx # ecx takes dedcimal value from pointer
	subl	$48, %ecx # subtract 48 from decimal value
	cmpl	$9, %ecx # compare with 9 -- if greater, not a number -> quit
	ja		L2 #jump to end
	imull	$10, %eax #eax = 10*eax
	addl	%ecx, %eax #eax = eax + ecx
	incl 	%edx #go to next char in array
	jmp		L1	#loop
L2:
	

##############################
# DON'T EDIT BELOW THIS LINE #
##############################
	movl	-12(%ebp), %esi
	movl	-8(%ebp), %edi
	movl	-4(%ebp), %ebx
	leave
	ret
	.size	my_atoi, .-my_atoi
	.ident	"GCC: (Debian 4.4.5-8) 4.4.5"
	.section	.note.GNU-stack,"",@progbits
