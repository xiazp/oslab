#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>

#define HZ	100

void cpuio_bound(int last, int cpu_time, int io_time);


int main(int argc, char * argv[])
{
	int i,exit_code;
	pid_t pid[4],pid_chid;
	char param[][3]={
		{10, 1, 0},
		{10, 0, 1},
		{10, 1, 1},
		{10, 1, 9},
	};

	memset(pid,0,sizeof(pid));
	for(i=0; i<4; i++){
		printf("pid=%d i=%d\r\n", getpid(), i);
		if( (pid[i] = fork()) == 0){
			printf("child %d run\r\n", getpid());
			cpuio_bound(param[i][0],param[i][1],param[i][2]);
			exit(0);
		}
	}

	for(i=0; i<4; i++){
		pid_chid=wait(&exit_code);
		if(pid_chid > 0)
			printf("child %d exit with code %d\r\n", pid_chid, exit_code);
	}

	return 0;
}

/*
 * 此函数按照参数占用CPU和I/O时间
 * last: 函数实际占用CPU和I/O的总时间，不含在就绪队列中的时间，>=0是必须的
 * cpu_time: 一次连续占用CPU的时间，>=0是必须的
 * io_time: 一次I/O消耗的时间，>=0是必须的
 * 如果last > cpu_time + io_time，则往复多次占用CPU和I/O
 * 所有时间的单位为秒
 */
void cpuio_bound(int last, int cpu_time, int io_time)
{
	struct tms start_time, current_time;
	clock_t utime, stime;
	int sleep_time;

	while (last > 0)
	{
		/* CPU Burst */
		times(&start_time);
		/* 其实只有t.tms_utime才是真正的CPU时间。但我们是在模拟一个
		 * 只在用户状态运行的CPU大户，就像“for(;;);”。所以把t.tms_stime
		 * 加上很合理。*/
		do
		{
			times(&current_time);
			utime = current_time.tms_utime - start_time.tms_utime;
			stime = current_time.tms_stime - start_time.tms_stime;
		} while ( ( (utime + stime) / HZ )  < cpu_time );
		last -= cpu_time;

		if (last <= 0 )
			break;

		/* IO Burst */
		/* 用sleep(1)模拟1秒钟的I/O操作 */
		sleep_time=0;
		while (sleep_time < io_time)
		{
			sleep(1);
			sleep_time++;
		}
		last -= sleep_time;
	}
}
