# Setup
- Setup a NetBSD installation as detailed here [https://adityapadala.com/2020/04/20/Setting-Up-NetBSD-Kernel-Dev-Environment/](https://adityapadala.com/2020/04/20/Setting-Up-NetBSD-Kernel-Dev-Environment/)
- Now fetch the sources in the vm from either CVS or git.
```
export CVSROOT="anoncvs@anoncvs.NetBSD.org:/cvsroot"
cvs checkout -A -P src
(or)
git clone https://github.com/NetBSD/src
```
- Build the distribution with fuzzer coverage.
```
./build.sh -j8 -N0 -U -u -V MAKECONF=/dev/null -V MKCOMPAT=no -V MKDEBUGLIB=yes -V MKDEBUG=yes -V MKSANITIZER=yes -V USE_SANITIZER=fuzzer-no-link -V MKLLVM=yes -V MKGCC=no -V HAVE_LLVM=yes -O ../destdir distribution
```
- Fetch pkgsrc
```
cd /usr && cvs -q -z2 -d anoncvs@anoncvs.NetBSD.org:/cvsroot checkout -P pkgsrc
```
- Install honggfuzz from pkgsrc in devel/honggfuzz
(You might want to have a proper llvm-toolchain installed before)
```
cd /usr/pkgsrc/devel/honggfuzz
make install
```
- Now chroot into the destdir
```
mount -t null /dev destdir/dev
mount -t null /dev/pts destdir/dev/pts
mount -t null /tmp destdir/tmp
mkdir -p destdir/usr/pkg
mount -t null /usr/pkg destdir/usr/pkg
chroot destdir
```
- Place your fuzzer.c in /tmp
- Compile it using hfuzz-clang with appropriate libraries
```
hongfuzz -P -f ./corpus -- ./a,out ;to test your fuzzer
```
 
