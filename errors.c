#include <pthread.h>
#include <stdlib.h>
#include "asim.h"
/*
  there are different types of errors:
  - bugs: which should never happen. crash (program can't tell what to do)
  - failure: filled up all the static memory for virtual objects, for example.
      in a reasonable usage, it should never happen. crash or error report.
  - user error: wrong use of a pin, of a function. either crash, or error report.

  warnings;
  - doubtful user usage of some function (like digitalWrite on an input,
    which changes the pullup state)
  -

*/

pthread_t threadView;

void _killThreadView(void){
  pthread_cancel(threadView);
}

void _fatalError(char *type, char *fn, char *message){
  _killThreadView();
  printf("\n\nFATAL %s ERROR: in %s(): %s\n", type, fn, message);
  printf("Aborting.\n");
  exit(1);
}