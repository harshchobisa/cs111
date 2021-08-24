#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <getopt.h>
#include <signal.h>
#include "SortedList.h"

SortedListElement_t *listNodes;
SortedList_t *listHeads;
long long threadsNum = 1;
long long iterationsNum = 1;
long long listHeadsNum = 1;
long long *lockDelay;
char sync = '0';
pthread_mutex_t *mutexLock;
int *spinLock;
int opt_yield = 0;
long long billion = 1000000000;

unsigned int hashFunction(const char * key)
{
  char first = key[0];
  unsigned int hash = first % listHeadsNum;
  return hash;
}

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
  int lockIndex = offsetfac/iterationsNum;

  struct timespec startIns;
  struct timespec finalIns;
  unsigned int index = 0;
  long long total = 0;

  for (; x < limit; x++)
  {
    index = hashFunction(listNodes[x].key);
    switch (sync)
    {
    case '0':
      // fprintf(stderr, "test");
      SortedList_insert(&listHeads[index], &listNodes[x]);
      break;
    case 'm':
      clock_gettime(CLOCK_MONOTONIC, &startIns);
      pthread_mutex_lock(&mutexLock[index]);
      clock_gettime(CLOCK_MONOTONIC, &finalIns);
      SortedList_insert(&listHeads[index], &listNodes[x]);
      pthread_mutex_unlock(&mutexLock[index]);
      break;
    case 's':
      clock_gettime(CLOCK_MONOTONIC, &startIns);
      while (__sync_lock_test_and_set(&spinLock[index], 1));
      clock_gettime(CLOCK_MONOTONIC, &finalIns);
      SortedList_insert(&listHeads[index], &listNodes[x]);
      __sync_lock_release(&spinLock[index]);
      break;
    }
    if (sync != '0')
    {
      total += billion * (finalIns.tv_sec - startIns.tv_sec) + (finalIns.tv_nsec - startIns.tv_nsec);
    }
  }

  struct timespec startLen;
  struct timespec finalLen;
  switch (sync)
  {
  case '0':

    for (x = 0; x < listHeadsNum; x++)
    {
      int len = SortedList_length(&listHeads[x]);
      if (len < 0)
      {
        fprintf(stderr, "Error, the list length can never be negative\n");
        exit(2);
      }
    }
    break;
  case 'm':
    for (x = 0; x < listHeadsNum; x++)
    {
      clock_gettime(CLOCK_MONOTONIC, &startLen);
      pthread_mutex_lock(&mutexLock[x]);
      clock_gettime(CLOCK_MONOTONIC, &finalLen);
      int len = SortedList_length(&listHeads[x]);
      pthread_mutex_unlock(&mutexLock[x]);
      if (len < 0)
      {
        fprintf(stderr, "Error, the list length can never be negative\n");
        exit(2);
      }
    }
    break;
  case 's':
    for (x = 0; x < listHeadsNum; x++)
    {
      clock_gettime(CLOCK_MONOTONIC, &startLen);
      while (__sync_lock_test_and_set(&spinLock[x], 1));
      clock_gettime(CLOCK_MONOTONIC, &finalLen);
      int len = SortedList_length(&listHeads[x]);
      __sync_lock_release(&spinLock[x]);
      if (len < 0)
      {
        fprintf(stderr, "Error, the list length can never be negative\n");
        exit(2);
      }
    }
    break;
  }
  if (sync != '0')
  {
    total += billion * (finalLen.tv_sec - startLen.tv_sec) + (finalLen.tv_nsec - startLen.tv_nsec);
  }

  struct timespec startDel;
  struct timespec finalDel;

  for (x = offsetfac; x < limit; x++)
  {
    index = hashFunction(listNodes[x].key);
    switch (sync)
    {
    case '0':
      if (SortedList_delete(SortedList_lookup(&listHeads[index], listNodes[x].key)))
      {
        fprintf(stderr, "Error deleting element.\n");
        exit(2);
      }
      break;
    case 'm':
      clock_gettime(CLOCK_MONOTONIC, &startDel);
      pthread_mutex_lock(&mutexLock[index]);
      clock_gettime(CLOCK_MONOTONIC, &finalDel);
      if (SortedList_delete(SortedList_lookup(&listHeads[index], listNodes[x].key)))
      {
        fprintf(stderr, "Error deleting element.\n");
        exit(2);
      }
      pthread_mutex_unlock(&mutexLock[index]);
      break;
    case 's':
      clock_gettime(CLOCK_MONOTONIC, &startDel);
      while (__sync_lock_test_and_set(&spinLock[index], 1))
        ;
      clock_gettime(CLOCK_MONOTONIC, &finalDel);
      if (SortedList_delete(SortedList_lookup(&listHeads[index], listNodes[x].key)))
      {
        fprintf(stderr, "Error deleting element.\n");
        exit(2);
      }
      __sync_lock_release(&spinLock[index]);
      break;
    }
    if (sync != '0')
    {
      total += billion * (finalDel.tv_sec - startDel.tv_sec) + (finalDel.tv_nsec - startDel.tv_nsec);
    }
  }
  lockDelay[lockIndex] += total;
  return NULL;
}

