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
        for(y = 0; y!= 1000; y++){
        }
    }
}

void *th0 (void *data)
{
    struct sched_attr attr;
    int ret;
    unsigned int flags = 0;

    attr.sched_priority = 0;
    attr.sched_policy = SCHED_DEADLINE;
    attr.sched_runtime = 10000000;
    attr.sched_period = 30000000;
    attr.sched_deadline = 20000000;
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

void *th1 (void *data)
{
    struct sched_attr attr;
    int ret;
    unsigned int flags = 0;

    attr.sched_priority = 0;
    attr.sched_policy = SCHED_DEADLINE;
    attr.sched_runtime = 20000000;
    attr.sched_period = 60000000;
    attr.sched_deadline = 40000000;
    ret = sched_setattr(0, &attr, flags);

    if (ret < 0) {
        perror("sched_setattr");
        exit(-1);
    }

    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
    printf("thread th1 start %ld\n", gettid());

    task_1();
}

void *th2 (void *data)
{
    struct sched_attr attr;
    int ret;
    unsigned int flags = 0;

    attr.sched_priority = 0;
    attr.sched_policy = SCHED_DEADLINE;
    attr.sched_runtime = 30000000;
    attr.sched_period = 90000000;
    attr.sched_deadline = 60000000;
    ret = sched_setattr(0, &attr, flags);

    if (ret < 0) {
        perror("sched_setattr");
        exit(-1);
    }

    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
    printf("thread th2 start %ld\n", gettid());

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
    pthread_t t[8];
    int x;

    printf("main thread [%ld]\n", gettid());

    pthread_create(&t[0], NULL, th0, NULL);
    printf("t0\n");
    pthread_create(&t[1], NULL, th1, NULL);
    printf("t1\n");
    pthread_create(&t[2], NULL, th1, NULL);
    printf("t2\n");
    pthread_create(&t[3], NULL, th2, NULL);
    printf("t3\n");
    pthread_create(&t[4], NULL, th2, NULL);
    printf("t4\n");

    printf("releasing the condition\n");
    pthread_cond_broadcast(&cond);

    pthread_join(t[0], NULL);
    pthread_join(t[1], NULL);
    pthread_join(t[2], NULL);
    pthread_join(t[3], NULL);
    pthread_join(t[4], NULL);

    printf("main dies [%ld]\n", gettid());
    return 0;
}
