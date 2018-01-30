#define __ASIM__

#include <stdio.h>
#ifdef __ASIM__
  #include "sim.h"
  #include <signal.h> // signal, SIGUSR1
#endif
#include "code.h"

// tmp
extern Arduino sim;
extern Diod diods[];
extern int waaa;

// to make it work:
int red = 0, yellow = 1, btn = 2;
int r = 3, g = 4, b = 5;
int bt2 = 6;
int bt3 = 7;
int bt4 = 8;


int redval = LOW;
void foo(void){
  //printf("waa\n");
  redval = 1 - redval;
  digitalWrite(red, redval);
  delay(5000);
}

void setup(void){
  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(btn, INPUT);
  pinMode(bt2, INPUT_PULLUP);
  pinMode(bt3, INPUT_PULLUP);
  pinMode(bt4, INPUT_PULLUP);
  pinMode(r, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(b, OUTPUT);
  attachInterrupt(0, foo, CHANGE);
}

#define FRQ 200
void blink(int pin){
  digitalWrite(pin, HIGH);
  delay(FRQ);
  digitalWrite(pin, LOW);
  delay(FRQ);
}
void analogBlink(int pin){
  analogWrite(pin, 255);
  delay(FRQ);
  analogWrite(pin, 0);
  delay(FRQ);
}

void onIfon(int inPin, int outPin){
  int in = digitalRead(inPin);
  digitalWrite(outPin, in);
}
int yval = LOW;
void loop(void){
  /*int bst = digitalRead(btn);
  if (bst == HIGH) {
    blink(r);
  }
  else blink(b);*/
  //onIfon(bt2, r);
  //onIfon(bt3, g);
  //onIfon(bt4, b);
  waaa = delay(5000);
  yval = 1 - yval;
  digitalWrite(yellow, yval);
  while(1){}
}

#ifdef __ASIM__
void init(void){
  setSim(UNO);
  diod(red, "red");
  diod(yellow, "yellow");
  button(btn, "one", 'c');
  button(bt2, "red", 'r');
  button(bt3, "green", 'g');
  button(bt4, "blue", 'b');
  diodRGB(r, g, b, "foo");
}

void main(void){
  nonblock(NB_ENABLE);
  init();
  setup();
  signal(SIGUSR1, iEventHandler);
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