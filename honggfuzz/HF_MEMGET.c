#define _KERNTYPES
#include <sys/types.h>
#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <pthread.h>
#include <rump/rump.h>
#include <rump/rump_syscalls.h>
#include <stdio.h>
#include <unistd.h>

#define DEBUG 1

int rump_syscall(int num, void *data, size_t dlen, register_t *retval);

// Get input from the fuzzer
extern void HF_ITER(uint8_t **buf, size_t *len);

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static
void Initialize()
{
        // Initialise the rumpkernel only once.
        if(rump_init()!=0)
                __builtin_trap();
}

static void
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
        Initialize();

        for (;;) {
                register_t retval[2];
                uint16_t syscall_number;
                HF_MEMGET(&syscall_number, 2);

                uint16_t syscall_val = syscall_number & 511;
        
                uint64_t args[8];
                HF_MEMGET(&args[0], 8*sizeof(uint64_t));
        
#ifdef DEBUG
                FILE *fp = fopen("/tmp/crashlog.txt","a+");
                fprintf(fp,"__syscall(%"PRIu32",
                        0x%"PRIx64", 0x%"PRIx64", 0x%"PRIx64", 0x%"PRIx64", 0x%"PRIx64", 0x%"PRIx64", 0x%"PRIx64" ,0x%"PRIx64")",
                        syscall_val, args[0],args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
                fclose(fp);
#endif

                rump_syscall(syscall_number, &args, sizeof(args), retval);
        }
        
        return 0;
}
