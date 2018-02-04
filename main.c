#include <signal.h> // signal, SIGUSR1
#include <pthread.h>
#include <stdio.h>
#include "asim.h"

void main(void){

  _noReturnNeededKBEvent(1);
  init();
    // ^ user-coded function
    // initializes `sim`,
    // creates the virtual objects

  signal(SIGUSR1, _interruptSignalHandler);
  _launchThreads();
}


void _launchThreads(void){
  pthread_t tid;
  pthread_create(&tid, NULL, _threadView, NULL);
  pthread_create(&tid, NULL, _threadUserCode, NULL);
  pthread_create(&tid, NULL, _threadListener, NULL);
  pthread_exit(NULL);
  return;
}










