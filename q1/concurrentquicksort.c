#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <wait.h>
#include <limits.h>
#include <pthread.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>
#include <math.h>

int partition(int arr[],int start,int end)
{
    int i=start -1;
    int pivot=arr[end];
    for(int j=start;j<end;j+=1)
    {
        if(arr[j] <= pivot)
        {
            int temp=arr[++i];
            arr[i]=arr[j];
            arr[j]=temp;
        }
    }
    int temporary=arr[i+1];
    arr[i+1]=arr[end];
    arr[end]=temporary;
    return (i+1);

}


int partition_random(int arr[],int start,int end)
{
    srand(time(NULL));
    int random=start + rand() % (end-start);
    //printf("%d %d %d",start,end,random);
    int temp=arr[random];
    arr[random]=arr[end];
    arr[end]=temp;
    return partition(arr,start,end);
}



void quicksort_concurrent(int arr[],int start,int end)
{
    if(start>=end)
    {
        return ;
    }
    else if((end -start)-5 < 0)
    {
        long long int p,j;
        for(long long int i=start+1;i<=end;i+=1)
        {
            j=i-1;
            p=arr[i];
            for(j=i-1;j>=start && arr[j]>p;j-=1)
            {
                arr[j+1]=arr[j];
            }
            arr[j+1]=p;
        }
        return;
    }
    else
    {
        int pivot=partition_random(arr,start,end);
        pid_t pid1 =fork();
        if(!pid1)
        {
            quicksort_concurrent(arr,start,pivot-1);
            exit(1);
        }
        else if(pid1>=1)
        {
            pid_t pid2 = fork();
            if(!pid2)	
            {
                quicksort_concurrent(arr,pivot+1,end);
                exit(1);
            }
            else
            {
                int status;
                waitpid(pid1,&status,0);
                waitpid(pid2,&status,0);
            }
        }
    }
    return;
}
void quicksort(int arr[],int start,int end)
{
    if(start<end)
    {
        int pivot=partition_random(arr,start,end);
        quicksort(arr,start,pivot-1);
        quicksort(arr,pivot+1,end);
    }
}
struct argument
{
    int end;
    int start;
    int *structurearray;

};
void* quicksort_thread(void* arg)
{
    struct argument * strarr=(struct argument*) arg;
    int end=strarr->end;
    int start=strarr->start;
    int *arr =strarr->structurearray;
    if(start>=end)
    {
        return NULL;
    }
    else if((end -start)-5 < 0)
    {
        long long int p,j;
        for(long long int i=start+1;i<=end;i+=1)
        {
            j=i-1;
            p=arr[i];
            for(j=i-1;j>=start && arr[j]>p;j-=1)
            {
                arr[j+1]=arr[j];
            }
            arr[j+1]=p;
        }
        return NULL;
    }
    else
    {
        int pivot=partition_random(arr,start,end);
        pthread_t tid1;
        pthread_t tid2;
        struct argument * argument1 =(struct argument *)malloc(sizeof(struct argument));
        argument1->end=pivot-1;
        argument1->structurearray =arr;
        argument1->start=start;
        struct argument * argument2 =(struct argument *)malloc(sizeof(struct argument));
        argument2->end=end;
        argument2->structurearray =arr;
        argument2->start=pivot+1;
        pthread_create(&tid1,NULL,quicksort_thread,(void*)argument1);
        pthread_create(&tid2,NULL,quicksort_thread,(void*)argument2);
        pthread_join(tid1,NULL);
        pthread_join(tid2,NULL);


    }
}
int main()
{
    struct timespec ts;
    long long int totallength;
    scanf("%lld", &totallength);
    key_t key = IPC_PRIVATE;
    int shmid = shmget(key , totallength*4, 0644| IPC_CREAT);
    int *arr = (int*) shmat(shmid,(void*)0,0);
    for(long long int t=0;t<=totallength-1;t+=1)
    {
        scanf("%d",&arr[t]);
    }
    int len=totallength;


    pthread_t tid;
    struct argument * final_args = (struct argument *) malloc(sizeof(struct argument));
    final_args->end=len-1;
    final_args->start=0;
    final_args->structurearray = arr;
    //printf("end ===%d\n",final_args->end);
    printf("Running threaded_concurrent_quicksort for n = %lld\n", totallength);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double st = ts.tv_nsec/(1e9)+ts.tv_sec;
    
    //printf("%d %d %d",final_args->end,final_args->start,len);
    pthread_create(&tid, NULL, quicksort_thread, (void*)final_args);
    pthread_join(tid, NULL);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t2 = en-st;
    




    printf("Running concurrent_quicksort for n = %lld\n", totallength);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;

    quicksort_concurrent(arr,0,len-1);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t1 = en-st;

    int brr[len+1];
    for(int i=0;i<len;i++) brr[i] = arr[i];

    printf("Running normal_quicksort for n = %lld\n", totallength);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;

    quicksort(brr,0,len-1);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);

    long double t3 = en-st;

    printf("normal_quicksort ran:\n\t[ %Lf  ] times faster than concurrent_quicksort\n\t[ %Lf  ] times faster than threaded_concurrent_quicksort\n\n\n", t1/t3, t2/t3);
//        shmdt(arr);

    for(long long int i=0;i<len;i+=1)
    {
        printf("%d ",arr[i]);
    }
    printf("\n");

return 0;
}
