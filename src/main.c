#include <signal.h> // signal, SIGUSR1
#include <pthread.h>
#include <stdio.h>
#include "asim.h"


pthread_t threadView;
pthread_t threadUserCode;

pthread_t threadListener;
  //| ^ for now, this latter value isn't used
  //| but it will probably be used soon

void main(void){
  _noReturnNeededKBEvent(1);
    //| ^ useful for threadListener
    //| cf events.c

  init();
    //| ^ user-written function
    //| initializes global variable `ardu`,
    //| creates the virtual objects
    //| cf user.c

  signal(SIGUSR1, _interruptSignalHandler);
    //| ^ used to simulate the interruptions
    //| cf events.c

  _launchThreads();
}

void _launchThreads(void){
  pthread_create(&threadView, NULL, _threadView, NULL);
    //| ^ cf view.c
  pthread_create(&threadUserCode, NULL, _threadUserCode, NULL);
    //| ^ cf user.c
  pthread_create(&threadListener, NULL, _threadListener, NULL);
    //| ^ cf events.c
  pthread_exit(NULL);
}


#include <termios.h>
#include <unistd.h> // STDIN_FILENO

//| called from main()
//| meant to allow the user to send characters
//| to threadListener from the keyboard
//| without the need to hit <enter> for fgetc()
//| to take them into account.
//| shamelessly copy-pasted from the internet.
void _noReturnNeededKBEvent(Bool yes){
    struct termios ttystate;
    //get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);
 
    if (yes) {
        //turn off canonical mode
        ttystate.c_lflag &= ~ICANON;
        //minimum of number input read.
        ttystate.c_cc[VMIN] = 1;
    }
    else {
        //turn on canonical mode
        ttystate.c_lflag |= ICANON;
    }
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}








