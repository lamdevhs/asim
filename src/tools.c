#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "asim.h"

//| add a new element to the link-based queue
void pushInQueue(Link *link, Queue *queue){
  if (queue->in == NULL) queue->out = link;
  else queue->in->next = link;
  queue->in = link;
  ++queue->size;
}

//| used to simulate shift registers
void copyList(int *xs, int *into, int size){
  int i;
  for (i = 0; i < size; i++){
    into[i] = xs[i];
  }
}

//| used to print the internal or visible
//| state of a shift register
void printList(int *xs, int size){
  int i;
  for (i = 0; i < size - 1; i++){
    printf("%d ", xs[i]);
  }
  printf("%d", xs[size - 1]);
}

//| used to simulate shift registers
//| pushes a new int value to the start of the
//| array, shifting by one all other elements
//| except the last one, which gets lost in the process.
void shiftList(int *xs, int size, int val){
  int i;
  for (i = 1; i < size; i++){
    xs[size - i] = xs[size - i - 1];
  }
  xs[0] = val;
}

//| returns 1 + the amount of newlines in str
//| aka, returns the amount of lines needed
//| to display str.
//| cf view.c
int countLines(char *str){
  int i;
  int out = 1;
  for (i = 0; str[i] != '\0'; i++) {
    if (str[i] == '\n') ++out;
  }
  return out;
}