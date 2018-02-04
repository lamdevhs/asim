#include <stdlib.h>
#include <signal.h> // signal, SIGUSR1
#include <termios.h>
#include <pthread.h>
#include <unistd.h> // STDIN_FILENO
#include "asim.h"

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

void *_threadListener(void *_){
  int i;
  char c;
  Button *button;
  while (1) {
    c = fgetc(stdin);
    
    for (i = 0; i < buttonCount; i++){
      button = &buttons[i];
      if (button->key == c) {
        _digitalChangeValue(button->pin, SWITCH, 1);
        button->isPressed = 1 - button->isPressed;
      }
    }
  }
}



// interruptions
void _digitalChangeValue(DigitalPin *pin, int newValue, int fromListener){
  int oldValue = pin->value;
  if (newValue == SWITCH) newValue = (oldValue == 0);
  
  pin->value = newValue;
  pin->isAnalog = 0;

  if (pin->onChange != NULL) {
    //printf("change!\n");
    pin->onChange(pin->onChangeArg, pin, oldValue);
  }

  if (!fromListener) return;
    // ^ for now we don't allow interruptions
    // triggered from userCodeThread

  if (pin->canInterrupt) {
    int changed = (newValue != oldValue);
    InterruptMode mode = pin->interrMode;

    if (mode == NO_INTERR) return;

    else if (mode == LOW || mode == HIGH){
      if (newValue == mode) addInterrupt(pin);
    }
    else if (!changed) return;

    else if (mode == RISING) {
      if (newValue == HIGH) addInterrupt(pin);
    }
    else if (mode == FALLING) {
      if (newValue == LOW) addInterrupt(pin);
    }
    else if (mode == CHANGE) addInterrupt(pin);

    else return; // BUG wrong value
    //pthread_kill(sim.userCodeThread, SIGUSR1);
  }
}



void addInterrupt(DigitalPin *pin){
  if (!pin->canInterrupt) return; // BUG!

  IEvent *ie = (IEvent *)malloc(sizeof(IEvent));
    // ^ this variable name already being cursed
    // i won't bother checking for null pointer
    // TODO
  Link *ieLink = (Link *)malloc(sizeof(Link));

  ie->pin = pin;
  ie->dead = 0;
  
  ieLink->content = (void *)ie;
  ieLink->next = NULL;
  pushInQueue(ieLink, &sim.ieq);
  pthread_kill(sim.userCodeThread, SIGUSR1);
}

void _interruptSignalHandler(int _){
  sim.interrupted = 1;
  while(1) {
    Link *ieLink = sim.ieq.out;
    IEvent *ie = (IEvent *)ieLink->content;
    // TODO: test the pin is the right kind
    // test out is not NULL, etc
    if (ie->dead) {
      // BUG?

    }
    else { // take care of it
      ie->dead = 1;
      ie->pin->interrFun();

      // we check that after interrFun in case that one
      // were meant to modify the mode of that pin
      InterruptMode mode = ie->pin->interrMode;
      if (mode == LOW || mode == HIGH)
        if (mode == ie->pin->value) {
          // TODO the case of analog is bleh
          ie->dead = 1; // don't delete the current event
      }
      else {
        --sim.ieq.size;
      }
    }
    
    
    if (ie->dead) {
      if (ieLink->next != NULL) {
        // physically delete it only if
        // ieq won't end up empty bc of it
        sim.ieq.out = ieLink->next;
        free(ie);
        free(ieLink);
        continue;
      }
      else { // we took care of all events
        break;
      }
    }
  }
  sim.interrupted = 0;
}