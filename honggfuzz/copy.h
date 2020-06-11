#ifndef COPY_H

#define COPY_H

int raise(int num);
int copyout(const void *kaddr, void *uaddr, size_t len);
int copyin(const void *uaddr, void *kaddr, size_t len);
int copyinstr(const void *uaddr, void *kaddr, size_t len, size_t *done);
int copyoutstr(const void *kaddr, void *uaddr, size_t len, size_t *done);

#endif
