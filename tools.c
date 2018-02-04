#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "asim.h"


// void strcpyUpTo(char *dest, char *src, int sup){
//   int len = strlen(src);
//   int i;
//   for (i = 0; i < sup - 1; i++) {
//     if (i >= len) {
//       dest[i] = '\0';
//     }
//     else {
//       dest[i] = src[i];
//     }
//   }

//   dest[sup - 1] = '\0';
// }


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
  int i; //, diviser;
  // for (i = size - 1; i > 0; i--){
  //   if (size % i == 0) {
  //     diviser = i;
  //     break;
  //   }
  // }
  for (i = 0; i < size - 1; i++){
    printf("%d ", xs[i]);
    // if ((i % diviser) == (diviser - 1)) {
    //   printf("_ ");
    // }
  }
  printf("%d", xs[size - 1]);
}

int countLines(char *str){
  int i;
  int out = 1;
  for (i = 0; str[i] != '\0'; i++) {
    if (str[i] == '\n') ++out;
  }
  return out;
}

void shiftList(int *xs, int size, int val){
  int i;
  for (i = 1; i < size; i++){
    xs[size - i] = xs[size - i - 1];
  }
  xs[0] = val;
}