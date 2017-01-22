/*
 * sem.c
 *
 *  Created on: 2017-1-19
 *      Author: justin
 */

#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/system.h>
#include <asm/segment.h>
#include <fcntl.h>
#include <semaphore.h>

static int fprintk_fd=3;

#define api_debug_open() \
	do{\
		fprintk_fd=open("/var/sem.log",O_CREAT|O_TRUNC|O_RDWR, 0666);\
	}while(0)

#define api_enter() \
	do{\
		fprintk(fprintk_fd,"[+ %s +]() enter\n",__func__);\
	}while(0)

#define api_exit() \
	do{\
		fprintk(fprintk_fd,"[- %s -]() exit\n",__func__);\
	}while(0)

#define api_printf(fmt,arg...) fprintk(fprintk_fd,fmt,##arg)


static sem_t semtable[20];

static int get_sd(const char *name)
{
	int i=0,retval=-1;

	for(i=0; i<sizeof(semtable)/sizeof(semtable[0]); i++){
		if(strcmp(semtable[i].name, name) == 0){
			retval= i;
			break;
		}
	}

	return retval;
}

static int get_empty_sem(void)
{
	int i,retval=-1;

	for(i=0; i<sizeof(semtable)/sizeof(semtable[0]); i++){
		if(strlen(semtable[i].name) == 0){
			retval= i;
			break;
		}
	}

	return retval;
}

sem_t *sys_sem_open(const char *name, int value)
{
	int retval,cnt;
	sem_t *psem='\0';
	char iname[20]={0};


	cli();
	api_enter();

	cnt = 0;
	memset(iname,0,sizeof(iname));
	while( (iname[cnt]=get_fs_byte(name+cnt)) != '\0' && cnt < 20)
    		cnt++;

	api_printf("O1 n=%s v=%d\n",iname, value);


	retval = get_sd(iname);
	if(retval >= 0)
		psem = &semtable[retval];
	else{
		retval = get_empty_sem();
		if(retval >= 0){
			psem= &semtable[retval];
			psem->value =value;
			strcpy(psem->name, iname);
			printf("");
		}
	}

	api_printf("O2 n=%s v=%d\n",psem->name,psem->value);
	api_exit();
	sti();

	return psem;
}

int sys_sem_wait(sem_t *sem)
{
	int retval = -1;

	cli();
	api_enter();

	api_printf("w1 n=%s v=%d\n",sem->name,sem->value);
	if( (--sem->value) < 0){
		api_printf("go sleep\n",sem->name,sem->value);
		sleep_on(&sem->queue);
	}
	api_printf("w2 n=%s v=%d\n",sem->name,sem->value);


	api_exit();
	sti();
	return retval;
}


int sys_sem_post(sem_t *sem)
{
	int retval=-1;

	cli();
	api_enter();

	api_printf("p1 n=%s v=%d\n",sem->name,sem->value);
	sem->value++;
	wake_up(&sem->queue);
	api_printf("p2 n=%s v=%d\n",sem->name,sem->value);


	api_exit();
	sti();
	return retval;
}


int sys_sem_unlink(const char *name)
{
	int retval=-1,cnt;
	sem_t *psem='\0';
	char iname[20]={0};

	cli();
	api_enter();

	cnt = 0;
	memset(iname,0,sizeof(iname));
	while( (iname[cnt]=get_fs_byte(name+cnt)) != '\0' && cnt < 20)
    		cnt++;

	retval =get_sd(iname);
	if(retval >= 0)
		psem = &semtable[retval];

	if(psem!='\0'){
		psem->value=0;
		memset(psem->name, 0, sizeof(psem->name));
		psem->queue='\0';
		retval=0;
	}

	api_exit();
	sti();
	return retval;
}


int sys_sem_close(sem_t *sem)
{
	int retval=-1;

	cli();
	api_enter();

	if(sem != '\0')
		sem='\0';

	api_exit();
	sti();
	return 0;
}





