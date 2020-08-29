This repository hosts the code written for GSOC 2020 with NetBSD for the Project <b>Rumpkernel Syscall Fuzzing</b>.

If you are not sure what a rumpkernel is then this might help [Rump Kernels](https://adityapadala.com/2020/04/01/Rump-Kernels/)

Posts on NetBSD blog detailing the work I did during my coding period. 

Blog Post 1 - [http://blog.netbsd.org/tnf/entry/gsoc_reports_fuzzing_rumpkernel_syscalls](http://blog.netbsd.org/tnf/entry/gsoc_reports_fuzzing_rumpkernel_syscalls)

Blog Post 2 - [http://blog.netbsd.org/tnf/entry/gsoc_reports_fuzzing_rumpkernel_syscalls1](http://blog.netbsd.org/tnf/entry/gsoc_reports_fuzzing_rumpkernel_syscalls1)

# Setup and Usage

- Setup a NetBSD installation as detailed here [https://adityapadala.com/2020/04/20/Setting-Up-NetBSD-Kernel-Dev-Environment/](https://adityapadala.com/2020/04/20/Setting-Up-NetBSD-Kernel-Dev-Environment/)
- SSH into the vm.
- Now fetch the NetBSD sources from either CVS or git.
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

- Build honggfuzz statically
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
