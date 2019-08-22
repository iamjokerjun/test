#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

void fun(){
	int i;
	for(i=0; i < 100;i++){
	}
	//打开sleep
	sleep(3);
}

//测量钟表时间(秒为单位)
void testTime(){
	time_t start ,end;
	start = time(NULL);
	fun();
	end = time(NULL);
	printf("fun cost %ld seconds measure by time function\n",end - start);
}

//测量钟表时间(微秒为单位)
void testGetTimeofday(){
	struct timeval start,end;
	gettimeofday(&start,NULL);
	fun();
	gettimeofday(&end,NULL);
	long seconds = (end.tv_sec - start.tv_sec);
	long micros = seconds*1000000 + end.tv_usec - start.tv_usec;
	printf("fun cost %ld micros measure by getTimeofday function\n",micros);
}

//测量cpu时间
void testClock() {
	double time_spend = 0.0;
	clock_t start,end;
	start = clock();	//获取开始滴答数
	fun();
	end = clock();		//获取开始滴答数
	time_spend = (double)(end - start) / CLOCKS_PER_SEC;
	printf("fun cost %lf seconds measure by clock function\n",time_spend);
}

void testClockGetTime() {
	struct timespec start,end;
	//clockid_t id = CLOCK_REALTIME; 			//测量钟表时间
	clockid_t id = CLOCK_PROCESS_CPUTIME_ID;	//测量cpu时间 	类似clock函数了
	//clockid_t id = CLOCK_MONOTONIC; 			//测量钟表时间  不受系统时间修改的影响
	clock_gettime(id,&start);
	fun();
	clock_gettime(id,&end);
	double time_spend = (end.tv_sec - start.tv_sec)*1000000000 +  (end.tv_nsec - start.tv_nsec);
	printf("fun cost %lf nanosecond measure by clock_gettime function\n",time_spend);

}



int main(){
	testTime();
	testGetTimeofday();
	testClock();
	testClockGetTime();
	return 0;
}