struct option getOptStruct[] =
    {
        {"threadsNum", 1, 0, 1},
        {"iterationsNum", 1, 0, 2},
        {"yield", 1, 0, 3},
        {"sync", 1, 0, 4},
        {"lists", 1, 0, 5},
        {0, 0, 0, 0}};

int main(int argc, char **argv)
{

  int returnValue = getopt_long(argc, argv, "", getOptStruct, NULL);
  int ins = 0, del = 0, look = 0;
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
      int len = (int)strlen(optarg);

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
    else if (returnValue == 5) //listHeads arg
    {
      listHeadsNum = atoi(optarg);
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

  if (listHeadsNum < 1)
  {
    fprintf(stderr, "Number of listHeads must be positive \n");
    exit(EXIT_FAILURE);
  }

  signal(SIGSEGV, handle_sig);

  listHeads = malloc(sizeof(SortedList_t) * listHeadsNum);
  if (listHeads == NULL)
  {
    fprintf(stderr, "Error allocating memory for sublistHeads\n");
    exit(EXIT_FAILURE);
  }

  int x = 0;
  for (x = 0; x < listHeadsNum; x++)
  {
    listHeads[x].next = &listHeads[x];
    listHeads[x].prev = &listHeads[x];
  }


  int nodesNum = threadsNum * iterationsNum;
  listNodes = malloc(nodesNum * sizeof(SortedListElement_t));
  if (listNodes == NULL)
  {
    fprintf(stderr, "Error allocating memory\n");
    exit(EXIT_FAILURE);
  }
  
  for (x = 0; x < listHeadsNum; x++)
  {
    listHeads[x].key = NULL;
  }
  for (x = 0; x < nodesNum; x++)
  {
    char *tempKey = malloc(2);
    if (tempKey)
    {
      tempKey[1] = '\0';
      tempKey[0] = 1 + rand() % 120;
      listNodes[x].key = tempKey;
    }
    else
    {
      fprintf(stderr, "Error allocating memory for key.\n");
      exit(EXIT_FAILURE);
    }
  }

  if (sync == 'm')
  {
    mutexLock = malloc(listHeadsNum * sizeof(pthread_mutex_t));
    if (mutexLock == NULL)
    {
      fprintf(stderr, "Error allocating memory for mutex lock");
      exit(EXIT_FAILURE);
    }
    int x = 0;
    for (; x < listHeadsNum; x++)
    {
      int check = pthread_mutex_init(&mutexLock[x], NULL);
      if (check != 0)
      {
        fprintf(stderr, "Error initializing Mutex Lock.\n");
        exit(EXIT_FAILURE);
      }
    }
  }

  if (sync == 's')
  {
    spinLock = malloc(listHeadsNum * 4);
    if (spinLock == NULL)
    {
      fprintf(stderr, "Error allocating memory for spin lock");
      exit(EXIT_FAILURE);
    }
  }

  if (sync == 's')
  {
    int x = 0;
    for (; x < listHeadsNum; x++)
    {
      spinLock[x] = 0;
    }
  }

  struct timespec timer;
  clock_gettime(CLOCK_MONOTONIC, &timer);

  pthread_t *threads = malloc(threadsNum * sizeof(pthread_t));
  if (threads == NULL)
  {
    fprintf(stderr, "Error allocating memory for threads\n");
    exit(EXIT_FAILURE);
  }

  int *thread_offset = malloc(4 * threadsNum);
  if (thread_offset == NULL)
  {
    fprintf(stderr, "Error allocating memory for thread offsets\n");
    exit(EXIT_FAILURE);
  }


  lockDelay = malloc(threadsNum * sizeof(long long));
  if (lockDelay == NULL)
  {
    fprintf(stderr, "Error allocating memory for lock delay array\n");
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
  // for (x = 0; x < nodesNum; x++)
  // {
  //     free(listNodes[x].key);
  // }
  free(listNodes);

  if (sync == 'm')
  {
    int check = pthread_mutex_destroy(&mutexLock[0]);
    if (check != 0)
    {
      fprintf(stderr, "Error destroying mutex lock.\n");
      exit(EXIT_FAILURE);
    }
    free(mutexLock);
  }

  if (sync == 's')
  {
    free(spinLock);
  }

  for (x = 0; x < listHeadsNum; x++)
  {
    int len = SortedList_length(&listHeads[x]);
    if (len != 0)
    {
      fprintf(stderr, "Error, the list length is not zero. The length is %d\n", len);
      exit(2);
    }
  }

  free(listHeads);

  long long averageLockDelay = 0;
  long long numOperations = threadsNum * iterationsNum * 3;
  long long sum = 0;

  for (x = 0; x < threadsNum; x++)
  {
    sum += lockDelay[x];
  }

  averageLockDelay = sum / numOperations;

  long long runTime = billion * (stopTimer.tv_sec - timer.tv_sec) + (stopTimer.tv_nsec - timer.tv_nsec);
  long long averageOperationRuntime = runTime / numOperations;

  char *yieldOutput;

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
    yieldOutput = "none";
  }

  char *syncOutput;
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

  fprintf(stdout, "list-%s-%s,%lld,%lld,%lld,%lld,%lld,%lld,%lld\n", yieldOutput, syncOutput, threadsNum, iterationsNum, listHeadsNum, numOperations, runTime, averageOperationRuntime, averageLockDelay);

  exit(0);
}
