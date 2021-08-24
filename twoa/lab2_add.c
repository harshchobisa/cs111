#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <pthread.h>
#include <time.h>

long long threadsNum = 1;
long long iterationsNum = 1;
long long counter = 0;
long long opt_yield = 0;
char sync = '0';
pthread_mutex_t mutexLock;
int spinLockVal = 0;
char * printPrefix;



void add(long long *pointer, long long value) 
{
    long long sum = *pointer + value;
    if (opt_yield)
        sched_yield();
    *pointer = sum;
}


struct option getOptStruct[] =
{
    {"threads", 1, 0, 1},
    {"iterations", 1, 0, 2},
    {"yield", 0, 0, 3},
    {"sync", 1, 0, 4},
    {0, 0, 0, 0}
};

void* adding_for_thread(void* throwaway)
{
    long long x = 0;
    long long old;


    for (x = 0; x < iterationsNum; x++)
    {
        switch (sync)
        {
            case '0':

                if (opt_yield)
                    printPrefix = "add-yield-none";
                else
                    printPrefix = "add-none";

                add(&counter, 1);
                add(&counter, -1);
                break;

            case 'm':

                if (opt_yield)
                    printPrefix = "add-yield-m";
                else
                    printPrefix = "add-m";

                pthread_mutex_lock(&mutexLock);
                add(&counter, 1);
                pthread_mutex_unlock(&mutexLock);

                pthread_mutex_lock(&mutexLock);
                add(&counter, -1);
                pthread_mutex_unlock(&mutexLock);
                break;

            case 's':

                if (opt_yield)
                    printPrefix = "add-yield-s";
                else
                    printPrefix = "add-s";

                while (__sync_lock_test_and_set(&spinLockVal, 1));
                add(&counter, 1);
                __sync_lock_release(&spinLockVal);

                while (__sync_lock_test_and_set(&spinLockVal, 1));
                add(&counter, -1);
                __sync_lock_release(&spinLockVal);
                
                break;

            case 'c':
                if (opt_yield)
                    printPrefix = "add-yield-c";
                else
                    printPrefix = "add-c";

                do{
                    old = counter;
                    if(opt_yield)
                    {
                        sched_yield();
                    }
                } while(__sync_val_compare_and_swap(&counter, old, old + 1) != old);

                do{
                    old = counter;
                    if(opt_yield)
                    {
                        sched_yield();
                    }
                } while(__sync_val_compare_and_swap(&counter, old, old - 1) != old);

                break;
        }
    }

    return throwaway;

}

int main(int argc, char* argv[])
{
    int returnValue = getopt_long(argc, argv, "", getOptStruct, NULL); 

    //parses through command line arguments
    while (1)
    {
        if (returnValue < 0)
        {
            break;
        }
        if (returnValue == 1) //threadsNum
        {
            threadsNum = atoi(optarg);

        }
        else if (returnValue == 2) //iterationsNum arg
        {
            iterationsNum = atoi(optarg);
        }
        else if (returnValue == 3) //iterationsNum arg
        {
            opt_yield = 1;
        }
        else if (returnValue == 4) //iterationsNum arg
        {
            sync = *optarg;
        }
        else
        {
            fprintf(stderr, "Invalid inputs formatting. Arguments should be formatted as such: ./lab2_add --threads=1 \n");
            exit(EXIT_FAILURE);
        }
        returnValue = getopt_long(argc, argv, "", getOptStruct, NULL);
    }

    if (threadsNum < 1)
    {
        fprintf(stderr, "Number of threads must be positive \n");
        exit(EXIT_FAILURE);
    }

    if (iterationsNum < 1)
    {
        fprintf(stderr, "Number of iterations must be positive \n");
        exit(EXIT_FAILURE);
    }

    if (sync == 'm')
    {
        int check = pthread_mutex_init(&mutexLock, NULL);
        if (check != 0)
        {
            fprintf(stderr, "Error initializing Mutex Lock.\n");
            exit(EXIT_FAILURE);
        }
    }

    struct timespec timer;


    clock_gettime(CLOCK_MONOTONIC, &timer);

    pthread_t *threads = malloc(threadsNum * sizeof(pthread_t));

    int x = 0;
    for (x = 0; x < threadsNum; x++)
    {
        int check = pthread_create(&threads[x], NULL, &adding_for_thread, NULL);
        if (check != 0)
        {
            fprintf(stderr, "Error when attempting to create threads.\n");
            exit(EXIT_FAILURE);
        }
    }

    for (x = 0; x < threadsNum; x++)
    {
        int check = pthread_join(threads[x], NULL);
        if (check != 0)
        {
            fprintf(stderr, "Error when attempting to join threads.\n");
            exit(EXIT_FAILURE);
        }
    }
    if (sync == 'm')
    {
        pthread_mutex_destroy(&mutexLock);
    }

    struct timespec stopTimer;

    clock_gettime(CLOCK_MONOTONIC, &stopTimer);

    long long operationsNum = iterationsNum * threadsNum * 2;
    
    long double billion = 10 * 9;
    long long runtime = (stopTimer.tv_sec - timer.tv_sec) * billion + (stopTimer.tv_nsec - timer.tv_nsec);

    long long averageOperationRuntime = runtime/operationsNum;


    fprintf(stdout, "%s,%lld,%lld,%lld,%lld,%lld,%lld\n", printPrefix, threadsNum, iterationsNum, operationsNum, runtime, averageOperationRuntime, counter);

    free(threads);

    exit(0);
}