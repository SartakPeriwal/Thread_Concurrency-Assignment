#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <fcntl.h>
#include <pthread.h>
#include <inttypes.h>
#include <inttypes.h>


long long int N;
long long int M;
long long int K;
#define BIRYANIPERTABLE 1

long long int table_nu=0;
long long int count=1;
//int robots[M];
//int serving_tables[N];
//int students[K];
long long int total_students = 0;
long long int done = 0;

//pthread_t tidR[M]; 
long long int table_id = 0;
pthread_t tidT[100000];
pthread_t tidS[100];


pthread_mutex_t mutexForTable;
pthread_mutex_t mutexForStud; 


long long int top = 0;
long long int bottom = 0;
long long int queue[10000];
void push(long long int x)
{
	queue[top] = x;
    top+=1;
}

long long int pop()
{
    long long int out=queue[bottom];
    bottom+=1;
	return out;
}
long long int isempty()
{
    if(bottom==top)
    {
        return 1;
    }
    else
	return 0;
}


long long int top2 = 0;
long long int bottom2 = 0;
long long int queue_table[10000];
void pusht(long long int x)
{
	queue_table[top2] = x;
    top2+=1;
}

long long int popt()
{
    long long int out=queue_table[bottom2];
    bottom2+=1;
	return out;
}
long long int isemptytable()
{
    if(bottom2==top2)
    {
        return 1;
    }
    else
	return 0;
}


long long int top1 = 0, bottom1 = 0;
long long int queue_stud[10000];
void pushs(long long int x)
{
	queue_stud[top1] = x;
    top1+=1;
}

long long int pops()
{
    long long int out=queue_stud[bottom1];
    bottom1+=1;
	return out;
}
long long int isemptystud()
{
    if(bottom1==top1)
    {
        return 1;
    }
    else
	return 0;
}

typedef struct table{
	long long int p;
}table;

/* */struct studen{
}studen_args;

struct arg{
}biryani_arg;


void student_in_slot(long long int *slots);
void *ready_to_serve_table(void *table_arg);

void *biryani_ready(void *biryani_arg)
{
	while(1)
	{ 

		long long int w = 2  + rand()%4;
        sleep(w);
        printf("preparation time %lld\n", w);
		long long int r = 1 + rand()%10;
        printf("%lld vessels made\n", r);
		long long int p = 25 + rand()%26;
        int sleeptime=w;
        //long long int m_sec =1000000 * w;
        //clock_t start_t=clock();
        //while(clock()< start_t + m_sec);
		while(r)
		{
			if(!(done-1))
				return NULL;
			if(isemptytable()==0 && isemptystud()==0)
			{
			pthread_mutex_lock(&mutexForTable);

				table table_arg;
				table_arg.p = p;
                popt();
                printf("table number %lld with biryani\n",table_nu++);
                table_nu%=M;
				r-=BIRYANIPERTABLE;
				pthread_create(&tidT[table_id], NULL, ready_to_serve_table, &table_arg);

                table_id+=1;
                if(r==0)
                {
                    printf("all vessels empty made by robot %lld are empty\n",count );
                }
                
			pthread_mutex_unlock(&mutexForTable);
			
			}
		}
	}
	if(!(done -1))
		return NULL;
}
void *wait_for_slot(void *studen_args)
{
	pthread_mutex_lock(&mutexForStud);
    //if(r==0)
    //{
    //    printf("all vessels empty made by")
    //}
    printf("student %lld waiting to be allocated a slot\n",top1);
	pushs(1);
    printf("student given slot to sit %lld\n",top1);
	pthread_mutex_unlock(&mutexForStud);
	return NULL;
}


void *ready_to_serve_table(void *table_arg)
{
    if(done==1)
    {
        return NULL;
    }
    long long int stud=1;
	long long int minimum = 10;
    long long int students[K];
	if(!(total_students - K))
		return NULL;
	//printf("student %lld served \n",stud);
	table *args = (table *) table_arg;
	long long int p = args->p;
	long long int slots = 1 + rand()%minimum;
	while(p)
	{
		pthread_mutex_lock(&mutexForStud);
		if(isemptystud()==0)
        {

            if(slots >= 1)
		    {
                stud=pops();
			    students[stud] = 1;
			    total_students+=1;

			    if(!(total_students - K))
			    {	
			        printf("total_students served till now : %lld\n", total_students);
				    done =1; 
				    return NULL;
			    }
			    p-=1;
              /* */// slots-=1;
			    printf("total_students served tilll now : %lld\n", total_students);
		    }
        }
		pthread_mutex_unlock(&mutexForStud);
	}
	pthread_mutex_lock(&mutexForTable);
	pusht(1);
    printf("table ready to serve %lld",top2 );
	pthread_mutex_unlock(&mutexForTable);

	return NULL;
}


void student_in_slot(long long int *slots)
{
	total_students++;
	printf("total_students : %lld\n", total_students);
	if(total_students == K)
	{
		done =1; 
		return;
	}
}

int main(void)
{
    scanf("%lld %lld %lld",&M, &N,&K);
    long long int robots[M];
    long long int serving_tables[N];
    long long int students[K];
    pthread_t tidR[M]; 
	long long int checkerr_init_1=pthread_mutex_init(&mutexForStud, NULL);

    if(!checkerr_init_1)
    {
        long long int flag=1;
    }
    else
    {
        printf("error found in initializing mutex");
    }
	long long int checkerr_init_2=pthread_mutex_init(&mutexForTable, NULL);

    if(!checkerr_init_2)
    {
        long long int flag=1;
    }
    else
    {
        printf("error found in initializing mutex");
    }
	long long int  tt=0;
	for(long long int i=0;i<=N-1;i+=1)
    {
		pusht(1);
    }

	for(long long int i=0;i<=M-1;i++)
	{
		pthread_create(&tidR[i], NULL, biryani_ready, &biryani_arg);
	}
    
	while(1)
	{
		if(done == 1)
        {
            printf("Simulation over\n");
			return 0;
        }
		long long int lol;
		long long int d = 0 + rand()%100;

		if(d <50 && tt<=K)
		{
            if(tt!=K)
			printf("Student Came\n");

			tt+=1;
            if(tt<=K)
			printf("total tally = %lld\n", tt);
        /* */    pthread_create(&tidS[total_students],NULL,wait_for_slot,&studen_args);
            table_nu++;
            sleep(1);
		//	wait_for_slot(&studen_args);
			//delay(3);
		}		
	}
	printf("total students served till now = %lld", total_students);
	long long int checkerr_destroy_1=pthread_mutex_destroy(&mutexForStud);
    if(!checkerr_destroy_1)
    {
        long long int flag=1;
    }
    else
    {
        printf("error found in destroying mutex");
    }
	long long int checkerr_destroy_2=pthread_mutex_destroy(&mutexForTable);
    if(!checkerr_destroy_2)
    {
        long long int flag=1;
    }
    else
    {
        printf("error found in destroying mutex");
    }
	return 0;
}
