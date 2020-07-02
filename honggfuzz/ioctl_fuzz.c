/*
build instructions:

./build.sh -j8 -N0 -U -u -V MAKECONF=/dev/null -V MKCOMPAT=no -V MKDEBUGLIB=yes -V MKDEBUG=yes -V MKSANITIZER=yes -V MKLIBCSANITIZER=yes -V USE_SANITIZER=fuzzer-no-link,address -V MKLLVM=yes -V MKGCC=no -V HAVE_LLVM=yes -O /public/netbsd.fuzzer distribution

- add -static to COMMON_CFLAGS in Makefile and build honggfuzz

mount -t null /dev /public/netsbd.fuzzer/destdir.amd64/dev
mount -t null /dev/pts /public/netbsd.fuzzer/destdir.amd64/dev/pts
mount -t null /tmp /public/netbsd.fuzzer/destdir.amd64/tmp
mkdir /public/netbsd.fuzzer/destdir.amd64/usr/pkg
mount -t null /usr/pkg /public/netbsd.fuzzer/destdir.amd64/usr/pkg

cp syscall_fuzz.c /tmp
chroot destdir.amd64/
cd /tmp

- Set detect_container_overflow=0 to suppress container overflow errors.

- Set LIBC_UBSAN=a to abort on error.

ASAN_OPTIONS=detect_container_overflow=0 hfuzz-clang -fsanitize=address -lrump -lrumpdev -lrumpvfs -lrumpvfs_nofifofs syscall_fuzz.c

mkdir corpus
honggfuzz -E LIBC_UBSAN=a -P -f corpus/ -- ./a.out 

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
#include <sys/socket.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <ufs/ufs/ufsmount.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>

//#define DEBUG 0
//#define CRASH_REPR

#ifdef __cplusplus
extern "C" {
#endif
#include <rump/rump.h>
#include <rump/rump_syscalls.h>
#ifdef __cplusplus
}
#endif

static uint8_t *buf;
static size_t buflen;
static uint8_t zero[255];
static bool zeroed;
static bool cold;

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif

static int Initialized=0;

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

#ifdef CRASH_REPR
void HF_ITER(uint8_t **buf, size_t *len) {
        uint8_t ret[1];
	ret[0] = getchar();
        *buf = (uint8_t *)ret;
        *len = (size_t)sizeof(ret);
        return;
}
#else
EXTERN void HF_ITER(uint8_t **buf, size_t *len);
#endif
EXTERN void HF_MEMGET(void *dst, size_t len);

static
void Initialize(void)
{
	// Initialise the rumpkernel only once.
	if(rump_init()!=0)
		__builtin_trap();
}

static
void Iteration(void)
{
	int fd;

	/* Close all open file descriptors */
	rump_sys_fcntl(0, F_CLOSEM);

	for (fd = 0; fd < 100; fd += 2) {
		rump_sys_pipe2(NULL /* copyout */, O_RDONLY);
	}
	for (fd = 100; fd < 255; fd++) {
		rump_sys_open("/tmp/file", O_RDONLY);
	}
}

EXTERN int
raise(int num)
{
	exit(0);
}

EXTERN int
rumpns_copyin(const void *uaddr, void *kaddr, size_t len)
{
	int error = 0;

	if (cold == true) {
		int (*original)(const void *uaddr, void *kaddr, size_t len);
		original = dlsym(RTLD_NEXT, "rumpns_copyin");
		return (*original)(uaddr, kaddr, len);
	}

	if (len == 0)
 		return 0;

 	HF_MEMGET(kaddr, len);

#ifdef DEBUG
        FILE *fp = fopen("/tmp/crashlog.txt.0","a+");
        fprintf(fp,"copyin: len=%zu buf=0x", len);
        for (size_t i = 0; i < len; i++)
                fprintf(fp,"%02x", (unsigned char)((char*)kaddr)[i]);
        fprintf(fp,"\n");
        fclose(fp);
#endif

	return error;
}

EXTERN int
rumpns_copyout(const void *kaddr, void *uaddr, size_t len)
{
	return 0;
}

