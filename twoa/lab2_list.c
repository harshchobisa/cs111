#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <getopt.h>
#include <signal.h>
#include "SortedList.h"

SortedListElement_t *listNodes;
SortedList_t *headptr;

long long threadsNum = 1;
long long iterationsNum = 1;

char sync = '0';


pthread_mutex_t mutexLock;
int spin = 0;
int opt_yield = 0;

void handle_sig() 
{ 
    fprintf(stderr, "Segmentation fault found\n");
    exit(2);
} 


void *thread_function_for_list(void *position)
{
  	int offsetfac = *((int *)position);
	int x = offsetfac;
    int limit = offsetfac + iterationsNum;
    for (; x < limit; x++)
	{
		switch (sync)
		{
            case '0':
                SortedList_insert(headptr, &listNodes[x]);
                break;
            case 'm':
                pthread_mutex_lock(&mutexLock);
                SortedList_insert(headptr, &listNodes[x]);
                pthread_mutex_unlock(&mutexLock);
                break;
            case 's':
                while (__sync_lock_test_and_set(&spin, 1));
                SortedList_insert(headptr, &listNodes[x]);
                __sync_lock_release(&spin);
                break;
		}
	}

	int len = 0;
	switch (sync)
	{
        case '0':
            len = SortedList_length(headptr);
            if (len < 0)
            {
                fprintf(stderr, "Error, list length can never be negative");
            }
            break;
        case 'm':
            pthread_mutex_lock(&mutexLock);
            len = SortedList_length(headptr);
            if (len < 0)
            {
                fprintf(stderr, "Error, list length can never be negative");
            }
            pthread_mutex_unlock(&mutexLock);
            break;
        case 's':
            while (__sync_lock_test_and_set(&spin, 1));
            len = SortedList_length(headptr);
            if (len < 0)
            {
                fprintf(stderr, "Error, list length can never be negative");
            }
            __sync_lock_release(&spin);
            break;

	}

    for (x = offsetfac; x < limit; x++)
	{
		switch (sync)
		{
            case '0':
                if (SortedList_delete(SortedList_lookup(headptr, listNodes[x].key)))
                {
                    fprintf(stderr, "Error: could not delete\n");
                    exit(2);
                }
                break;
            case 'm':
                pthread_mutex_lock(&mutexLock);
                if (SortedList_delete(SortedList_lookup(headptr, listNodes[x].key)))
                {
                    fprintf(stderr, "Error: could not delete\n");
                    exit(2);
                }
                pthread_mutex_unlock(&mutexLock);
                break;
            case 's':
                while (__sync_lock_test_and_set(&spin, 1));
                if (SortedList_delete(SortedList_lookup(headptr, listNodes[x].key)))
                {
                    fprintf(stderr, "Error: could not delete\n");
                    exit(2);
                }
                __sync_lock_release(&spin);
                break;
		}
	}

  return NULL;
}
struct option getOptStruct[] =
{
    {"threadsNum", 1, 0, 1},
    {"iterationsNum", 1, 0, 2},
    {"yield", 1, 0, 3},
    {"sync", 1, 0, 4},
    {0, 0, 0, 0}
};

