#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <stdio.h>
#include <sched.h>
#include <thread>
#include <sys/time.h> 

//////////////自旋锁开始//////////////
//test and swap SpinLock
struct tas_SpinLock{
    volatile int Lock = 0;
    void lock(){
        while(__sync_lock_test_and_set(&Lock,1));
    }
    void unlock(){
        __sync_lock_release(&Lock);
    }
};

//compare and swap SpinLock
struct cas_SpinLock{
    volatile int Lock = 0;
    void lock(){
        while(__sync_bool_compare_and_swap(&Lock,0,1) == 1);
    }
    void unlock(){
       Lock = 0;
    }
};

//tick tick_SpinLock
struct tick_SpinLock {
    volatile int ticket = 0;
    volatile int turn = 0;
    void lock(){
        int myturn = __sync_fetch_and_add(&ticket,1);
        while (turn != myturn);
    }
    void unlock(){
        turn++; 
    }
};


//几种不同方式实现的自旋锁的测试
//struct tas_SpinLock myLock;
//struct cas_SpinLock myLock;
struct tick_SpinLock myLock;


//共享内存
volatile int data = 0;

//设置最大轮询次数
#define MAX_LOOP 0x1000000
#define THREAD_NUM 4

//屏障
pthread_barrier_t b;
//工作线程
void* worker(void*arg){
    pthread_barrier_wait(&b);
    while(1){
        myLock.lock();
        data++;
        if(data > MAX_LOOP) 
            break;
        myLock.unlock();
    }
    myLock.unlock();
    return NULL;
}


int main(){
    struct timeval start, end;
    pthread_barrier_init(&b,NULL,THREAD_NUM);        //初始化屏障
    pthread_t* pthreads = (pthread_t*)calloc(THREAD_NUM,sizeof(pthread_t));
    for(int i=0;i<THREAD_NUM;i++){
        if(pthread_create(pthreads+i,NULL,worker,NULL) <0){
            fprintf(stderr,"pthread_create = %d\n",errno);
        }
    }
    gettimeofday(&start, NULL);             //开始测试时间
    //是否绑定到固定cpu核心
    /*
    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    CPU_SET(0, &cpus);
    pthread_setaffinity_np(pid1, sizeof(cpu_set_t), &cpus);
    pthread_setaffinity_np(pid2, sizeof(cpu_set_t), &cpus);
    */
    for(int i=0;i<THREAD_NUM;i++){
        if(pthread_join(pthreads[i],NULL) < 0 ){
            fprintf(stderr,"pthread_join = %d\n",errno);
        }
    }
    free(pthreads);
    gettimeofday(&end, NULL); 
    //测试程序运行时间
    long long total_time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    printf("total time is %lld us\n", total_time);
    return 0;
}
