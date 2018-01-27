#define __ASIM__

#include <stdio.h>
#ifdef __ASIM__
  #include "sim.h"
#endif
#include "bases.h"

// tmp
extern Arduino sim;
extern Diod diods[];

// to make it work:
int red = 0, yellow = 1;

void setup(void){
  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);
}

void blink(int pin){
  digitalWrite(pin, HIGH);
  delay(1000);
  digitalWrite(pin, LOW);
  delay(1000);
}

void loop(void){
  while(1) {blink(red);}
  //blink(yellow);
}

#ifdef __ASIM__
void init(void){
  setSim(UNO);
  diod(red, "red");
  diod(yellow, "yellow");
}

void main(void){
  init();
  setup();
  printf("%d  %p\n", sim.id == UNO, diods[0].pin);
  //return;
  launchThreads();
}

#endif

//---------------


// pinMode
//   if prevmode != NONE: error
// attachInterrupt
//   -> event listener
// digitalWrite
//   if mode != OUTPUT: error
//   -> event listener
// digitalRead
//   if mode != INPUT: error

// ---analogRead
// analogWrite
// delay
// timers

// create buttons,
// define names for colors, etc
// sim
// initialize global var `arduino`
// create event listeners (buttons, etc)
// call setup: modifies `arduino` and event handling
// forever:
//   call loop (only sets a list of things to do)
//   while todo not empty
//     do one thing
//       case of delay: merely defines a variable to decrement depending on time spent... except time spend during interruptions
//     check kb event, if true: launch listener
//     else: redo

// three threads: simulator, printer, and listener
// listener modifies state of buttons, and calls interruptions (or maybe puts it as next todo, in delay or outside of loop)
// simulator loops over loop, maybe calls interruptions
// printer continuously prints the state of the system (read only)