int main(int argc, char **argv)
{
    int ins = 0;
    int del = 0;
    int look = 0;

    int returnValue = getopt_long(argc, argv, "", getOptStruct, NULL); 
    while (EXIT_FAILURE)
    {
        if (returnValue < 0)
        {
            break;
        }
        if (returnValue == 1) //threadsNumNum
        {
            threadsNum = atoi(optarg);

        }
        else if (returnValue == 2) //iterationsNumNum arg
        {
            iterationsNum = atoi(optarg);
        }
        else if (returnValue == 3) //yield arg
        {
            int len = (int) strlen(optarg);

            int x = 0;
            while (x < len)
            {
                switch (optarg[x])
                {
                    case 'l':
                        look = 1;
                        opt_yield = opt_yield | LOOKUP_YIELD;
                        break;

                    case 'd':
                        del = 1;
                        opt_yield = opt_yield | DELETE_YIELD;
                        break;

                    case 'i':
                        ins = 1;
                        opt_yield = opt_yield | INSERT_YIELD;
                        break;
                }

                x++;
            }
                        
        }
        else if (returnValue == 4) //sync arg
        {
            sync = *optarg;
        }
        else
        {
            fprintf(stderr, "Invalid inputs formatting. Arguments should be formatted as such: ./lab2_add --threadsNum=1 \n");
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

    signal(SIGSEGV, handle_sig);


    if (sync == 'm')
    {
        int check = pthread_mutex_init(&mutexLock, NULL);
        if (check != 0)
        {
            fprintf(stderr, "Error initializing Mutex Lock.\n");
            exit(EXIT_FAILURE);
        }
    }

	headptr = malloc(sizeof(SortedList_t));
	if (headptr == NULL) {
		fprintf(stderr, "Error: Unable to malloc\n");
		exit(EXIT_FAILURE);
	}

	int nodesNum = threadsNum * iterationsNum;
	listNodes = malloc(nodesNum * sizeof(SortedListElement_t));
	if (listNodes == NULL) {
		fprintf(stderr, "Error: Unable to malloc\n");
		exit(EXIT_FAILURE);
	}

    headptr->next = headptr;
	headptr->prev = headptr;
    headptr->key = NULL;

    int x = 0;
    for (x = 0; x < nodesNum; x++)
    {
        char* tempKey = malloc(2);
        if (tempKey)
        {
            tempKey[1] = '\0';
            tempKey[0] = rand();
            listNodes[x].key = tempKey;

        }
        else
        {
            fprintf(stderr, "Error allocating memory for key.\n");
            exit(EXIT_FAILURE);
        }
    }

	struct timespec timer;
	clock_gettime(CLOCK_MONOTONIC, &timer);

	pthread_t *threads = malloc(threadsNum * sizeof(pthread_t));
    int *thread_offset = malloc(4 * threadsNum);

    if (threads == NULL)
    {
        fprintf(stderr, "Error: Unable to malloc\n");
		exit(EXIT_FAILURE);
    }
    if (thread_offset == NULL)
    {
        fprintf(stderr, "Error: Unable to malloc\n");
		exit(EXIT_FAILURE);
    }

	x = 0;
    for (x = 0; x < threadsNum; x++)
    {
        thread_offset[x] = x * iterationsNum;
    }
    for (x = 0; x < threadsNum; x++)
	{
		int check = pthread_create(&threads[x], NULL, thread_function_for_list, &thread_offset[x]);
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
            fprintf(stderr, "Error when joining threads.\n");
            exit(EXIT_FAILURE);
        }

	}

	struct timespec stopTimer;
	clock_gettime(CLOCK_MONOTONIC, &stopTimer);

    free(threads);
	free(thread_offset);
	free(listNodes);

    if (sync == 'm')
    {
        int check = pthread_mutex_destroy(&mutexLock);
        if (check != 0)
        {
            fprintf(stderr, "Error destroying mutex lock.\n");
            exit(EXIT_FAILURE);
        }
    }
    
    int len = SortedList_length(headptr);
    if (len != 0)
    {
        fprintf(stderr, "Error, the list length is not zero. The length is %d", len);
        exit(2);
    }

    free(headptr);

    
    long long numOperations = threadsNum * iterationsNum * 3;
	long long runTime = (stopTimer.tv_sec - timer.tv_sec) * 1000000000 + (stopTimer.tv_nsec - timer.tv_nsec);
	long long averageOperationRuntime = runTime / numOperations;

    char * yieldOutput;

    yieldOutput = "";

	if (ins)
	{
		yieldOutput = "i";
	}
	if (del)
	{
		yieldOutput = "d";
	}
	if (look)
	{
		yieldOutput = "l";
	}
    if (ins && del)
    {
        yieldOutput = "id";
    }
    if (ins && look)
    {
        yieldOutput = "il";
    }
    if (look && del)
    {
        yieldOutput = "dl";
    }
    if (look && del && ins)
    {
        yieldOutput = "idl";
    }
	if (!ins && !del && !look)
	{
		yieldOutput =  "none";
	}

    char * syncOutput;
    syncOutput = "";

    if (sync == 'm')
    {
        syncOutput = "m";
    }
    else if (sync == 's')
    {
        syncOutput = "s";
    }
    else if (sync == '0')
    {
        syncOutput = "none";

    }

	fprintf(stdout, "list-%s-%s,%lld,%lld,%d,%lld,%lld,%lld\n", yieldOutput, syncOutput, threadsNum, iterationsNum, 1, numOperations, runTime, averageOperationRuntime);

	exit(0);
}