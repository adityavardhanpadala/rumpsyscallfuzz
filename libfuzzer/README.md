clang -fsanitize=fuzzer -lrumpvfs -lrump -lrumpfs_ffs -lrumpuser -lrumpdev_disk -lrumpdev syscall_fuzz.ic

# Info

With this fuzzer we will be able to fuzz the syscalls in rump/include/rump/rump_syscalls.h .
This fuzzer right now is a fairly basic fuzzer that just fuzzes 10 syscalls handpicked for the sake simplicity.
