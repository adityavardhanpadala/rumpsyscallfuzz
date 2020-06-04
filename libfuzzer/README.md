clang -fsanitize=fuzzer -lrumpvfs -lrump -lrumpfs_ffs -lrumpuser -lrumpdev_disk -lrumpdev syscall_fuzz.c
