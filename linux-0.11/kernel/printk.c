/*
 *  linux/kernel/printk.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * When in kernel-mode, we cannot use printf, as fs is liable to
 * point to 'interesting' things. Make a printf with fs-saving, and
 * all is well.
 */
#include <stdarg.h>
#include <stddef.h>

#include <linux/kernel.h>

static char buf[1024];

extern int vsprintf(char * buf, const char * fmt, va_list args);

int printk(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i=vsprintf(buf,fmt,args);
	va_end(args);
	__asm__("push %%fs\n\t"
		"push %%ds\n\t"
		"pop %%fs\n\t"
		"pushl %0\n\t"
		"pushl $buf\n\t"
		"pushl $0\n\t"
		"call tty_write\n\t"
		"addl $8,%%esp\n\t"
		"popl %0\n\t"
		"pop %%fs"
		::"r" (i):"ax","cx","dx");
	return i;
}


#include "linux/sched.h"
#include "sys/stat.h"
static char logbuf[1024];
int __fprintk(int fd, const char *fmt, ...)
{
    va_list args;
    int count;
    struct file * file;
    struct m_inode * inode;

    va_start(args, fmt);
    count=vsprintf(logbuf, fmt, args);
    va_end(args);

    if (fd < 3)    /* 濡傛灉杈撳嚭鍒皊tdout鎴杝tderr锛岀洿鎺ヨ皟鐢╯ys_write鍗冲彲 */
    {
        __asm__("push %%fs\n\t"
            "push %%ds\n\t"
            "pop %%fs\n\t"
            "pushl %0\n\t"
            "pushl $logbuf\n\t" /* 娉ㄦ剰瀵逛簬Windows鐜鏉ヨ锛屾槸_logbuf,涓嬪悓 */
            "pushl %1\n\t"
            "call sys_write\n\t" /* 娉ㄦ剰瀵逛簬Windows鐜鏉ヨ锛屾槸_sys_write,涓嬪悓 */
            "addl $8,%%esp\n\t"
            "popl %0\n\t"
            "pop %%fs"
            ::"r" (count),"r" (fd):"ax","cx","dx");
    }
    else    /* 鍋囧畾>=3鐨勬弿杩扮閮戒笌鏂囦欢鍏宠仈銆備簨瀹炰笂锛岃繕瀛樺湪寰堝鍏跺畠鎯呭喌锛岃繖閲屽苟娌℃湁鑰冭檻銆�*/
    {
        if (!(file=task[0]->filp[fd]))    /* 浠庤繘绋�0鐨勬枃浠舵弿杩扮琛ㄤ腑寰楀埌鏂囦欢鍙ユ焺 */
            return 0;
        inode=file->f_inode;

        __asm__("push %%fs\n\t"
            "push %%ds\n\t"
            "pop %%fs\n\t"
            "pushl %0\n\t"
            "pushl $logbuf\n\t"
            "pushl %1\n\t"
            "pushl %2\n\t"
            "call file_write\n\t"
            "addl $12,%%esp\n\t"
            "popl %0\n\t"
            "pop %%fs"
            ::"r" (count),"r" (file),"r" (inode):"ax","cx","dx");
    }
    return count;
}
