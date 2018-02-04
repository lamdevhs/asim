#include <stdlib.h>
#include <signal.h> // signal, SIGUSR1
#include <pthread.h>
#include "asim.h"


//| thread responsible for continuously waiting
//| for a character on stdin, then changing the
//| state of all buttons whose Button.key is this character.
//| the use of _digitalChangeValue() allows interruptions
//| to be generated thus.
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


//| changes the state of the pin.
//| this wrapper is used to trigger potential events.
//| if it exists, calls the onChange event handler for the pin.
//| adds an interruption to the queue (via _addInterrupt)
//| if the conditions match:
//| * if the pin can interrupt (is one of the interrupt pins)
//| * the change of value matches the event meant to trigger
//|   the interruption.
//| cf attachInterrupt().
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
    // triggered from threadUserCode

  if (pin->canInterrupt) {
    int changed = (newValue != oldValue);
    InterruptMode mode = pin->interrMode;

    if (mode == NO_INTERR) return;

    else if (mode == LOW || mode == HIGH){
      if (newValue == mode) _addInterrupt(pin);
    }
    else if (!changed) return;

    else if (mode == RISING) {
      if (newValue == HIGH) _addInterrupt(pin);
    }
    else if (mode == FALLING) {
      if (newValue == LOW) _addInterrupt(pin);
    }
    else if (mode == CHANGE) _addInterrupt(pin);

    else {
      //| wrong mode value; should never happen
      _fatalError("BUG", "_digitalChangeValue",
        "unknown pin.interrMode encountered");
    }
  }
}

//| add an interruption to the queue, and then
//| calls for an interruption of the userCode thread.
void _addInterrupt(DigitalPin *pin){
  if (!pin->canInterrupt) return; // BUG!

  IEvent *ie = (IEvent *)malloc(sizeof(IEvent));
  Link *ieLink = (Link *)malloc(sizeof(Link));

  ie->pin = pin;
  ie->dead = 0;
  
  ieLink->content = (void *)ie;
  ieLink->next = NULL;
  pushInQueue(ieLink, &ardu.ieq);
  pthread_kill(threadUserCode, SIGUSR1);
}

//| handler called when threadUserCode receives SIGUSR1
void _interruptSignalHandler(int _){
  ardu.interrupted = 1;
  while(1) {
    Link *ieLink = ardu.ieq.out;
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
        --ardu.ieq.size;
      }
    }
    
    
    if (ie->dead) {
      if (ieLink->next != NULL) {
        // physically delete it only if
        // ieq won't end up empty bc of it
        ardu.ieq.out = ieLink->next;
        free(ie);
        free(ieLink);
        continue;
      }
      else { // we took care of all events
        break;
      }
    }
  }
  ardu.interrupted = 0;
}