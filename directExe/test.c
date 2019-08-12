 #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

//是否打开上下文测试 本地测试打开不打开测试之差为300ms左右  所以测量上下文切换的时间消耗在3微妙左右
#define TEST_CONTEXT_SWITCH 
#define NUM 100000 //300ms 300*1000微妙/100000 = 3微妙
int main(){
   

    int pipefd1[2];
    int pipefd2[2];
    char buf;
   
    double msTime;
    if(pipe(pipefd1)<0)
        fprintf(stderr,"pipe1 error\n");
    if(pipe(pipefd2)<0)
        fprintf(stderr,"pipe2 error\n");
    int pid1,pid2;
    if((pid1 = fork()) < 0 )
        fprintf(stderr,"fork1 error\n");
    if(pid1 == 0){
        int count = NUM; 
        struct timeval begin,end;
         cpu_set_t cpus;
        CPU_ZERO(&cpus);
        CPU_SET(0, &cpus);
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpus);
        printf("begin measure \n");
        gettimeofday(&begin,NULL);
        while(count--)
        {
            write(pipefd1[1],&buf,1);
            #ifdef TEST_CONTEXT_SWITCH
                read(pipefd2[0],&buf,1);
            #else
                read(pipefd1[0],&buf,1);
            #endif
        }
        gettimeofday(&end,NULL);
        msTime = (end.tv_sec - begin.tv_sec)*1000 + (end.tv_usec - begin.tv_usec)/1000;
        printf("time cost is %lf(ms)\n",msTime);
    }
    else{ 
        if((pid2 = fork()) < 0 )
            fprintf(stderr,"fork2 error\n");
        if(pid2 == 0){
            #ifdef TEST_CONTEXT_SWITCH
                int count = NUM;
                //child process 2
                cpu_set_t cpus;
                CPU_ZERO(&cpus);
                CPU_SET(0, &cpus);
                pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpus);
                while(count--)
                {
                    read(pipefd1[0],&buf,1);
                    write(pipefd2[1],&buf,1);
                }
            #endif
        }
        else
        {
            wait(NULL);
            wait(NULL);
            printf("wait is over\n");
        }
    }
    return 0;
}
/*
int main(){
    struct timeval begin,end;
    double msTime;
    int count = 10000000; //1000万 500ms = 500*1000*1000ns ==》500*1000*1000ns / 1000*10000 =  500*1000 / 1000 = 500ns

    gettimeofday(&begin,NULL);
    while(count--)
    {
        int a = getppid();
    }
    gettimeofday(&end,NULL);
    msTime = (end.tv_sec - begin.tv_sec)*1000 + (end.tv_usec - begin.tv_usec)/1000;
    printf("time cost is %lf(ms)\n",msTime);
    return 0;
}*/