# My Setup

notes on [adityapadala.com](https://adityapadala.com)

- Setup a NetBSD installation as detailed here [https://adityapadala.com/2020/04/20/Setting-Up-NetBSD-Kernel-Dev-Environment/](https://adityapadala.com/2020/04/20/Setting-Up-NetBSD-Kernel-Dev-Environment/)
- Now fetch the sources in the vm from either CVS or git.
```
export CVSROOT="anoncvs@anoncvs.NetBSD.org:/cvsroot"
cvs checkout -A -P src
(or)
git clone https://github.com/NetBSD/src
```
- Build the distribution with fuzzer coverage and required llvm toolchain.
```
./build.sh -j8 -N0 -U -u -V MAKECONF=/dev/null -V MKCOMPAT=no -V MKDEBUGLIB=yes -V MKDEBUG=yes -V MKSANITIZER=yes -V USE_SANITIZER=fuzzer-no-link -V MKLLVM=yes -V MKGCC=no -V HAVE_LLVM=yes -O ../destdir distribution
```
- Fetch pkgsrc
```
cd /usr && cvs -q -z2 -d anoncvs@anoncvs.NetBSD.org:/cvsroot checkout -P pkgsrc
```

- Install honggfuzz from pkgsrc in devel/honggfuzz
```
cd /usr/pkgsrc/devel/honggfuzz
make install
```
