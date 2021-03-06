/*
 * Code for basic C skills diagnostic.
 * Developed for courses 15-213/18-213/15-513 by R. E. Bryant, 2017
 */

 /*
  * This program implements a queue supporting both FIFO and LIFO
  * operations.
  *
  * It uses a singly-linked list to represent the set of queue elements
  */

#include <stdlib.h>
#include <stdio.h>

#include "harness.h"
#include "queue.h"

  /*
    Create empty queue.
    Return NULL if could not allocate space.
  */
queue_t* q_new()
{
  /* Remember to handle the case if malloc returned NULL */
  queue_t* q;

  q = malloc(sizeof(queue_t));
  if (q == NULL) {
    return q;
  }
  q->head = NULL;
  q->tail = NULL;
  q->size = 0;
  return q;
}

/* Free all storage used by queue */
void q_free(queue_t* q)
{
  /* Remember to free the queue structue and list elements */
  if (q == NULL) {
    return;
  }
  if (q->head == NULL) {
    free(q);
    return;
  }

  list_ele_t* prev = q->head;
  list_ele_t* curr = prev;
  while (curr != NULL) {
    curr = curr->next;
    free(prev);
    prev = curr;
  }
  free(q);
}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
 */
bool q_insert_head(queue_t* q, int v)
{
  /* What should you do if the q is NULL? */
  /* What if malloc returned NULL? */
  if (q == NULL) {
    return false;
  }
  //1 or more elements
  list_ele_t* ele = malloc(sizeof(list_ele_t));
  if (ele == NULL) {
    return false;
  }
  ele->value = v;
  ele->next = NULL;
  if (q->head == NULL) {
    q->head = ele;
    q->tail = ele;
  }
  else {
    ele->next = q->head;
    q->head = ele;
  }
  q->size++;
  return true;
}

/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
 */
bool q_insert_tail(queue_t* q, int v)
{
  /* Remember: It should operate in O(1) time */
  if (q == NULL) {
    return false;
  }
  //1 or more elements
  list_ele_t* ele = malloc(sizeof(list_ele_t));
  if (ele == NULL) {
    return false;
  }
  ele->value = v;
  ele->next = NULL;
  if (q->head == NULL) {
    q->head = ele;
    q->tail = ele;
  }
  else {
    q->tail->next = ele;
    q->tail = ele;
  }
  q->size++;
  return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If vp non-NULL and element removed, store removed value at *vp.
  Any unused storage should be freed
*/
bool q_remove_head(queue_t* q, int* vp)
{
  if (q == NULL || q->head == NULL) {
    return false;
  }
  list_ele_t* rem = q->head;
  q->head = q->head->next;
  int i = rem->value;
  free(rem);
  if (vp != NULL) {
    *vp = i;
  }
  q->size--;
  return true;
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t* q)
{
  /* Remember: It should operate in O(1) time */
  if (q == NULL || q->head == NULL) {
    return 0;
  }
  return q->size;
}

/*
  Reverse elements in queue.

  Your implementation must not allocate or free any elements (e.g., by
  calling q_insert_head or q_remove_head).  Instead, it should modify
  the pointers in the existing data structure.
 */
void q_reverse(queue_t* q)
{
  if (q == NULL || q->head == NULL || q->head->next == NULL) {
    return;
  }
  list_ele_t* tmp;
  list_ele_t* prev = q->head;
  list_ele_t* curr = q->head->next;
  while (curr != NULL) {
    tmp = curr->next;
    curr->next = prev;
    prev = curr;
    curr = tmp;
  }
  q->head->next = NULL;
  tmp = q->head;
  q->head = q->tail;
  q->tail = tmp;
  return;
}
