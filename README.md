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
- Build the distribution with fuzzer coverage.
```
./build.sh -j8 -N0 -U -u -V MAKECONF=/dev/null -V MKCOMPAT=no -V MKDEBUGLIB=yes -V MKDEBUG=yes -V MKSANITIZER=yes -V USE_SANITIZER=fuzzer-no-link -V MKLLVM=yes -V MKGCC=no -V HAVE_LLVM=yes -O ../destdir distribution
```
- Fetch pkgsrc
```
cd /usr && cvs -q -z2 -d anoncvs@anoncvs.NetBSD.org:/cvsroot checkout -P pkgsrc
```
- Install LLVM-toolchain
```
git clone https://github.com/llvm/llvm-project
cd llvm-project
mkdir build
cd build
cmake -G ninja DLLVM_ENABLE_PROJECTS="clang;compiler-rt;clang-tools-extra;libcxx;libcxxabi" ../llvm
ninja
```

- Install honggfuzz from pkgsrc in devel/honggfuzz
```
cd /usr/pkgsrc/devel/honggfuzz
make install
```
