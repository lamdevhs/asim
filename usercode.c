#include <stdio.h>
#ifdef __ASIM__
  #include "asim.h"
#endif

// tmp
extern Arduino sim;
extern Diode diodes[];

// to make it work:
int red = 0, yellow = 1, btn = 2;
int r = 3, g = 4, b = 5;
int bt2 = 6;
int bt3 = 7;
int bt4 = 8;

int tR = 9;
int tY = 10;
int tG = 11;

// register
int rg1 = 12;
int rg2 = 13;
int rg3 = 14;


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
  // buttons
  pinMode(btn, INPUT);
  pinMode(bt2, INPUT_PULLUP);
  pinMode(bt3, INPUT_PULLUP);
  pinMode(bt4, INPUT_PULLUP);
  // diode rgb
  pinMode(r, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(b, OUTPUT);
  // trafficControl
  pinMode(tR, OUTPUT);
  pinMode(tY, OUTPUT);
  pinMode(tG, OUTPUT);
  // register
  pinMode(rg1, OUTPUT);
  pinMode(rg2, OUTPUT);
  pinMode(rg3, OUTPUT);
  attachInterrupt(0, foo, CHANGE);
}

#define FRQ 300
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

#define REGSIZE 16
int regVal = 0;
int regCount = 0;

void walk(){
  digitalWrite(rg1, 1);
  digitalWrite(rg2, LOW);
  digitalWrite(rg2, HIGH);
  digitalWrite(rg2, LOW);
  digitalWrite(rg3, LOW);
  digitalWrite(rg3, HIGH);
  digitalWrite(rg3, LOW);
}

void pushReg(){
  //printf("\n");
  digitalWrite(rg1, (++regVal % 2) && (regVal % 3));
  ++regCount;
  digitalWrite(rg2, LOW);
  digitalWrite(rg2, HIGH);
  digitalWrite(rg2, LOW);
  if (regCount == REGSIZE) {
    digitalWrite(rg3, LOW);
    digitalWrite(rg3, HIGH);
    digitalWrite(rg3, LOW);
    delay(1000);
    regCount = 0;
  }
}

int nums[][8] = {
  {1,1,1,1,1,1,0,0}, // 0
  {0,1,1,0,0,0,0,0}, // 1
  {1,1,0,1,1,0,1,0}, // 2
  {1,1,1,1,0,0,1,0}, // 3
  {0,1,1,0,0,1,1,0}, // 4
  {1,0,1,1,0,1,1,0}, // 5
  {1,0,1,1,1,1,1,0}, // 6
  {1,1,1,0,0,0,0,0}, // 7
  {1,1,1,1,1,1,1,0}, // 8
  {1,1,1,1,0,1,1,0}, // 9
};

void pushSeq(int rg1, int rg2, int rg3, int *seq, int size){
  digitalWrite(rg2, LOW);
  //printf("\n");
  int i;
  for (i = size - 1; i >= 0; i--){
    digitalWrite(rg1, seq[i]);
    digitalWrite(rg2, HIGH);
    digitalWrite(rg2, LOW);
  }
  digitalWrite(rg3, LOW);
  digitalWrite(rg3, HIGH);
  digitalWrite(rg3, LOW);
}
int xvar = 0;
int yvar = 0;
#define COLN 4
void loop(void){
  /*int bst = digitalRead(btn);
  if (bst == HIGH) {
    blink(r);
  }
  else blink(b);*/
  //onIfon(bt2, r);
  //onIfon(bt3, g);
  //onIfon(bt4, b);
  digitalWrite(tR, HIGH);
  digitalWrite(tY, HIGH);
  digitalWrite(tG, HIGH);
  digitalWrite(r, HIGH);
  digitalWrite(g, HIGH);
  digitalWrite(b, HIGH);
  delay(1000);
  xvar++;
  yvar = (yvar + 1) % COLN;
  pushSeq(rg1, rg2, rg3, nums[xvar % 10], 8);
  pushSeq(rg1, rg2, rg3, nums[(xvar % 100) / 10], 8);
}



enum colorss {
  cred,
  cblue,
  cblack,
  cgreen
};

int colorz(int val) {
  switch (val) {
    // case 0: printf("red"); break;
    // case 1: printf("blue"); break;
    // case 2: printf("black"); break;
    // case 3: printf("green"); break;
    SYMCASE(cred);
    SYMCASE(cblue);
    SYMCASE(cblack);
    SYMCASE(cgreen);
    default: return -1;
  }
}

#ifdef __ASIM__
void init(void){
  arduino(MEGA);
  diode(red, "red");
  diode(yellow, "yellow");
  button(btn, "one", 'c');
  button(bt2, "red", 'r');
  button(bt3, "green", 'g');
  separation('#');
  button(bt4, "blue", 'b');
  tricolor(r, g, b, "foo");
  trafficControl(tR, tY, tG, "horizon");
  digitalDisplay(rg1, rg2, rg3, "myreg");
  shiftRegister(rg1, rg2, rg3, "myreg", 16, NULL, 1);
  spy(&xvar, "xvar", NULL);
  spy(&yvar, "color", colorz);
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
