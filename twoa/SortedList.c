#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "SortedList.h"

//find specifications for function in the SortedList.h file


void SortedList_insert(SortedList_t *list, SortedListElement_t *element) 
{

  if (element != NULL && list != NULL)
  {
    SortedList_t* curr = list->next;

    if (opt_yield && INSERT_YIELD) 
    {
      sched_yield();
    }

    while (list != curr && strcmp(element->key, curr->key) != 0) 
    {
      curr = curr->next;
    }

    element->prev = curr->prev;
    element->next = curr;
    curr->prev->next = element;
    curr->prev = element;

  }
  else
  {
    return;
  }
  
}


int SortedList_delete( SortedListElement_t *element) 
{

  if (opt_yield && DELETE_YIELD) {
    sched_yield();
  }

  if (element == NULL)
  {
    return 1;
  }

  if (element->next->prev == NULL)
  {
    return 1;
  }

  if (element->prev->next == NULL)
  {
    return 1;
  }
  
  element->next->prev = element->prev;
  element->prev->next = element->next;
  return 0;
  
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) 
{
  if (key != NULL && list != NULL) 
  {
    SortedListElement_t *curr = list->next;
    
    while (list != curr) 
    {
      if (opt_yield && LOOKUP_YIELD) 
      {
        sched_yield();
      }
      if (strcmp(curr->key, key) == 0) 
      {
        return curr;
      }

      curr = curr->next;
    }

    return NULL;
  }
  else
  {
    return NULL;
  }
    
}


int SortedList_length(SortedList_t *list) 
{
  int count = 0;

  if (list != NULL)
  {
    SortedListElement_t *curr = list->next;
    while (list != curr) 
    {
        if (opt_yield && LOOKUP_YIELD) {
        sched_yield();
      }
      curr = curr->next;
      count++;
    }

    return count;
  }

  else
  {
    return -1;
  }
}