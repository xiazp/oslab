/*
 *  linux/kernel/system_call.s
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 *  system_call.s  contains the system-call low-level handling routines.
 * This also contains the timer-interrupt handler, as some of the code is
 * the same. The hd- and flopppy-interrupts are also here.
 *
 * NOTE: This code handles signal-recognition, which happens every time
 * after a timer-interrupt and after each system call. Ordinary interrupts
 * don't handle signal-recognition, as that would clutter them up totally
 * unnecessarily.
 *
 * Stack layout in 'ret_from_system_call':
 *
 *	 0(%esp) - %eax
 *	 4(%esp) - %ebx
 *	 8(%esp) - %ecx
 *	 C(%esp) - %edx
 *	10(%esp) - %fs
 *	14(%esp) - %es
 *	18(%esp) - %ds
 *	1C(%esp) - %eip
 *	20(%esp) - %cs
 *	24(%esp) - %eflags
 *	28(%esp) - %oldesp
 *	2C(%esp) - %oldss
 */

SIG_CHLD	= 17

EAX		= 0x00
EBX		= 0x04
ECX		= 0x08
EDX		= 0x0C
FS		= 0x10
ES		= 0x14
DS		= 0x18
EIP		= 0x1C
CS		= 0x20
EFLAGS		= 0x24
OLDESP		= 0x28
OLDSS		= 0x2C


ESP0 = 4
KERNEL_STACK = 12

//struct tss_struct *tss = &(init_task.task.tss);

/*
 * Ok, I get parallel printer interrupts while using the floppy for some
 * strange reason. Urgel. Now I just ignore them.
 */
.globl switch_to_by_stack


.align 2
switch_to_by_stack:
	pushl 	%ebp
	movl 	%esp,%ebp
	pushl	%ecx
	pushl	%ebx
	pushl	%eax
	movl	8(%ebp),%ebx
	cmpl	%ebx,current
	je		1f
	#ÇÐ»»PCB
	movl	%ebx,%eax
	xchgl	%eax,current
	#TSSÖÐÄÚºËÕ»Ö¸ÕëµÄÖØÐ´
	movl tss,%ecx
	addl $4096£¬%ebx
	movl %ebx, ESP0(%ecx)
	#ÇÐ»»ÄÚºËÕ»
	movl %esp, KERNEL_STACK(%eax)
	movl 8(%ebp), %ebx
	movl KERNEL_STACK(%ebx),%esp
	#ÇÐ»»LDT
	movl	$0x17,%ecx
	mov		%cx,%fs
	movl	$0x17,%ecx
	mov		%cx,%fs
	cmpl	%eax,last_task_used_path
	jne		1f
	clts
1:	popl	%eax
	popl	%ebx
	popl	%ecx
	ret






