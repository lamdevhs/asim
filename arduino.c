#include <stdio.h>



void pint(int i){
  printf("%d\n", i);
}

void pintt(int* i){
  printf("%p\n", (void *) i);
}




#define __ASIM__
#define BIGN 50


#define LOW 0
#define HIGH 1


#define UNO 0

typedef enum {
  OUTPUT,
  INPUT,
  INTERRUPT,
  NONE
} PinMode;

PinMode INPUT_PULLUP = INPUT;
  // ^ since not relevant to the simulator

typedef struct DigitalPin {
  PinMode mode;
  int value; // HIGH or LOW
  int bInterrupt;
} DigitalPin;

typedef struct Arduino {
  int id;
  int val;
  DigitalPin pins[BIGN];
  int minDigitalPin;
  int maxDigitalPin;
} Arduino;

Arduino sim;

void initUno() {
  sim.val = 42;
  //sim.pins.value = LOW;
}


Arduino init(int id) {
  int i;
  //Arduino out = sim;
  sim.id = UNO;
  sim.val = 0;
  for (i = 0; i < BIGN; i++){
    sim.pins[i].value = 55;
  }
  if (id == UNO) {
    sim.minDigitalPin = 1;
    sim.maxDigitalPin = 1;
    initUno();
  }
  //sim = sim;
  printf("%d\n", sim.val);
}

void crash(){}

void me(){
  printf("<simulator> ");
}

void name(int pidId, char* str){
  
}

void pinMode(int pinId, int mode){
  if (pinId < sim.minDigitalPin || sim.maxDigitalPin < pinId) {
    me();
    printf("pinMode(): pinId = %d is not valid for this arduino.\n", pinId);
    crash(); return;
  }
}
void digitalWrite(int pinId, int value){}

void setup();
void loop();

int simulate() {
  printf("%d\n", sim.val);
  printf("%d\n", sim.pins[20].value);
  setup();
  loop();
  return 0;
}


// test

int red = 0;

void setup(){
  name(red, "red");
  pinMode(red, OUTPUT);
}


void loop(){
  digitalWrite(red, HIGH);
}

void main() {
  init(UNO);
  int res = simulate();
}


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