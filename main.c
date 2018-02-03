#include <unistd.h> // STDIN_FILENO
#include <signal.h> // signal, SIGUSR1
#include <pthread.h>
#include <stdio.h>
#include "asim.h"



void main(void){
  nonblock(NB_ENABLE);
  init();
    // ^ user-coded function
    // initializes `sim`,
    // creates the virtual objects

  signal(SIGUSR1, iEventHandler);
  _launchThreads();
}


void _launchThreads(void){
  pthread_t tid;
  pthread_create(&tid, NULL, _threadDisplay, NULL);
  pthread_create(&tid, NULL, threadLoop, NULL);
  pthread_create(&tid, NULL, threadListener, NULL);
  pthread_exit(NULL);
  return;
}

void *threadLoop(void *_) {
  sim.loopThread = pthread_self();
  setup();
  while (1) {
    loop();
    //usleep(1000 + DISPLAY_FREQ);
  }
}



void *threadListener(void *_){
  int i;
  char c;
  Button *button;
  //int pinIx;
  while (1) {
    //while(!kbhit());
    c = fgetc(stdin);
    
    for (i = 0; i < buttonCount; i++){
      button = &buttons[i];
      if (button->key == c) {
        //pinIx = button->pin - sim.pins;
        digitalChange(button->pin, SWITCH, 1);
        button->isPressed = 1 - button->isPressed;
      }
    }
  }
}




#include <termios.h>
#include <stdlib.h>

void nonblock(int state)
{
    struct termios ttystate;
 
    //get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);
 
    if (state==NB_ENABLE)
    {
        //turn off canonical mode
        ttystate.c_lflag &= ~ICANON;
        //minimum of number input read.
        ttystate.c_cc[VMIN] = 1;
    }
    else if (state==NB_DISABLE)
    {
        //turn on canonical mode
        ttystate.c_lflag |= ICANON;
    }
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
 
}

Bool kbhit(void)
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}
