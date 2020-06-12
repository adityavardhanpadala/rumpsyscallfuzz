/*
 * hfuzz-clang -fPIC -shared copy.c -o copy.so
 * honnggfuzz -E LD_PRELOAD=/path/to/copy.so -P -f corpus -- ./fuzzer
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int raise(int num)
{
	exit(0);
}
int rumpns_copyout(const void *kaddr, void *uaddr, size_t len)
{
	return 0;
}
int rumpns_copyin(const void *uaddr, void *kaddr, size_t len)
{
	memset(kaddr, 0, len);
	return 0;
}
int rumpns_copyinstr(const void *uaddr, void *kaddr, size_t len, size_t *done)
{
	memset(kaddr, 0, len);
	return 0;
}
int rumpns_copyoutstr(const void *kaddr, void *uaddr, size_t len, size_t *done)
{
	return 0;
}
