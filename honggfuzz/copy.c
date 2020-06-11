#include <stdlib.h>
#include <string.h>
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
int copyinstr(const void *uaddr, void *kaddr, size_t len, size_t *done)
{
	memset(kaddr, 0, len);
	return 0;
}
int copyoutstr(const void *kaddr, void *uaddr, size_t len, size_t *done)
{
	return 0;
}
