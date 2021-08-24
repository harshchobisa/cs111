// NAME: Hari Senthilkumar
// EMAIL: harikrishna.s2000@gmail.com
// ID: 005144135


// Import necessary libraries

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <poll.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <zlib.h>
#include <pthread.h>
#include <time.h>
#include "SortedList.h"

SortedList_t *head;
SortedListElement_t *elements;

//variables for parlalel num Threads and num Iterations
long long numberOfThreads = 1;
long long numberofIterations = 1;

//lock variables to keep track
int spinLock = 0;
char lockSig = 0;
pthread_mutex_t mutex;
int opt_yield = 0;
int insert_flag;
int lookup_flag;
int delete_flag;

char string_yield[7];
char string_arg[6];

void segFaultHandler(){
    fprintf(stderr, "Error: Segmentation Fault");
    exit(2);
}

void *startRoutine(void *array){    
    int beg = *((int *)array);
    int i;
    i = beg;
    while(i < beg + numberofIterations){
        switch(lockSig){
            case 'm':
                pthread_mutex_lock(&mutex);
                SortedList_insert(head, &elements[i]);
                pthread_mutex_unlock(&mutex);
                break;
            case 's':
                while(__sync_lock_test_and_set(&spinLock, 1));
                SortedList_insert(head, &elements[i]);
                __sync_lock_release(&spinLock);
                break;
            default:
                SortedList_insert(head, &elements[i]);
                break;
        }
        i++;
    }

    int len = 0;
    switch(lockSig){
        case 'm':
            pthread_mutex_lock(&mutex);
            len = SortedList_length(head);
            pthread_mutex_unlock(&mutex);
            break;
        case 's':
            while(__sync_lock_test_and_set(&spinLock, 1));
            len = SortedList_length(head);
            __sync_lock_release(&spinLock);
            break;
        default:
            len = SortedList_length(head);
            break;
    }
    if(len < 0){
        fprintf(stderr, "Error: length of list is negative. \n");
        exit(1);
    }

    i = beg;
	while (i < beg + numberofIterations){
		switch (lockSig){
		case 'm':
			pthread_mutex_lock(&mutex);
			if (SortedList_delete(SortedList_lookup(head, elements[i].key))){
				fprintf(stderr, "Error: could not delete\n");
			}
			pthread_mutex_unlock(&mutex);
			break;
		case 's':
			while (__sync_lock_test_and_set(&spinLock, 1)){
				continue;
			}
			if (SortedList_delete(SortedList_lookup(head, elements[i].key))){
				fprintf(stderr, "Error: could not delete\n");
			}
			__sync_lock_release(&spinLock);
			break;
		default:
			if (SortedList_delete(SortedList_lookup(head, elements[i].key))){
				fprintf(stderr, "Error: could not delete\n");
			}
			break;
		}
		i++;
	}
    return NULL;
}

void printWrapper(long long *passedTime, long *averageTime, long long *numberOfOperations){
    sprintf(string_yield, "-");
    if(insert_flag == 1){
        sprintf(string_yield + strlen(string_yield), "i");
    }
    if(delete_flag == 1){
        sprintf(string_yield + strlen(string_yield), "d");
    }
    if(lookup_flag == 1){
        sprintf(string_yield + strlen(string_yield), "l");
    }
    if(insert_flag == 0 && delete_flag == 0 && lookup_flag == 0){
        sprintf(string_yield + strlen(string_yield), "none");
    }

    if(lockSig == 'm'){
        sprintf(string_arg, "-m");
    }else if(lockSig == 's'){
        sprintf(string_arg, "-s");
    }else{
        sprintf(string_arg, "-none");
    }
    
    int listNum = 1;

    fprintf(stdout, "list%s%s,%lld,%lld,%d,%lld,%lld,%ld\n", string_yield, string_arg, numberOfThreads, numberofIterations, listNum, *(numberOfOperations), *(passedTime), *(averageTime));

}

