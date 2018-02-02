#include <stdlib.h>
#include <string.h>
#include "asim.h"


void setDisplayName(char *dest, char *src){
  int len = strlen(src);
  int i;
  for (i = 0; i < SIZE_NAME - 1; i++) {
    if (i >= len) {
      dest[i] = '\0';
    }
    else {
      dest[i] = src[i];
    }
  }

  dest[SIZE_NAME - 1] = '\0';
}


void pushInQueue(Link *link, Queue *queue){
  if (queue->in == NULL) queue->out = link;
  else queue->in->next = link;
  queue->in = link;
  ++queue->size;
}




void copyList(int *xs, int *into, int size){
  int i;
  for (i = 0; i < size; i++){
    into[i] = xs[i];
  }
}

void printList(int *xs, int size){
  int i;
  for (i = 0; i < size - 1; i++){
    printf("%d ", xs[i]);
  }
  printf("%d", xs[size - 1]);
}

