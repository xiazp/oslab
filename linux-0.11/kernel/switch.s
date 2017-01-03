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



/*
 * Ok, I get parallel printer interrupts while using the floppy for some
 * strange reason. Urgel. Now I just ignore them.
 */
.globl switch_to_by_stack, first_return_from_kernel


.align 2
switch_to_by_stack:
	#C调用汇编，需要处理栈帧
	pushl 	%ebp		# ebp基址寄存器 ：保存进入函数时sp栈顶位置;这里是保存上一个函数的ebp
	movl 	%esp,%ebp   # 当前esp栈指针保存在ebp，这是当前函数的栈帧
	pushl	%ecx
	pushl	%ebx
	pushl	%eax
	#取下一个进程的PCB
	movl	8(%ebp),%ebx  # ebp+8是pnext的地址
	cmpl	%ebx,current
	je		1f
	#切换PCB
	movl	%ebx,%eax
	xchgl	%eax,current
	#TSS中内核栈指针的重写
	movl tss,%ecx   # tss->ecx
	addl $4096,%ebx # 栈底=ebx+4096
	movl %ebx, ESP0(%ecx) # 设置当前任务的内核栈esp0
	#切换内核栈
	movl %esp, KERNEL_STACK(%eax)
	movl 8(%ebp), %ebx # pnex ->ebx
	movl KERNEL_STACK(%ebx),%esp
	#切换LDT
	#movl	$0x17,%ecx
	#mov		%cx,%fs
  	movl 	12(%ebp),%ecx    #取出第二个参数，_LDT(next)
    lldt	%cx        #切换LDT
	movl	$0x17,%ecx
	mov		%cx,%fs
	cmpl	%eax,last_task_used_math
	jne		1f
	clts
1:	popl	%eax
	popl	%ebx
	popl	%ecx
	popl 	%ebp
	ret

first_return_from_kernel:
	 popl %edx
	 popl %edi
	 popl %esi
	 pop %gs
	 pop %fs
	 pop %es
	 pop %ds
	 iret

