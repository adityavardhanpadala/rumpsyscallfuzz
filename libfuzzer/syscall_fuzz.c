#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <rump/rump.h>
#include <rump/rump_syscalls.h>

#define SYSCALLS 10
#define INTSYSCALLS 6
#define INT2SYSCALLS 4

int val,val1,Initialized=0;

typedef int (*fp)(int); //syscalls that take only one int as argument
typedef int (*fp2)(int,int); //syscalls that take int,int as argument

fp sys_array1[6] = {rump_sys_dup, rump_sys_close, rump_sys_fsync, rump_sys_fdatasync, rump_sys_fchroot, rump_sys_kqueue1};

fp2 sys_array2[4] = {rump_sys_dup2, rump_sys_listen, rump_sys_flock, rump_sys_shutdown};

static
void Initialize()
{
	if(rump_init()!=0)
		__builtin_trap();
}

int  LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
	if(!Initialized){
		Initialize();
		Initialized=1;
	}

	val = (int)Data[1];
	val1 = (int)Data[2];
	
	int random = rand() % SYSCALLS;

	if(random < INTSYSCALLS)
	{
		sys_array1[random](val);
	}
	else if(random > INTSYSCALLS)
	{
		sys_array2[random-INTSYSCALLS](val,val1);
	}

	return 0;
}

