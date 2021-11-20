#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <sys/types.h>

#define HZ  100 /*时钟中断时间,10ms一次, 在include/linux/sched.h中有定义*/

/*
 * 此函数按照参数占用CPU和I/O时间
 * last: 函数实际占用CPU和I/O的总时间，不含在就绪队列中的时间，>=0是必须的
 * cpu_time: 一次连续占用CPU的时间，>=0是必须的
 * io_time: 一次I/O消耗的时间，>=0是必须的
 * 如果last > cpu_time + io_time，则往复多次占用CPU和I/O，直到总运行时间超过last为止
 * 所有时间的单位为秒
 */
void cpuio_bound(int last, int cpu_time, int io_time){
    struct tms start_time, current_time; /* 声明两个结构体; */
    clock_t utime,stime; /*utime 用户使用时间, stime 系统使用时间, 单位都是滴答数.*/
    int sleep_time=0; /*模拟io过程*/
    while(last>0){
        times(&start_time); /*装载起始时间*/
        do{
            times(&current_time);/*装载目前的时间*/
            utime = current_time.tms_utime-start_time.tms_utime; /*计算用户使用时间*/
            stime = current_time.tms_stime-start_time.tms_stime; /*计算系统使用时间*/
        }while ( ( (utime + stime) / HZ )  < cpu_time ); /*通过计算tick数字来计算cpu占用时间*/
        last -=cpu_time;
        if(last<=0){
            break;
        }
        while (sleep_time<io_time)
        {
            sleep(1); /*利用休眠来模拟io操作*/
            sleep_time++;
        }
        last-=sleep_time;
    }
}


int main(){
    pid_t n_proc[10];/*设置10个子进程*/
    int i;
    for(i=0;i<10;i++){
        n_proc[i] = fork(); /*fork当前进程,如果fork成功则返回0;*/
        if(n_proc[i]==0){
            cpuio_bound(20,20*i,20-2*i);
            return 0; /* 子进程返回 */
        }
        else if(n_proc[i] <0){
            printf("Failed to fork child process %d!\n",i+1);
            return -1; /* fork失败,返回-1*/
        }
    }
    for(i=0;i<10;i++){
        printf("Child PID: %d\n",n_proc[i]); /*子进程打印信息*/
    }
    wait(&i);
    return 0;
}

