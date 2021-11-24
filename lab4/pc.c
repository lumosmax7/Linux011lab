#define __LIBRARY__ /* 好像是定义系统调用库*/
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
/*增加系统调用*/
_syscall2(sem_t*,sem_open,const char *,name,unsigned int,value);
_syscall1(int,sem_wait,sem_t*,sem);
_syscall1(int,sem_post,sem_t*,sem);
_syscall1(int,sem_unlink,const char *,name);

#define NUMBER 500 /* 设置写入总数的数字是500 */
#define CHILD 5 /* 自进程，即消费者进程数目 */
#define BUFFSIZE    10 /* 缓冲区大小 */
/*
 * empty 是空闲资源
 * full 是产品资源
 * mutex 是互斥量
 */
sem_t *empty,*full,*mutex;

int fno; /*文件描述符*/
FILE *fp;

int main(){
    pid_t p;
    int i,j,k;
    int data;
    int buf_out =0; /* 读出缓冲区的指针*/
    int buf_in = 0;/* 写入缓冲区的指针*/

    fno = open("buffer.dat",O_CREAT|O_RDWR|O_TRUNC,0666);  /*后面都是一些定义，权限等*/

    /* 创建三个信号量 */
    if((mutex = sem_open("carmutex",1)) == SEM_FAILED){
        perror("sem_open() error!\n");
        return -1;
    }
    if((empty = sem_open("carempty",BUFFSIZE)) == SEM_FAILED)
    {
        perror("sem_open() error!\n");
        return -1;
    }
    if((full = sem_open("carfull",0)) == SEM_FAILED)
    {
        perror("sem_open() error!\n");
        return -1;
    }
    /* 把读取的位置存在buffsize的最后一个单位, 方便子进程间的通信*/
    lseek(fno,10*sizeof(int),SEEK_SET);
    write(fno,(char *)&buf_out,sizeof(int));
    /* 返回0说明fork成功，为子进程*/
    /* 生产者进程*/
    if((p = fork())==0){
        for(i=0;i<NUMBER;i++){
            sem_wait(empty);
            sem_wait(mutex);
            /* 写入到缓冲区 */
            lseek(fno,buf_in*sizeof(int),SEEK_SET);
            write(fno,(char *)&i,sizeof(int));
            buf_in = (buf_in+1)%BUFFSIZE; /* 每10个为1组 */

            sem_post(mutex);
            sem_post(full);
        }
    }else if(p<0){
        perror("Fail to fork!\n");
        return -1;
    }
    for(j=0;j<CHILD;j++){
        if((p = fork())==0){
            /* 五个进程来读，每个进程一次都一个数字，需要多少轮（每轮读5个）*/
            for(k=0;k<NUMBER/CHILD;k++){
                sem_wait(full);
                sem_wait(mutex);
                /* 获取要读取的位置*/
                lseek(fno,10*sizeof(int),SEEK_SET);
                read(fno,(char *)&buf_out,sizeof(int));
                /* 读入数字*/
                lseek(fno,buf_out*sizeof(int),SEEK_SET);
                read(fno,(char *)&data,sizeof(int));
                buf_out=(buf_out+1)%BUFFSIZE;
                /* 存储下一个要读取的位置*/
                lseek(fno,10*sizeof(int),SEEK_SET);
                write(fno,(char *)&buf_out,sizeof(int));
                
                /* 写文件操作*/
                fp =fopen("pc.log","a+");
                fprintf(fp,"%d:  %d\n",getpid(),data);
                fclose(fp);
    		
                sem_post(mutex);
                sem_post(empty);
                /* 
                printf("%d:  %d\n",getpid(),data);
                fflush(stdout);
                sleep(1); */
            }
        } else if(p<0){
            perror("Fail to fork!\n");
            return -1;
        }
    }
    wait(NULL);
    sem_unlink("carfull");
    sem_unlink("carempty");
    sem_unlink("carmutex");
    /*释放资源*/
    close(fno);
    
    return 0;

}