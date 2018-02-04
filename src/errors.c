#include <pthread.h>
#include <stdlib.h>
#include "asim.h"

//| types of error (non-exhaustive):
//| - internal/bugs: things that should never happen,
//|   but if it does, better have a clear message
//|   before it crashes
//| - user errors: misuse of the simulation, trying
//|   to do absurd things like using a pin that doesn't exist
//| - failures: limitations of the program, eg:
//|   creating more than 'BIGN' diodes, buttons, etc,
//|   is impossible due to (reasonable) limitations of the static
//|   declarations of the simulation variables. maybe in the future
//|   i'll use dynamic structures instead, although that doesn't seem
//|   really useful...


void _killThreadView(void){
  pthread_cancel(threadView);
}

void _fatalError(char *type, char *fn, char *message){
  _killThreadView();
  printf("\n\nFATAL %s ERROR: in %s(): %s\n", type, fn, message);
  printf("Aborting.\n");
  exit(1);
}