#define _GNU_SOURCE
#include <stdio.h>
#include <sched.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <wait.h>
#include <memory.h>
#include <sys/stat.h>

#include <errno.h>
#include <sys/capability.h>
#include <sys/mount.h>

const char* cgroup_root = "/sys/fs/cgroup/isolate/";
const char* box_id = "test/";

int init() {
    pid_t pid = getpid();
    printf("[%d] entering init func\n", pid);
    char cgroup_path[1024];
    sprintf(cgroup_path, "%s%s", cgroup_root, box_id);
    printf("[%d] creating %s\n", pid, cgroup_path);
    if (mkdir(cgroup_path, 0755) < 0) {
        printf("[%d] mkdir failed: %s\n", pid, strerror(errno));
        return -1;
    }
    printf("[%d] exiting init\n", pid);
    return 0;
}

int prepare_root() {
    if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) < 0) {
        return -1;
    }
    if (mkdir("root", 0755) < 0) {
        return -1;
    }
    if (mkdir("root/usr", 0755) < 0) {
        return -1;
    }
    if (mount("/usr", "root/usr", NULL, MS_BIND, NULL) < 0) {
        return -1;
    }
    if (mount(NULL, "root/usr", NULL, MS_BIND | MS_REMOUNT | MS_RDONLY, NULL) < 0) {
        return -1;
    }
    if (mkdir("root/bin", 0755) < 0) {
        return -1;
    }
    if (mount("/bin", "root/bin", NULL, MS_BIND, NULL) < 0) {
        return -1;
    }
    if (mount(NULL, "root/bin", NULL, MS_BIND | MS_REMOUNT | MS_RDONLY, NULL) < 0) {
        return -1;
    }
    if (mkdir("root/sbin", 0755) < 0) {
        return -1;
    }
    if (mount("/sbin", "root/sbin", NULL, MS_BIND, NULL) < 0) {
        return -1;
    }
    if (mount(NULL, "root/sbin", NULL, MS_BIND | MS_REMOUNT | MS_RDONLY, NULL) < 0) {
        return -1;
    }
    if (mkdir("root/lib", 0755) < 0) {
        return -1;
    }
    if (mount("/lib", "root/lib", NULL, MS_BIND, NULL) < 0) {
        return -1;
    }
    if (mount(NULL, "root/lib", NULL, MS_BIND | MS_REMOUNT | MS_RDONLY, NULL) < 0) {
        return -1;
    }
    if (mkdir("root/lib64", 0755) < 0) {
        return -1;
    }
    if (mount("/lib64", "root/lib64", NULL, MS_BIND, NULL) < 0) {
        return -1;
    }
    if (mount(NULL, "root/lib64", NULL, MS_BIND | MS_REMOUNT | MS_RDONLY, NULL) < 0) {
        return -1;
    }
    if (mkdir("root/proc", 0755) < 0) {
        return -1;
    }
    if (mount("proc", "root/proc", "proc", 0, NULL) < 0) {
        return -1;
    }
    if (mkdir("root/box", 0755) < 0) {
        return -1;
    }
    if (mount("box", "root/box", NULL, MS_BIND, NULL) < 0) {
        return -1;
    }
    return 0;
}

int box(void* arg) {
    pid_t pid = getpid();
    printf("[%d] entering box func\n", pid);
    uid_t orig_euid = getuid();
    setresuid(0, 0, 0);
    if (prepare_root() < 0) {
        printf("[%d] prepare_root failed: %s\n", pid, strerror(errno));
        return -1;
    }
    if (chdir("root/box") < 0) {
        printf("[%d] chdir failed: %s\n", pid, strerror(errno));
        return -1;
    }
    if (chroot("..") < 0) {
        printf("[%d] chroot failed: %s\n", pid, strerror(errno));
        return -1;
    }
    setresuid(orig_euid, orig_euid, orig_euid);
    printf("[%d] original uid %d\n", pid, orig_euid);
    if (execve("/bin/sh", NULL, NULL) < 0) {
        printf("[%d] execve failed: %s\n", pid, strerror(errno));
    }
    printf("[%d] exiting box func\n", pid);
    return 0;
}

char STACK[8192]; // something strange

int run() {
    pid_t pid = getpid();
    printf("[%d] entering run func\n", pid);
    int flags = SIGCHLD | CLONE_NEWNS | CLONE_NEWPID;
    pid_t box_pid = clone(box, STACK + 8192, flags, 0); // I really don't know how does it work
    if (box_pid < 0) {
        printf("[%d] clone failed: %s\n", pid, strerror(errno));
        return -1;
    }
    printf("[%d] waiting box %d\n", pid, box_pid);
    waitpid(box_pid, 0, 0);
    printf("[%d] exiting run func\n", pid);
}

int clean() {
    pid_t pid = getpid();
    printf("[%d] entering clean func\n", pid);
    char cgroup_path[1024];
    sprintf(cgroup_path, "%s%s", cgroup_root, box_id);
    printf("[%d] deleting %s\n", pid, cgroup_path);
    if (rmdir(cgroup_path) < 0) {
        printf("[%d] rmdir failed: %s\n", pid, strerror(errno));
        return -1;
    }
    printf("[%d] exeting clean func\n", pid);
}
