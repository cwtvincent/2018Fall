#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <linux/sched.h>

#define gettid() syscall(__NR_gettid)

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond =  PTHREAD_COND_INITIALIZER;

struct sched_attr {
    __u32 size;

    __u32 sched_policy;
    __u64 sched_flags;

    __s32 sched_nice;

    /* SCHED_FIFO, SCHED_RR */
    __u32 sched_priority;

    __u64 sched_runtime;
    __u64 sched_deadline;
    __u64 sched_period;
};

int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags)
{
    return syscall(__NR_sched_setattr, pid, attr, flags);
}

int sched_getattr(pid_t pid, struct sched_attr *attr, unsigned int size, unsigned int flags)
{
    return syscall(__NR_sched_getattr, pid, attr, size, flags);
}

void task_1()
{
    int x = 0, y = 0;
    for(x = 0;x != 10000; x++){
        for(y = 0; y!= 10000; y++){
        }
    }
}

void *unlock (void *data)
{
    pthread_cond_broadcast(&cond);
}

void *th0 (void *data)
{
    struct sched_attr attr;
    int ret;
    unsigned int flags = 0;

    attr.sched_priority = 99;
    attr.sched_policy = SCHED_RR;
    ret = sched_setattr(0, &attr, flags);

    if (ret < 0) {
        perror("sched_setattr");
        exit(-1);
    }

    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
    printf("thread th0 start %ld\n", gettid());

    task_1();
}

void set_cpu(pthread_t t, cpu_set_t cpu, int cpuindex)
{
    int x;

    CPU_ZERO(&cpu);
    CPU_SET(cpuindex, &cpu);
    pthread_setaffinity_np(t, sizeof(cpu), &cpu);
}

int main (int argc, char **argv)
{
    pthread_t t[8], ul;
    cpu_set_t c0, c1, c2, c3;
    int x;

    printf("main thread [%ld]\n", gettid());

    pthread_create(&t[0], NULL, th0, NULL);
    set_cpu(t[0], c0, 0);
    printf("t0\n");
    pthread_create(&t[1], NULL, th0, NULL);
    set_cpu(t[1], c1, 0);
    printf("t1\n");
    pthread_create(&t[2], NULL, th0, NULL);
    set_cpu(t[2], c2, 0);
    printf("t2\n");
    pthread_create(&t[3], NULL, th0, NULL);
    set_cpu(t[3], c3, 0);    
    printf("t3\n");
    pthread_create(&t[4], NULL, th0, NULL);
    set_cpu(t[4], c0, 0);
    printf("t4\n");

    printf("releasing the condition\n");
    pthread_create(&ul, NULL, unlock, NULL);

    pthread_join(t[0], NULL);
    pthread_join(t[1], NULL);
    pthread_join(t[2], NULL);
    pthread_join(t[3], NULL);
    pthread_join(t[4], NULL);

    printf("main dies [%ld]\n", gettid());
    return 0;
}
