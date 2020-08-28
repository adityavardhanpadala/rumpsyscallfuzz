# Setup and Usage

notes on [adityapadala.com](https://adityapadala.com)

- Setup a NetBSD installation as detailed here [https://adityapadala.com/2020/04/20/Setting-Up-NetBSD-Kernel-Dev-Environment/](https://adityapadala.com/2020/04/20/Setting-Up-NetBSD-Kernel-Dev-Environment/)
- ssh into the vm.
- Now fetch the sources from either CVS or git.
```
export CVSROOT="anoncvs@anoncvs.NetBSD.org:/cvsroot"
cvs checkout -A -P src
(or)
git clone https://github.com/NetBSD/src
```

- Build the distribution with fuzzer coverage and required llvm toolchain.
```
./build.sh -j8 -N0 -U -u -V MAKECONF=/dev/null -V MKCOMPAT=no -V MKDEBUGLIB=yes -V MKDEBUG=yes -V MKSANITIZER=yes -V MKLIBCSANITIZER=yes -V USE_SANITIZER=fuzzer-no-link,address -V MKLLVM=yes -V MKGCC=no -V HAVE_LLVM=yes -O /public/netbsd.fuzzer distribution
```

- Fetch pkgsrc (if required)
```
cd /usr && cvs -q -z2 -d anoncvs@anoncvs.NetBSD.org:/cvsroot checkout -P pkgsrc
```

- Install honggfuzz from pkgsrc in devel/honggfuzz
```
git clone https://github.com/google/honggfuzz.git
add -static to COMMON_CFLAGS in honggfuzz/Makefile
cd honggfuzz
make install
```
- Fuzzing
```
mount -t null /dev /public/netsbd.fuzzer/destdir.amd64/dev
mount -t null /dev/pts /public/netbsd.fuzzer/destdir.amd64/dev/pts
mount -t null /tmp /public/netbsd.fuzzer/destdir.amd64/tmp
mkdir /public/netbsd.fuzzer/destdir.amd64/usr/pkg
mount -t null /usr/pkg /public/netbsd.fuzzer/destdir.amd64/usr/pkg

cp fuzzer.c /tmp
chroot destdir.amd64/
cd /tmp

#Set detect_container_overflow=0 to suppress container overflow errors.
#Set LIBC_UBSAN=a to abort on error.

ASAN_OPTIONS=detect_container_overflow=0 hfuzz-clang -fsanitize=address -lrump -lrumpdev -lrumpvfs -lrumpvfs_nofifofs syscall_fuzz.c

mkdir corpus
honggfuzz -E LIBC_UBSAN=a -P -f corpus/ -- ./a.out 

sysctl -w kern.defcorename='%n.%p.%t.core' #to define corefile name
and pass to honggfuzz: --rlimit_core=1000  #only pass when you need a core file or else we will be						 #spamming the /tmp directory with huge core files
```
