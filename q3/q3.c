#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>
#include<time.h>
#include<sys/sem.h>

#define UNDEFINED 0
#define POOLRIDE 1
#define PREMIERRIDE 2

#define waitState 0
#define onRidePoolOne 1
#define onRidePoolFull 2
#define onRidePremier 4

sem_t pool_1cabsem;
sem_t poolfullcabsem;
sem_t premiercabsem;
sem_t paymentserversem;
sem_t waitcabsem;

long long int *paymentprogress;
long long int *driverstate;
long long int n;
long long int m;
long long int k;

long long int err[100];

pthread_mutex_t driverstatemutex;
pthread_mutex_t queuemutex;
pthread_t *riderthreads, *serverthreads;
pthread_mutex_t paymentmutex;
pthread_mutex_t progressmutex;



struct drivernode {
    long long int id;
    struct drivernode* next;
} drivernode;
struct drivernode* poolhead;
struct drivernode* pooltail;

struct serverstruct {
    long long int id;
} serverstruct;

struct paymentnode {
    long long int riderid;
    long long int cabType;
    struct paymentnode *next;
} paymentnode;
struct paymentnode* paymenthead;
struct paymentnode* paymenttail;

struct riderstruct {
    long long int id;
    long long int arrivalTime;
    long long int cabType;
    long long int maxWaitTime;
    long long int rideTime;
} riderstruct;



void* driverthread(void* arg);
void makePayment(struct riderstruct *arg);

