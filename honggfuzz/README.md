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