EXTERN int
rumpns_copyinstr(const void *uaddr, void *kaddr, size_t len, size_t *done)
{
	size_t size;

	if (len == 0)
		return EFAULT;

	if (cold == true) {
		int (*original)(const void *uaddr, void *kaddr, size_t len, size_t *done);
		original = dlsym(RTLD_NEXT, "rumpns_copyinstr");
		return (*original)(uaddr, kaddr, len, done);
	}

	HF_MEMGET(kaddr, len-1);

	((char*)kaddr)[len-1]=0;

	size = strlen(kaddr) + 1;
	if (done)
		*done = size;

#ifdef DEBUG
        FILE *fp = fopen("/tmp/crashlog.txt.1","a+");
        fprintf(fp,"copyinstr: len=%zu buf=0x", strlen((char *)kaddr) + 1);
        for (size_t i = 0; i < size; i++)
                fprintf(fp,"%02x", (unsigned char)((char*)kaddr)[i]);
        fprintf(fp,"\n");
        fclose(fp);
#endif

	return 0;
}
EXTERN int
rumpns_copyoutstr(const void *kaddr, void *uaddr, size_t len, size_t *done)
{
	len = MIN(strnlen(uaddr, len), len) + 1;

	if (done)
		*done = len;
	return 0;
}

EXTERN void
HF_MEMGET(void *dst, size_t len)
{
    size_t diff;

    if (len == 0)
       return;

    pthread_mutex_lock(&mtx);
    do {
        if (buflen == 0) {
            buf = zero;
            buflen = sizeof(zero);
	    zeroed = true;
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

#ifdef DEBUG
        FILE *fp = fopen("/tmp/crashlog.txt.x","a+");
        fprintf(fp,"HF_GET: dst=%p len=%zu\n", dst, len);
        fclose(fp);
#endif
}

static uint8_t
get_u8(void)
{
	uint8_t u;
	HF_MEMGET(&u, sizeof(u));
	return u;
}

static int
get_int(void)
{
	int u;
	HF_MEMGET(&u, sizeof(u));
	return u;
}

static unsigned long
get_ulong(void)
{
	unsigned long u;
	HF_MEMGET(&u, sizeof(u));
	return u;
}

int
main(int argc, char **argv)
{
	static uint64_t count = 0;

	cold = true;

	if(!Initialized){
		Initialize();
		Initialized=1;
	}

	cold = false;


	for (;;) {
		count++;
        	HF_ITER(&buf, &buflen);

#ifdef DEBUG
		{
	        FILE *fp = fopen("/tmp/crashlog.txt.1","a+");
        	fprintf(fp,"HF_ITER:pid=%d:count=%" PRIu64 ":buflen=%zu\n", getpid(), count, buflen);
	        fclose(fp);
		}
#endif

		zeroed = false;

#ifdef DEBUG
		{
	        FILE *fp = fopen("/tmp/crashlog.txt.1","a+");
        	fprintf(fp,"HF_ITER:::pid=%d:count=%" PRIu64 ":buflen=%zu:zeroed=%d\n", getpid(), count, buflen, zeroed ? 1 : 0);
	        fclose(fp);
		}
#endif

		cold = true;
		Iteration();
		cold = false;

#ifdef DEBUG
		{
	        FILE *fp = fopen("/tmp/crashlog.txt.1","a+");
        	fprintf(fp,"HF_ITER:pid=%d:count=%" PRIu64 ":buflen=%zu:zeroed=%d\n", getpid(), count, buflen, zeroed ? 1 : 0);
	        fclose(fp);
		}
#endif

		while (zeroed == false) {
//		__builtin_trap();
#ifdef DEBUG
		{
	        FILE *fp = fopen("/tmp/crashlog.txt.1","a+");
        	fprintf(fp,"HF_ITER:pid=%d:count=%" PRIu64 ":buflen=%zu...\n", getpid(), count, buflen);
	        fclose(fp);
		}
#endif

			register_t retval[2];
			uint16_t syscall_number;
			HF_MEMGET(&syscall_number, 2);

			uint16_t syscall_val = syscall_number & 511;
	
                	switch (syscall_val) {
	                case 54: /* ioctl */
				rump_sys_ioctl(get_u8(), get_ulong());
				break;
	                case 90: /* dup2 */
				rump_sys_dup2(get_u8(), get_u8());
				break;
	                case 453:
				rump_sys_pipe2(NULL /* copyout */, get_int());
				break;
			}
                }
	}
	return 0;
}