void* bookCab(void* arg) {
    struct riderstruct *rider = (struct riderstruct*) arg;
    long long int rideTime = rider->rideTime;
    long long int maxWaitTime = rider->maxWaitTime;
    long long int cabType = rider->cabType;
    long long int arrivalTime = rider->arrivalTime;
    long long int riderid = rider->id;

    long long int sleep_time=arrivalTime;
    sleep(arrivalTime);
    if(cabType == 1) 
    {
        printf("Rider %lld has booked POOL ride and is waiting to get accepted by a driver\n", riderid);
        int checkerr1=pthread_mutex_lock(&queuemutex);
        if(!checkerr1)
        {
            err[1]=-1;
        }
        else
        {
            printf("error in passenger queue");
        }
        if(!poolhead ) 
        {
            int checkerr2=pthread_mutex_unlock(&queuemutex);
            struct timespec tm;
            if(!checkerr2)
            {
                err[2]=-1;
            }
            else
            {
                printf("error in unlocking passenger allocation to cab");
            }
            clock_gettime(CLOCK_REALTIME, &tm);
            tm.tv_sec += maxWaitTime;
            if(!(sem_timedwait(&waitcabsem,&tm)))
            {
                printf("Rider given ride before maxwait time");
            }
            else if(sem_timedwait(&waitcabsem, &tm) == -1) 
            {
                printf("Rider %lld cancelled his ride due to longer wait time\n", riderid);
                return NULL;
            }

            int checkerr3=sem_post(&pool_1cabsem);
            if(!checkerr3)
            {
                printf("cabs available now");
                err[3]=-1;
            }
            else
            {
                printf("error in increasing back count of pool1");
            }

            long long int driverforthis;
            struct drivernode *driver = (struct drivernode*) malloc (sizeof(struct drivernode));

            for(long long int i=1;i<n+1;i+=1)
            {
                int checkerr4=pthread_mutex_lock(&driverstatemutex);
                if(!checkerr4)
                {
                    err[4]=-1;
                }
                else
                {
                    printf("error in locking for cabtype\n");
                }
                if(!driverstate[i])
                {
                    driverforthis = i;
                    driver->id = i;
                    driverstate[i] = 1;
                    int checkerr5=pthread_mutex_unlock(&driverstatemutex);
                    if(!checkerr5)
                    {
                        err[5]=-1;
                    }
                    else
                    {
                        printf("error in unlocking for allocation in cab  tye\n");
                    }
                    break;
                }
                pthread_mutex_unlock(&driverstatemutex);
            }
            pthread_mutex_lock(&queuemutex);
            if(!poolhead )
            {
                pooltail = driver;
                poolhead = driver;
            }
            else
            {
                pooltail = driver;
                pooltail->next = driver;
            }
            pthread_mutex_unlock(&queuemutex);
            printf("Driver %lld picked up rider %lld. Ride in progress\n", driverforthis, riderid);
            printf("POOL ride confirmed for rider %lld\n", riderid);
            sleep(rideTime);
            printf("POOL ride completed for rider %lld\n", riderid);
            pthread_mutex_lock(&driverstatemutex);
            if(driverstate[driver->id] == 1) 
            {
                sem_wait(&pool_1cabsem);
                sem_post(&waitcabsem);
                driverstate[driverforthis] = 0;
            } 
            else 
            {
                sem_wait(&poolfullcabsem);
                sem_post(&pool_1cabsem);
                driverstate[driverforthis] = 1;
            }
            pthread_mutex_unlock(&driverstatemutex);
            pthread_mutex_lock(&queuemutex);
        } else {


            int checkerr6=pthread_mutex_unlock(&queuemutex);
            struct timespec tm;
            if(!checkerr6)
            {
                err[6]=-1;
            }
            else
            {
                printf("error in unlocking cab queue mutex");
            }
            clock_gettime(CLOCK_REALTIME, &tm);
            tm.tv_sec += maxWaitTime;
            if(!(sem_timedwait(&pool_1cabsem, &tm) +1)) {
                printf("Rider %lld cancelled his ride due to longer wait time\n", riderid);
                return NULL;
            }
            sem_post(&poolfullcabsem);
            long long int driverforthis;


            for(long long int i=1;i<=n;i++) {
                pthread_mutex_lock(&driverstatemutex);
                if(driverstate[i] == 1) {

                    driverstate[i] = 2;

                    driverforthis = i;
                    int checkerr8=pthread_mutex_unlock(&driverstatemutex);
                    if(!checkerr8)
                    {
                        err[8]=-1;
                    }
                    else
                    {
                        printf("error found in unlocking for cab state during pool ride with pool1 available");
                    }
                    break;
                }
                int checkerr9=pthread_mutex_unlock(&driverstatemutex);
                if(!checkerr9)
                {
                    err[9]=-1;
                }
                else
                {
                    printf("error found in unlocking for cab state during pool ride with empty cabs");
                }
            }
            printf("POOL ride confirmed for rider %lld\n", riderid);
            printf("Driver %lld picked up rider %lld. Ride in progress\n", driverforthis, riderid);
            sleep(rideTime);
            printf("POOL ride completed for rider %lld\n", riderid);
            pthread_mutex_lock(&driverstatemutex);
            if(driverstate[driverforthis] == 1) {
                driverstate[driverforthis] = 0;
                sem_post(&waitcabsem);
                sem_wait(&pool_1cabsem);
            } else {
                driverstate[driverforthis] = 1;
                sem_post(&pool_1cabsem);
                sem_wait(&poolfullcabsem);
            }
            pthread_mutex_unlock(&driverstatemutex);
            pthread_mutex_lock(&queuemutex);
        }
        pthread_mutex_unlock(&queuemutex);
    } 
    else if(cabType == 2) 
    {
        printf("Rider %lld has booked PREMIER ride and is waiting to get accepted by a driver\n", riderid);
        struct timespec tm;
        clock_gettime(CLOCK_REALTIME, &tm);
        long long int wait;
        tm.tv_sec += maxWaitTime;
        if(!(sem_timedwait(&waitcabsem, &tm) +1))
        {
            wait=-1;
            printf("Rider %lld cancelled his ride due to longer wait time\n", riderid);
            return NULL;
        }
        sem_post(&premiercabsem);
        long long int driverforthis;



        for(long long int i=1;i<n+1;i+=1) {
            pthread_mutex_lock(&driverstatemutex);
            if(driverstate[i] == 0) {
                driverforthis = i;
                driverstate[i] = 4;

                int checkerr9=pthread_mutex_unlock(&driverstatemutex);
                if(!checkerr9)
                {
                    err[9]=-1;
                }
                else
                {
                    printf("error in unlocking in cab type for premier cabs1 ");
                }
                break;
            }
            int checkerr10=pthread_mutex_unlock(&driverstatemutex);
            if(!checkerr10)
            {
                err[10]=-1;
            }
            else
            {
                printf("error in unlocking in cab type for premier cabs2");
            }
        }
        printf("PREMIER ride confirmed for rider %lld.\n", riderid);
        printf("Driver %lld picked up rider %lld. Ride in progress\n", driverforthis, riderid);
        sleep(rideTime);
        int checker=sem_wait(&premiercabsem);
        if(!checker)
        {
            printf("cabs under usage\n");
        }
        else
        {
            printf("cabs count having problem in sem_wait");
        }
        int checker1=sem_post(&waitcabsem);
        if(!checker)
        {
            printf("cabs made available\n");
        }
        else
        {
            printf("cabs count not incresing in sem_post for empty cabs");
        }
        printf("PREMIER ride completed for rider %lld\n", riderid);
    }
    makePayment(rider);
    pthread_exit(NULL);
}

