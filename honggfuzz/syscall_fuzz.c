/*
build instructions:

mount -t null /dev destdir.amd64/dev
mount -t null /dev/pts destdir.amd64/dev/pts
mount -t null /tmp destdir.amd64/tmp
mkdir destdir.amd64/usr/pkg
mount -t null /usr/pkg destdir.amd64/usr/pkg

cp syscall_fuzz.c /tmp
chroot destdir.amd64/
cd /tmp 

hfuzz-clang -lrump -lrumpdev -lrumpvfs -lrumpvfs_nofifofs syscall_fuzz.c
mkdir corpus
honggfuzz -P -f corpus/ -- ./a.out 

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/lwp.h>
#include <inttypes.h>
#include <pthread.h>

#include <rump/rump.h>
#include <rump/rump_syscalls.h>

#define DEBUG 1

int rump_syscall(int num, void *data, size_t dlen, register_t *retval);

int Initialized=0;

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

extern void HF_ITER(uint8_t **buf, size_t *len);
void HF_MEMGET(void *dst, size_t len);

static
void Initialize()
{
	// Initialise the rumpkernel only once.
	if(rump_init()!=0)
		__builtin_trap();
}

int
raise(int num)
{
	exit(0);
}
int
rumpns_copyin(const void *uaddr, void *kaddr, size_t len)
{
	int error = 0;

	if (len == 0)
 		return 0;

 	HF_MEMGET(kaddr, len);
	
	return error;
}

int
rumpns_copyout(const void *kaddr, void *uaddr, size_t len)
{
	return 0;
}
int rumpns_copyinstr(const void *uaddr, void *kaddr, size_t len, size_t *done)
{
	if (len == 0)
		return EFAULT;

	HF_MEMGET(kaddr, len-1);

	((char*)kaddr)[len-1]=0;

	if (done)
		done = strlen(kaddr) + 1;

	return 0;
}
int rumpns_copyoutstr(const void *kaddr, void *uaddr, size_t len, size_t *done)
{
	len = MIN(strnlen(uaddr, len), len) + 1;

	if (done)
		*done = len;
	return 0;
}

void
HF_MEMGET(void *dst, size_t len)
{
    static uint8_t *buf;
    static size_t buflen;
    size_t diff;

    if (len == 0)
       return;

    pthread_mutex_lock(&mtx);
    do {
        if (buflen == 0) {
            HF_ITER(&buf, &buflen);
            continue;
        }
        diff = MIN(buflen, len);
        memcpy(dst, buf, diff);
        buf += diff;
        buflen -= diff;
        dst += diff;
        len -= diff;
    } while (len > 0);
    pthread_mutex_unlock(&mtx);
}

int
main(int argc, char **argv)
{
	if(!Initialized){
		Initialize();
		Initialized=1;
	}
	for(;;){
		register_t retval[2];
		uint16_t syscall_number;
		HF_MEMGET(&syscall_number, 2);

		uint16_t syscall_val = syscall_number & 511;
	
		uint64_t args[8];
		HF_MEMGET(&args[0], 8*sizeof(uint64_t));
	
#ifdef DEBUG
		FILE *fp = fopen("/tmp/crashlog.txt","a+");
		fprintf(fp,"__syscall(%"PRIu32", %#"PRIx64", %#"PRIx64", %#"PRIx64", %#"PRIx64", \
			%#"PRIx64", %#"PRIx64", %"PRIx64", %#"PRIx64")\n", syscall_val,  
		       		args[0],args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
		fclose(fp);
#endif

		rump_syscall(syscall_number, &args, sizeof(args), retval);
	}
	return 0;
}
