#define _GNU_SOURCE
#include <stdio.h>
#include <sched.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <wait.h>
#include <memory.h>

void debug(int pid, const char* message, int count) {
    printf("[%d] %s\n", pid, message);
    for (int i = 0; i < count; i++) {
        sleep(1);
        printf("[%d] %s\n", pid, message);
    }
}

int f(void* args) {
    int pid = getpid();
    debug(pid, "preparing", 0);
    int* pipe_fds = (int*)args, fd = pipe_fds[0];
    char c;
    debug(pid, "waiting for start command", 0);
    read(fd, &c, 1);
    debug(pid, "executing", 2);
    debug(pid, "exiting", 0);
    return 0;
}

char* STACK[8192]; // something strange...

void run() {
    int pid = getpid();
    debug(pid, "preparing", 0);
    int pipe_fds[2];
    debug(pid, "creating pipe", 0);
    if (pipe(pipe_fds)) {
        debug(pid, "pipe creating error", 0);
        return;
    }
    int fd = pipe_fds[1];
    // preparing namespaces here
    debug(pid, "cloning", 0);
    int child_pid = clone(f, STACK + 8192, SIGCHLD, (void*)pipe_fds); // something very strange...
    debug(pid, "doing something else", 2);
    debug(pid, "creating cgroup", 0);
    // preparing cgroup here
    FILE* file = fopen("/sys/fs/cgroup/test/cgroup.procs", "w");
    if (!file) {
        debug(pid, "adding proc to cgroup error", 0);
        return;
    }
    fprintf(file, "%d", child_pid);
    fclose(file);
    debug(pid, "doing something else", 2);
    char c;
    debug(pid, "sending start command", 0);
    write(fd, &c, 1);
    debug(pid, "waiting child", 0);
    waitpid(child_pid, 0, 0);
    debug(pid, "exiting", 0);
}