void makePayment(struct riderstruct *arg) {
    struct paymentnode *node = (struct paymentnode*) malloc (sizeof(struct paymentnode));



    node->next = NULL;
    node->cabType = arg->cabType;
    node->riderid = arg->id;
    pthread_mutex_lock(&paymentmutex);
    if(!paymenthead ) {
        paymenttail = node;
        paymenthead = node;
    } else {
        paymenttail = node;

        paymenttail->next = node;
    }
    int checkerr13=pthread_mutex_unlock(&paymentmutex);
    if(!checkerr13)
    {
        err[13]=-1;
    }
    else
    {
        printf("error in payment exiting from lock");
    }
    while(1) 
    {
        pthread_mutex_lock(&progressmutex);
        if(paymentprogress[arg->id] == 1) {
            int checkerr14=pthread_mutex_unlock(&progressmutex);
            if(!checkerr14)
            {
                err[14]=-1;
            }
            else
            {
                printf("error before exiting to join threads");
            }
            printf("Payment is successfully completed for rider %lld\n",node->riderid);
            break;
        }
        int checkerr15=pthread_mutex_unlock(&progressmutex);
        if(!checkerr15)
        {
            err[15]=-1;
        }
        else
        {
            printf("error before exiting to join threads2\n");
        }
    }
    return;
}

void* acceptPayment(void* arg) {
    while(1) {
        pthread_mutex_lock(&paymentmutex);
        if(!paymenthead)
        {
            pthread_mutex_unlock(&paymentmutex);
            continue;
        }
        struct paymentnode *node = paymenthead;
        if(paymenthead == paymenttail) 
        {
            paymenttail = NULL;
        }
        paymenthead = paymenthead->next;
        int checkerr10=pthread_mutex_unlock(&paymentmutex);
        if(!checkerr10)
        {
            err[10]=-1;
        }
        else
        {
            printf("error occured while payment is made in mutex\n");
        }
        printf("Payment in progress for rider %lld in payment server %lld\n", node->riderid, ((struct serverstruct*)arg)->id);
        sleep(2);
        int checkerr11=pthread_mutex_lock(&progressmutex);
        if(!checkerr11)
        {
            err[11]=-1;
        }
        else
        {
            printf("error occured while ride is being finished and lock is initiated\n");
        }
        paymentprogress[node->riderid] = 1;
        int checkerr12=pthread_mutex_unlock(&progressmutex);
        if(!checkerr12)
        {
            err[12]=-1;
        }
        else
        {
            printf("error in finishing progress\n");
        }
        int sleeptime=1;
        sleep(sleeptime);
    }
}
int main(void) 
{
    pthread_mutex_init(&driverstatemutex, NULL);
    printf("drivers, riders, payment servers\n");
    pthread_mutex_init(&queuemutex, NULL);
    scanf("%lld %lld %lld", &n, &m, &k);
    pthread_mutex_init(&paymentmutex, NULL);
    printf("Enter informaion of all rider , give Arrival time, prefernce pool(1) or premier(2), Max waittime before exit, Ride time of service\n");
    pthread_mutex_init(&progressmutex, NULL);
    printf("do not enter more than given arguments may lead to segmentation fault\n");
    long long int riderinfo[m][4];
    long long int r_no=0;
    while(r_no<m)
    {
        long long int temp1;
        long long int temp2;
        long long int temp3;
        long long int temp4;
        scanf("%lld %lld %lld %lld",&temp1,&temp2,&temp3,&temp4);
        riderinfo[r_no][0]=temp1;
        riderinfo[r_no][1]=temp2;
        riderinfo[r_no][2]=temp3;
        riderinfo[r_no][3]=temp4;
        r_no+=1;
    }
    sem_init(&waitcabsem, 0, n);
    serverthreads = (pthread_t*) malloc (sizeof(pthread_t)*k);
    sem_init(&paymentserversem, 0, k);
    driverstate = (long long int*) malloc (sizeof(long long int)*(n+1));

    sem_init(&premiercabsem, 0, 0);
    sem_init(&pool_1cabsem, 0, 0);
        paymentprogress = (long long int*) malloc (sizeof(long long int)*(m+1));
    sem_init(&poolfullcabsem, 0, 0);
    riderthreads = (pthread_t*) malloc (sizeof(pthread_t)*m);
    for(long long int i=0;i<k;i++) {
        struct serverstruct *server = (struct serverstruct*) malloc (sizeof(struct serverstruct));
        server->id = i+1;
        pthread_create(&serverthreads[i], NULL, acceptPayment, (void*)server);

    }
    for(long long int i=0;i<m;i++) {
        struct riderstruct *rider = (struct riderstruct*) malloc (sizeof(struct riderstruct));
        rider->cabType = riderinfo[i][1];
        rider->maxWaitTime = riderinfo[i][2];
        long long int flag=0;
        if(flag!=0)
        {
            struct serverstruct * srv =(struct serverstruct *) malloc(sizeof(struct serverstruct ));
        }
        rider->arrivalTime = riderinfo[i][0];
        flag=0;
        rider->rideTime = riderinfo[i][3];
            rider->id = i+1;
        pthread_create(&riderthreads[i], NULL, bookCab, (void*)rider);

    }
    for(long long int i=0;i<m;i++) {
        pthread_join(riderthreads[i], NULL);

    }
    printf("Simulation over\n");
    return 0;

}
