/*
build instructions:

./build.sh -j8 -N0 -U -u -V MAKECONF=/dev/null -V MKCOMPAT=no -V MKDEBUGLIB=yes -V MKDEBUG=yes -V MKSANITIZER=yes -V MKLIBCSANITIZER=yes -V USE_SANITIZER=fuzzer-no-link,address -V MKLLVM=yes -V MKGCC=no -V HAVE_LLVM=yes -O /public/netbsd.fuzzer distribution

mount -t null /dev /public/netsbd.fuzzer/destdir.amd64/dev
mount -t null /dev/pts /public/netbsd.fuzzer/destdir.amd64/dev/pts
mount -t null /tmp /public/netbsd.fuzzer/destdir.amd64/tmp
mkdir /public/netbsd.fuzzer/destdir.amd64/usr/pkg
mount -t null /usr/pkg /public/netbsd.fuzzer/destdir.amd64/usr/pkg

cp syscall_fuzz.c /tmp
chroot destdir.amd64/
cd /tmp

ASAN_OPTIONS=detect_container_overflow=0 hfuzz-clang -fsanitize=address,undefined -lrump -lrumpdev -lrumpvfs -lrumpvfs_nofifofs syscall_fuzz.c

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
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <rump/rump.h>
#include <rump/rump_syscalls.h>
#ifdef __cplusplus
}
#endif

#define DEBUG 1

#ifdef __cplusplus
#define  EXTERN extern "C"
#else
#define EXTERN
#endif

EXTERN int rump_syscall(int num, void *data, size_t dlen, register_t *retval);

int Initialized=0;

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

EXTERN void HF_ITER(uint8_t **buf, size_t *len);
EXTERN void HF_MEMGET(void *dst, size_t len);

static
void Initialize()
{
	// Initialise the rumpkernel only once.
	if(rump_init()!=0)
		__builtin_trap();
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

#ifdef DEBUG
        FILE *fp = fopen("/tmp/crashlog.txt.x","a+");
        fprintf(fp,"HF_GET: dst=%p len=%zu\n", dst, len);
        fclose(fp);
#endif
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
	
		bool rumpified;
                switch (syscall_val) {
                case 3:
                case 4:
                case 5:
                case 6:
                case 9:
                case 10:
                case 12:
                case 13:
                case 14:
                case 15:
                case 16:
                case 20:
                case 22:
                case 23:
                case 24:
                case 25:
                case 27:
                case 28:
                case 29:
                case 30:
                case 31:
                case 32:
                case 33:
                case 34:
                case 35:
                case 36:
                case 39:
                case 41:
                case 42:
                case 43:
                case 45:
                case 47:
                case 49:
                case 50:
                case 54:
                case 56:
                case 57:
                case 58:
                case 60:
                case 61:
                case 79:
                case 80:
                case 81:
                case 82:
                case 90:
                case 92:
                case 93:
                case 95:
                case 98:
                case 104:
                case 105:
                case 106:
                case 118:
                case 120:
                case 121:
                case 123:
                case 124:
                case 126:
                case 127:
                case 128:
                case 131:
                case 132:
                case 133:
                case 134:
                case 135:
                case 136:
                case 137:
                case 138:
                case 147:
                case 155:
                case 173:
                case 174:
                case 181:
                case 182:
                case 183:
                case 191:
                case 192:
                case 193:
                case 194:
                case 195:
                case 199:
                case 200:
                case 201:
                case 202:
                case 206:
                case 207:
                case 208:
                case 209:
                case 235:
                case 236:
                case 239:
                case 241:
                case 246:
                case 247:
                case 248:
                case 249:
                case 250:
                case 251:
                case 252:
                case 253:
                case 254:
                case 255:
                case 256:
                case 270:
                case 274:
                case 275:
                case 276:
                case 283:
                case 284:
                case 285:
                case 286:
                case 288:
                case 289:
                case 290:
                case 296:
                case 297:
                case 304:
                case 305:
                case 306:
                case 344:
                case 345:
                case 354:
                case 360:
                case 361:
                case 362:
                case 363:
                case 364:
                case 365:
                case 366:
                case 367:
                case 368:
                case 369:
                case 370:
                case 371:
                case 372:
                case 373:
                case 374:
                case 375:
                case 376:
                case 377:
                case 378:
                case 379:
                case 380:
                case 381:
                case 382:
                case 383:
                case 384:
                case 385:
                case 386:
                case 387:
                case 388:
                case 389:
                case 390:
                case 394:
                case 395:
                case 396:
                case 398:
                case 399:
                case 400:
                case 401:
                case 402:
                case 403:
                case 405:
                case 406:
                case 410:
                case 416:
                case 417:
                case 418:
                case 419:
                case 420:
                case 421:
                case 423:
                case 424:
                case 425:
                case 426:
                case 427:
                case 428:
                case 429:
                case 430:
                case 435:
                case 436:
                case 437:
                case 438:
                case 439:
                case 440:
                case 441:
                case 446:
                case 447:
                case 450:
                case 451:
                case 453:
                case 454:
                case 455:
                case 456:
                case 457:
                case 458:
                case 459:
                case 460:
                case 461:
                case 462:
                case 463:
                case 464:
                case 466:
                case 467:
                case 468:
                case 469:
                case 470:
                case 471:
                case 472:
                case 473:
                case 475:
                case 476:
                case 477:
                case 479:
                case 480:
                case 483:
                case 484:
                case 485:
                case 486:
                case 499:
                        rumpified = true;
			break;
                default:
                        rumpified = false;
			break;
                }

#ifdef DEBUG
                FILE *fp1 = fopen("/tmp/crashlog.txt.2","a+");
                fprintf(fp1,"SYSNO: %u rumpified=%s\n", syscall_val, rumpified ? "yes" : "no");
                fclose(fp1);
#endif

                if (!rumpified)
                        continue;
		
		uint64_t args[8];
		HF_MEMGET(&args[0], 8*sizeof(uint64_t));
	
#ifdef DEBUG
		FILE *fp = fopen("/tmp/crashlog.txt.3","a+");
		fprintf(fp,"__syscall(%" PRIu32 ", %#" PRIx64 ", %#" PRIx64 ", %#" PRIx64 ", %#" PRIx64 ", \
			%#" PRIx64 ", %#" PRIx64 ", %" PRIx64 ", %#" PRIx64 ")\n", syscall_val,  
		       		args[0],args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
		fclose(fp);
#endif

		rump_syscall(syscall_number, &args, sizeof(args), retval);
	}
	return 0;
}
