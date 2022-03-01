#define _GNU_SOURCE
#include <stdio.h>
#include <sched.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <wait.h>
#include <memory.h>

int f(void* arg) {
    printf("clone pid %d trying execve\n", getpid());
    execve("/bin/sh", 0, 0);
    return 0;
}

char* STACK[8192];

void run() {
    int pid = clone(f, STACK + 8192, SIGCHLD, 0);
    printf("waiting pid %d\n", pid);
    FILE* fd = fopen("/sys/fs/cgroup/test/cgroup.procs", "w");
    printf("fd: %d\n", fd);
    fprintf(fd, "%d", pid);
    fclose(fd);
    waitpid(pid, 0, 0);
    printf("done\n");
}
