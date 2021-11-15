#include <asm/segment.h>
#include <errno.h>
#include <linux/kernel.h> 
//应该是在include目录下搜索

#define NAMELEN 23

char username[NAMELEN+1]; //写在内核中的全局变量

int sys_iam(const char *name){

    printk("Now we are in Kernel");
    unsigned int namelen = 0;
    int res =-1;
    int i;
    while(get_fs_byte(name+namelen)!='\0'){
        namelen++; //调用get_fs_byte来获得输入的字符串,namelen是字符串的长度
    }
    if(namelen<=NAMELEN){
        for(i=0;i<namelen;i++){
            username[i]=get_fs_byte(name+i);
        }
        username[i]='\0';  //字符串的结尾符号
        res =namelen;
    }else{
        printk("Error,the name's length is %d,longer than 23\n",namelen);
        res =-(EINVAL);
    }
    return res;
}

int sys_whoami(char *name,unsigned int size){
    unsigned int namelen=0;
    int i =0;
    int res=-1;
	while(username[namelen] != '\0'){
		namelen++;
    }
    if(namelen<size){  //满足长度
        for (i = 0; i < namelen; i++) {
		    put_fs_byte(username[i], name+i); //将内核态的数据输出到用户态上
		}
        put_fs_byte('\0',name+i);
        res =namelen;
    }else{
        printk("Error,the name's length is longer than %d\n",size);
        res = -(EINVAL);
    }
    return res;
}