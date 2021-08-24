// NAME: Hari Senthilkumar
// EMAIL: harikrishna.s2000@gmail.com
// ID: 005144135


// Import necessary libraries

// #include <stdio.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <getopt.h>
// #include <stdlib.h>
// #include <errno.h>
// #include <signal.h>
// #include <sys/stat.h>
// #include <string.h>
// #include <sys/types.h>
// #include <poll.h>
// #include <sys/wait.h>
// #include <sys/types.h>
// #include <signal.h>
// #include <termios.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <netdb.h>
// #include <zlib.h>
// #include <pthread.h>
// #include <time.h>
// #include "SortedList.h"

// int opt_yield = 0;
// /*
//     SortedList_insert is a method that inserts an element into the sorted list
//     The passed in element will be inserted into the passed in list 
//     which will be sorted by ascending order based on the keys. 

//     param: SortedList_t *list points to the header of the list
//     param: SortedListElement_t *element which is the element added to the list 
// */

// void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
//     if(list == NULL || element == NULL){
//         return;
//     }

//     SortedListElement_t *temp = list->next;

//     while(temp != NULL && strcmp(element->key, temp->key) > 0){
//         temp = temp->next;
//     }

//     if(opt_yield & INSERT_YIELD){
//         sched_yield();
//     }

//     element->prev = temp->prev;
//     element->next = temp;
//     temp->prev->next = element;
//     temp->prev = element;
    

// }


// /*
//     SortedList_delete is a method that removes an element 
//     from the sorted list where the passed in element will
//     be removed from the list that its specifed in.

//     param: SortedListElement_t *element which is the element that is removed from the list 

//     return 0 if element was deleted and 1 if not
// */

// int SortedList_delete(SortedListElement_t *element){
//     if(element == NULL){
//         return 1;
//     }

//     if(element->next == NULL){
//         return 1;
//     }
//     if(element->prev == NULL){
//         return 1;
//     }
//     if(opt_yield & DELETE_YIELD){
//         sched_yield();
//     }

//     if(element->prev->next ==  element->next->prev && element->prev->next == element && element->next->prev == element){
//         element->prev->next = element->next;
//         element->next->prev = element->prev;
//         return 0;
//     }else{
//         return 1;
//     }
// }


// /*
//     SortedList_lookup is a method that searches through
//     a sorted list for a specified key that is passed in
//     as a paramenter.

//     param: SortedList_t *list is the header for the passed in list
//     param: const char *key is the key that needs to be looked up

//     return: return a pointer to the matching element if found
//     and NULL if not found
// */

// SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
//     if(list == NULL){
//         return NULL;
//     }
//     if(key == NULL){
//         return NULL;
//     }
//     SortedListElement_t *ptr = list->next;
    
//     while(ptr != list){
//         if(strcmp(key, ptr->key) == 0){
//             return ptr;
//         }

//         if(opt_yield & LOOKUP_YIELD){
//             sched_yield();
//         }
        
//         ptr = ptr->next;
//     }
//     return NULL;
// }



// /*
//     SortedList_length counts all the elements in a sorted list

//     param: SortedList_t *list is the header for the list

//     return number of elements in list and -1 if list is null/empty
// */


// int SortedList_length(SortedList_t *list){
//     if(list == NULL){
//         return -1;
//     }
//     int cnt = 0;

//     SortedListElement_t *ptr = list->next;

//     while(ptr != list){
//         if(opt_yield & LOOKUP_YIELD){
//             sched_yield();
//         }
//         ptr = ptr->next;
//         cnt++;
//     }
//     return cnt;
// }

#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "SortedList.h"

/**
 * SortedList_insert ... insert an element into a sorted list
 *
 *	The specified element will be inserted in to
 *	the specified list, which will be kept sorted
 *	in ascending order based on associated keys
 *
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {

  if (list == NULL || element == NULL) {
    return;
  }
  
  SortedList_t* temp = list->next;
  while (temp != list && strcmp(element->key, temp->key) > 0) {
    temp = temp->next;
  }

  if (opt_yield && INSERT_YIELD) {
    sched_yield();
  }


    element->prev = temp->prev;
    element->next = temp;
    temp->prev->next = element;
    temp->prev = element;
  
}
/**
 * SortedList_delete ... remove an element from a sorted list
 *
 *	The specified element will be removed from whatever
 *	list it is currently in.
 *
 *	Before doing the deletion, we check to make sure that
 *	next->prev and prev->next both point to this node
 *
 * @param SortedListElement_t *element ... element to be removed
 *
 * @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 *
 */
int SortedList_delete( SortedListElement_t *element) {

  if (opt_yield && DELETE_YIELD) {
    sched_yield();
  }
  
  if (element != NULL && element->prev->next != NULL && element->next->prev != NULL) {
    element->prev->next = element->next;
    element->next->prev = element->prev;
    return 0;
  }
  else {
    return 1;
  }
  
}

/**
 * SortedList_lookup ... search sorted list for a key
 *
 *	The specified list will be searched for an
 *	element with the specified key.
 *
 * @param SortedList_t *list ... header for the list
 * @param const char * key ... the desired key
 *
 * @return pointer to matching element, or NULL if none is found
 */
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {

  if (list == NULL || key == NULL) {
    return NULL;
  }

  SortedListElement_t *temp = list->next;
  
  while (temp != list) {
    if (strcmp(temp->key, key) == 0) {
      return temp;
    }
    if (opt_yield && LOOKUP_YIELD) {
      sched_yield();
    }
    temp = temp->next;
  }

  return NULL;
  
}

/**
 * SortedList_length ... count elements in a sorted list
 *	While enumeratign list, it checks all prev/next pointers
 *
 * @param SortedList_t *list ... header for the list
 *
 * @return int number of elements in list (excluding head)
 *	   -1 if the list is corrupted
 */
int SortedList_length(SortedList_t *list) {

  if (list == NULL) {
    return -1;
  }

  int counter = 0;
  SortedListElement_t *temp = list->next;
  while (temp != list) {
    if (opt_yield && LOOKUP_YIELD) {
      sched_yield();
    }
    counter++;
    temp = temp->next;
  }

  return counter;
  
}