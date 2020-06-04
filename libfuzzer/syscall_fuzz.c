#include <stdio.h>
#include <sys/types.h>

#include <rump/rump.h>
#include <rump/rump_syscalls.h>

int val,val1;

static
void Initialize()
{
	if(rump_init()!=0)
		__builtin_trap();
}

int  LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
	val = (int)Data[1];
	val1 = (int)Data[2];
	Initialize();
	rump_sys_shutdown(val, val1);
	return 0;
}

