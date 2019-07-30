
#include <atomic>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <stdio.h>

sem_t Sema;
class Obj{
    public:
        Obj(char arg)
        {
            value = arg;
            fprintf(stdout,"Obj %c malloc\n",value);
        }
        ~Obj(){
            fprintf(stdout,"Obj %c free\n",value);
        }
    public:
        class Obj* next;
        char value;
};

struct Obj A('A');
std::atomic<struct Obj*> topPtr;
struct Obj* Pop(bool bSleep){
    for(;;){
        struct Obj* retPtr = topPtr;
        if(!retPtr)
            return NULL;
        struct Obj* nextPtr  = retPtr->next;
        //第一步 retPtr指向A nextPtr指向B
        if(bSleep)
        {
            sem_post(&Sema); //post信号量
            sleep(2);
            fprintf(stdout,"i am wake up topPtr.value[%c] retPtr.value[%c] nextPtr.value[%c]\n",((struct Obj*)topPtr)->value,retPtr->value,nextPtr->value);
        }
        if(topPtr.compare_exchange_weak(retPtr,nextPtr))
            return retPtr;
    }
}

void Push(struct Obj* objPtr){
    for(;;){
        struct Obj* nextPtr = topPtr;
        objPtr->next = nextPtr;
        if(topPtr.compare_exchange_weak(nextPtr,objPtr))
            return;
    }
}

void* worker1(void*arg){
    fprintf(stdout,"worker1 is work\n");
    Pop(true);
    fprintf(stdout,"worker1 is over\n");
    return NULL;
}


void* worker2(void*arg){
    sem_wait(&Sema); //等待信号量
    fprintf(stdout,"worker2 is wake\n");
    Pop(false); //top -> B -> C
    Pop(false); //top -> C
    Push(&A);   //top ->A -> C
    fprintf(stdout,"worker2 is over\n");
    return NULL;
}


int testFun()
{
    struct Obj B('B'),C('C');
    sem_init(&Sema, 0, 0);
    Push(&C);
    Push(&B);
    Push(&A);
    pthread_t pthread1,pthread2;
    if(pthread_create(&pthread1,NULL,worker1,NULL) < 0 ){
        fprintf(stderr,"pthread1 create = %d\n",errno);
        return -1;
    }
    if(pthread_create(&pthread2,NULL,worker2,NULL) < 0 ){
        fprintf(stderr,"pthread1 create = %d\n",errno);
        return -1;
    }
    pthread_join(pthread1,NULL);
    pthread_join(pthread2,NULL);
    return 1;
}

void printChain(){
    struct Obj* APtr = topPtr;
    while(APtr){
        printf("-->%c",APtr->value);
        APtr = APtr->next;
    }
    printf("\n");
}
int main(){
    testFun();
    char array[64] = {0,};      //栈上之前分配的B,C虽然已经释放掉了，但是栈上数据还存在。所以把栈数据清0，让其必定崩溃。
    struct Obj* APtr = topPtr;
    printChain();
    return 0;
}