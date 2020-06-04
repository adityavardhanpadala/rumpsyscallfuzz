#include <sys/types.h>

#include <rump/rump.h>
#include <rump/rump_syscalls.h>

#include <stdio.h>

/*
int
rump_syscall(int num, void *data, size_t dlen, register_t *retval)
*/

int ret,sysnum = 3;
int len = 8;
register_t rptr;

int Initialise()
{
	rump_init();
}

int  LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
	Initialise();	
	ret = rump_sys_read(sysnum,(void *)Data,len);
	printf("%d",ret);
	return 0;
}
