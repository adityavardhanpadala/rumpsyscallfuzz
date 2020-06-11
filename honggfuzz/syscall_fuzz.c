#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <inttypes.h>
#include <rump/rump.h>
#include <rump/rump_syscalls.h>

#define DEBUG 1

int rump_syscall(int num, void *data, size_t dlen, register_t *retval);

int val,val1,Initialized=0;

static
void Initialize()
{
	// Initialise the rumpkernel only once.
	if(rump_init()!=0)
		__builtin_trap();
}

int raise(int num)
{
	exit(0);
}

int copyout(const void *kaddr, void *uaddr, size_t len) 
{
       	return 0;
}
int copyin(const void *uaddr, void *kaddr, size_t len) 
{ 
	memset(kaddr, 0, len); 
	return 0; 
}

int getsize()
{
	/*To Do*/
	return 0;
}

int
LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
	if (Size != 2 + 8 * sizeof(uint64_t))
		  return 0;
	
	if(!Initialized){
		Initialize();
		Initialized=1;
	}
	register_t retval[2];
	uint16_t syscall_number;
	memcpy(&syscall_number,Data,2);

	uint16_t syscall_val = syscall_number & 511;
	Data += 2;
	Size -= 2;
	
	uint64_t args[8];
	memcpy(&args[0],Data, 8*sizeof(uint64_t));
	
	#ifdef DEBUG
	FILE *fp = fopen("/tmp/crashlog.txt","w+");
	fprintf(fp,"__syscall(%"PRIu32", \
0x%"PRIx64", 0x%"PRIx64", 0x%"PRIx64", 0x%"PRIx64", 0x%"PRIx64", 0x%"PRIx64", 0x%"PRIx64" ,0x%"PRIx64")",\
syscall_val, args[0],args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
	fclose(fp);
	#endif

	rump_syscall(syscall_number, &args, sizeof(args), retval);
	
	return 0;
}
/*
int main()
{
	rump_init();
	char *buf = "aaaa";
	register_t retval[2];
	printf("%d--%d\n",retval[0],retval[1]);
	rump_syscall(3,buf,10,retval);
	printf("%d--%d\n",retval[0],retval[1]);
	return 0;
}
*/
