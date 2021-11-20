#define __LIBRARY__
#include <unistd.h>
#include <errno.h>


_syscall1(int, iam, const char*, name)

#define NAMELEN 100
char name[NAMELEN];

/* argc 显示命令的个数,argv显示命令具体的值,其中*argv[]的形式是确定的 */
/* 规定第二个参数必须是二级指针,arg[]是第一级指针,加*号之后变成二级指针*/
/* argc[] 是一个string */
int main(int argc, char *argv[]) {
	int res;
	int namelen = 0;
	if (2 <= argc) {
		while ((name[namelen] = argv[1][namelen]) != '\0')
			namelen++; 
		printf("iam.c: %s, %d\n", name, namelen);
		res = iam(name);
			errno = EINVAL;
		return res;
	}
}