int main(int argc,  char *argv[]){

    signal(SIGSEGV, segFaultHandler);

    int totalElements;

    // options to be parsed by get opt
    opterr = 0;
    struct option long_options[] = {
        {"threads", required_argument, NULL, 't'}, 
        {"iterations", required_argument, NULL, 'i'}, 
        {"yield", required_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
        {0,0,0,0}
    };

    char ch; 
    int ii;
    while((ch = getopt_long(argc, argv, "", long_options, 0)) != -1){
        switch(ch){
            case 't':
                numberOfThreads = atoi(optarg);
                break;
            case 'i':
                numberofIterations = atoi(optarg);
                break;
            case 'y':
                ii = 0;
                while(ii < (int)strlen(optarg)){
                    if(optarg[ii] == 'i'){
                        opt_yield |= INSERT_YIELD;
                        insert_flag = 1;
                    }else if(optarg[ii] == 'd'){
                        opt_yield |= DELETE_YIELD;
                        delete_flag = 1;
                    }else if(optarg[ii] == 'l'){
                        opt_yield |= LOOKUP_YIELD;
                        lookup_flag = 1;
                    }
                    ii++;
                }
                break;
            case 's':
                lockSig = *optarg;
                break;
            default:
                fprintf(stderr, "Error: Unrecognized input. \n");
                exit(1); 
        }
    }
    head = malloc(sizeof(SortedList_t));

    if(head == NULL){
        fprintf(stderr, "Error: Unable to allocate memory for head");
        exit(1);
    }

    head->key = NULL;
    head->next = head;
    head->prev = head;

    totalElements = numberOfThreads * numberofIterations;
    elements = malloc(sizeof(SortedListElement_t) * totalElements);

    if(elements == NULL){
        fprintf(stderr, "Error: Unable to allocate memory for elements");
        exit(1);
    }
    
    int a;
    a = 0;
    while(a < totalElements){
        char *key = malloc(2 * sizeof(char));

        if(key == NULL){
            fprintf(stderr, "Error: Unable to allocate memory for keys. \n");
            exit(1);
        }

        key[0] = rand() % 256 + 'A';
        key[1] = '\0';
        
        elements[a].key = key;
        a++;
    }

    struct timespec begTime, finishTime;
    clock_gettime(CLOCK_MONOTONIC, &begTime);

    pthread_t *allThreads = malloc(numberOfThreads * sizeof(pthread_t));
    int *thread_pos = malloc(numberOfThreads * sizeof(int));
    if(allThreads == NULL){
        fprintf(stderr, "Error: Unable to allocate memory for threads. \n");
        exit(1); 
    }
    if(thread_pos == NULL){
        fprintf(stderr, "Error: Unable to allocate memory for threads. \n");
        exit(1); 
    }
    int index = 0;
    while(index < numberOfThreads){
        thread_pos[index] = index * numberofIterations;
        int err = pthread_create(&allThreads[index], NULL, startRoutine, &thread_pos[index]);
        if(err != 0){
            fprintf(stderr, "Error: Unable to create thread. \n");
            exit(1);
        }
        index++;
    }


    int jj = 0;
    while(jj < numberOfThreads){
        if(pthread_join(allThreads[jj], NULL)){
            fprintf(stderr, "Error: Unable to join threads. \n");
            exit(1);
        }
        jj++;
    }
    clock_gettime(CLOCK_MONOTONIC, &finishTime);

    if(SortedList_length(head) != 0){
        fprintf(stderr, "Error: Length of list is not 0. \n");
        exit(2);
    }
    
    long long numberOfOperations = numberOfThreads * numberofIterations * 3;
    long long passedTime = 1000000000L * (finishTime.tv_sec - begTime.tv_sec) + finishTime.tv_nsec - begTime.tv_nsec;
    long averageTime = passedTime / numberOfOperations;

   
    
    printWrapper(&passedTime, &averageTime, &numberOfOperations);

    pthread_mutex_destroy(&mutex);
    free(allThreads);
    free(thread_pos);
    free(head);
    free(elements);

    exit(0);
}