#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>


#define NB_ENABLE 0
#define NB_DISABLE 1

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

int kbhit(void)
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

int stopped = 0;
int called = 0;
int answered = 0;

void usr1Handler(int num){
  while(answered < called) {
    ++answered;
    printf("usr1!!!\n");
    usleep(1500*1000);
    stopped = 0;
  }
}

pthread_t a = 0;


pid_t gettid(){
  return syscall(SYS_gettid);
}

void *thb(void *_){
  while(1){
    usleep(500*1000);
    printf("---- %d %d\n", called, answered);
    //stopped = 1;
    if (called < 11) {
      ++called;
      pthread_kill(a, SIGUSR1);
    }
    //while(stopped){};
  }
  //printf("----: %d", a);
}

void *tha(void *_){
  a = pthread_self();
  while(1){
    printf("tha --- main\n");
    usleep(500*1000);
  }
}


void launchThreads(void){
  nonblock(NB_ENABLE);
  signal(SIGUSR1, usr1Handler);
  pthread_t tid;
  pthread_create(&tid, NULL, tha, NULL);
  pthread_create(&tid, NULL, thb, NULL);
  pthread_exit(NULL);
  return;
}


void main (void){
  launchThreads();
}



