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

void task_0()
{
    int x = 0, y = 0;
    for(x = 0;x < 100000; x++){
        for(y = 0; y < 20000; y++){
        }
    }
}
void task_1()
{
    int x = 0, y = 0;
    for(x = 0;x < 100000; x++){
        for(y = 0; y< 20000; y++){
        }
    }
}
void task_2()
{
    int x = 0, y = 0;
    for(x = 0;x < 100000; x++){
        for(y = 0; y< 20000; y++){
        }
    }
}

void *th0 (void *data)
{
    struct sched_attr attr;
    int ret;
    unsigned int flags = 0;
    attr.sched_priority = 90;
    attr.sched_policy = SCHED_RR;
    ret = sched_setattr(0, &attr, flags);

    if (ret < 0) {
        perror("sched_setattr");
        exit(-1);
    }

    //pthread_mutex_lock(&mutex);
    //pthread_cond_wait(&cond, &mutex);
    //pthread_mutex_unlock(&mutex);

        printf("thread th0 start %ld\n", gettid());
        task_0();
        printf("thread th0 end %ld\n", gettid());
    return NULL;
}

void *th1 (void *data)
{
    struct sched_attr attr;
    int ret;
    unsigned int flags = 0;
    attr.sched_priority = 80;
    attr.sched_policy = SCHED_RR;
    ret = sched_setattr(0, &attr, flags);

    if (ret < 0) {
        perror("sched_setattr");
        exit(-1);
    }

    //pthread_mutex_lock(&mutex);
    //pthread_cond_wait(&cond, &mutex);
    //pthread_mutex_unlock(&mutex);

        printf("thread th1 start %ld\n", gettid());
        task_1();
        printf("thread th1 end %ld\n", gettid());
    return NULL;
}

void *th2 (void *data)
{
    struct sched_attr attr;
    int ret;
    unsigned int flags = 0;
    attr.sched_priority = 70;
    attr.sched_policy = SCHED_RR;
    ret = sched_setattr(0, &attr, flags);

    if (ret < 0) {
        perror("sched_setattr");
        exit(-1);
    }

    //pthread_mutex_lock(&mutex);
    //pthread_cond_wait(&cond, &mutex);
    //pthread_mutex_unlock(&mutex);

        printf("thread th2 start %ld\n", gettid());
        task_2();
        printf("thread th2 end %ld\n", gettid());
    return NULL;
}

void *th3 (void *data)
{
    struct sched_attr attr;
    int ret;
    unsigned int flags = 0;
    attr.sched_priority = 90;
    attr.sched_policy = SCHED_RR;
    ret = sched_setattr(0, &attr, flags);

    if (ret < 0) {
        perror("sched_setattr");
        exit(-1);
    }

        printf("thread th3 start %ld\n", gettid());
        task_0();
        printf("thread th3 end %ld\n", gettid());
    return NULL;
}

void *th4 (void *data)
{
    struct sched_attr attr;
    int ret;
    unsigned int flags = 0;
    attr.sched_priority = 80;
    attr.sched_policy = SCHED_RR;
    ret = sched_setattr(0, &attr, flags);

    if (ret < 0) {
        perror("sched_setattr");
        exit(-1);
    }

        printf("thread th4 start %ld\n", gettid());
        task_1();
        printf("thread th4 end %ld\n", gettid());
    return NULL;
}

void *th5 (void *data)
{
    struct sched_attr attr;
    int ret;
    unsigned int flags = 0;
    attr.sched_priority = 70;
    attr.sched_policy = SCHED_RR;
    ret = sched_setattr(0, &attr, flags);

    if (ret < 0) {
        perror("sched_setattr");
        exit(-1);
    }

        printf("thread th5 start %ld\n", gettid());
        task_2();
        printf("thread th5 end %ld\n", gettid());
    return NULL;
}

void set_cpu(pthread_t t, cpu_set_t cpu, int cpuindex)
{
    int x;

    CPU_ZERO(&cpu);
    CPU_SET(cpuindex, &cpu);
    pthread_setaffinity_np(t, sizeof(cpu_set_t), &cpu);
}

int main (int argc, char **argv)
{
    pthread_t t[8];
    cpu_set_t c[8];
    int x;

    printf("main thread [%ld]\n", gettid());

    pthread_create(&t[0], NULL, th0, NULL);
    //set_cpu(t[0], c[0], 0);
    printf("t0\n");

    pthread_create(&t[1], NULL, th1, NULL);
    //set_cpu(t[1], c[1], 0);
    printf("t1\n");

    pthread_create(&t[2], NULL, th2, NULL);
    //set_cpu(t[2], c[2], 0);
    printf("t2\n");

    //printf("releasing the condition\n");
    //pthread_cond_broadcast(&cond);

    sleep(3);
    pthread_create(&t[3], NULL, th3, NULL);
    //set_cpu(t[3], c[3], 0);
    printf("t3\n");

    sleep(1);
    pthread_create(&t[4], NULL, th4, NULL);
    //set_cpu(t[4], c[4], 0);
    printf("t4\n");

    sleep(1);
    pthread_create(&t[5], NULL, th5, NULL);
    //set_cpu(t[5], c[5], 0);
    printf("t5\n");

    sleep(20);
    pthread_join(t[0], NULL);    printf("j0\n");
    pthread_join(t[1], NULL);    printf("j1\n");
    pthread_join(t[2], NULL);    printf("j2\n");
    pthread_join(t[3], NULL);    printf("j3\n");
    pthread_join(t[4], NULL);    printf("j4\n");
    pthread_join(t[5], NULL);    printf("j5\n");

    printf("main dies [%ld]\n", gettid());
    return 0;
}
