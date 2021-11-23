#define __LIBRARY__  
#include <unistd.h>  
#include <linux/sched.h>  
#include <linux/kernel.h>  
#include <asm/segment.h>  
#include <asm/system.h>   

#define SEM_COUNT   32 /* 信号量的总个数*/
sem_t semaphores[SEM_COUNT]; /* 总的信号量的列表 */

void init_queue(sem_queue *q){
    q->front = q->reat = 0; /* 初始化信号量链表*/
}

int is_empty(sem_queue *q){
    return q->front == q->rear? 1:0; /*空返回1 ,非空返回0 */
}

int is_full(sem_queue*q){
    return (q->rear +1)%QUE_LEN ==q->front?1:0; /*满返回1,非满返回0*/
}

struct task_struct * get_task(sem_queue *q){
    if(is_empty(q)){
        printk("Queue is empty.\n");
        return NULL;
    }
    struct task_struct *tmp = q->wait_tasks[q->front]; /*获取第一个的指针*/
    q->front=(q->front+1)%QUE_LEN; /*更改队列首序号*/ 
    return tmp;
}

/* 插入成功返回1 ,插入失败返回-1*/
int insert_task(struct task_struct *p, sem_queue *q){
    if(is_full(q)){
        printk("Queue is full.\n");
        return -1; 
    }
    q->wait_tasks[q->rear] = p;
    q->rear = (q->rear+1)%QUE_LEN ;
    return 1; 
}
/* 遍历所有的信号量,查找到是否有相同的,如果有相同的,且occupied为1(表示已经打开了), 把序列号输出 */
int sem_location(const char *name){
    for(int i =0;i<SEM_COUNT;i++){
        if(strcmp(name,semaphores[i].name)==0 && semaphores.occupied==1){
            return i;
        }
    }
    return -1;
}

/* 创建的时候必须使value值大于等于0*/
sem_t * sys_sem_open(const char*name, unsigned int value){
    char tmp[16]; /* 存储信号量的名称 */
    char c;
    int i;
    for(i=0;i<16;i++){
        c = get_fs_byte(name+i);  /* 该函数输入的是字符串的地址*/
        tmp[i]=c;
        if(c=='\0'){
            break;
        }
    }
    if(i>=16){ 
        printk("Semaphore name is too long\n");
        return NULL;
    }
    if ((i=sem_location(name))!=-1) /* 表示已打开 */
    {
       return &semaphores[i];
    }
    for(i=0;i<SEM_COUNT;i++){
        if(!semaphores[i].occupied){ /* 如果信号量没有占用,即没有打开 */
            strcpy(semaphores[i].name,tmp);
            semaphores[i].occupied=1;
            semaphores[i].value =value;
            init_queue(&(semaphores[i].wait_queue));
            return &semaphores[i];
        }
    }
    printk("Enough of semaphores! \n");
    return NULL;

}
/* 使用资源 */
int sys_sem_wait(sem_t *sem){
    cli();/* 关闭中断,因为有调度就会有中断,这里避免时间片中断和用户态程序的中断 */
    sem->value--;
    if(sem->value<0){
        current->state = TASK_UNINTERRUPTIBLE;   /* 这里current从sched中的引用,资源被用光所以不可中断休眠 */
        insert_task(current,&(sem->wait_queue)); /*加入队列后,执行调度程序*/
        schedule();
    }
    sti();/*调度完成后开启中断*/
    return 0;
}
/* 释放资源 */
int sys_sem_post(sem_t *sem){
    cli();
    struct task_struct *p;
    sem->value++; /*这里已经给信号量增加了一个资源了, 所以下面的value是小于等于0*/
    if(sem->value<=0){
        p = get_task(&(sem->wait_queue));
        if(p!=NULL){
        (*p).state=TASK_RUNNING;
        }
    }
    sti();
    return 0;
}

int sys_sem_unlink(const char *name){
    char tmp[16]; /* 存储信号量的名称 */
    char c;
    int i;
    for(i=0;i<16;i++){
        c = get_fs_byte(name+i);  /* 该函数输入的是字符串的地址*/
        tmp[i]=c;
        if(c=='\0'){
            break;
        }
    }
    if(i>=16){ 
        printk("Semaphore name is too long\n");
        return NULL;
    }
    int res = sem_location(tmp);
    if(res !=-1){
        semaphores[res].value = 0;
    	strcpy(semaphores[res].name,"\0");
    	semaphores[res].occupied = 0;
    	return 0;
    }
    return -1;
}