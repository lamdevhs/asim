#define __ASIM__

#include <time.h>
#include <stdlib.h>

#include <stdio.h>
#ifdef __ASIM__
  #include "sim.h"
  #include <signal.h> // signal, SIGUSR1

extern Arduino sim;
extern Diod diods[];
extern int waaa;
#endif
#include "code.h"

// tmp


// physical:
int red = 0, yellow = 1;
int bYellow = 2, bRed = 3;

int good = 5;
int bad = 6;
int _blue = 7;


#define N 4
volatile int seq[N];
volatile int attemptIx = 0; // always will be % N


// state managers:
volatile Bool attempting = 0; // after showing sequence
volatile Bool restart = 0; // end game

typedef enum st {
  INIT,
  ATTEMPTING,
  OK,
  YOUWON,
  YOULOST
} State;

State state = INIT;


int randColorSeq(){
  if (rand() % 2 == 0) {
    return yellow;
  }
  return red;
}
void off(int which){
  digitalWrite(which, LOW);
}
void on(int which){
  digitalWrite(which, HIGH);
}
void newSeq(){
  int i;
  for(i = 0; i < N; i++){
    seq[i] = randColorSeq();
  }
}

#define BLINK_N 10
// ^ how much the diod blinks when losing/winning
#define BLINK_RATE 300

void showSeq(){
  int i, which;
  for(i = 0; i < N; i++){
    which = seq[i];
    on(which); delay(1000);
    off(which); delay(BLINK_RATE);
      // ^ to make the change more obvious
  }
}
void blink(int which, int amount){
  int i;
  for (i = 0; i < amount; i++){
    digitalWrite(which, HIGH);
    delay(BLINK_RATE);
    digitalWrite(which, LOW);
    delay(BLINK_RATE);
  }
}

void ok(){
  blink(good, 1);
}

void youWon(){
  blink(good, BLINK_N);
}
void gameOver(){
  blink(bad, BLINK_N);
}

void onButton2(int which){
  
  if (!attempting) return;
    // ^ not the right time, so do nothing
  if (seq[attemptIx] == which) {
    ok();
    attemptIx = (attemptIx + 1) % N;
    if (attemptIx == 0) { // successful end of attempt
      attempting = 0;
      youWon();
      restart = 1;  // game restarted
    }
  }
  else {
    attempting = 0; // unsuccessful end of attempt
    gameOver();
    restart = 1; // game restarted
  }
}

void onButton(int which){
  if (state != ATTEMPTING) return;
    // ^ not the right time, so do nothing
  if (seq[attemptIx] == which) {
    attemptIx = (attemptIx + 1) % N;
    if (attemptIx == 0) { // successful end of attempt
      state = YOUWON;
    }
    else state = OK;
  }
  else {
    state = YOULOST;
  }
}

void onRedButton() {
  //digitalWrite(red, HIGH);
  onButton(red);
}
void onYellowButton() {
  //digitalWrite(yellow, HIGH);
  onButton(yellow);
}


void setup(void){
  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);

  pinMode(good, OUTPUT);
  pinMode(bad, OUTPUT);

  pinMode(bYellow, INPUT);
  pinMode(bRed, INPUT);

  attachInterrupt(digitalPinToInterrupt(bYellow),
    onYellowButton, RISING);
  attachInterrupt(digitalPinToInterrupt(bRed),
    onRedButton, RISING);
}


void loop(void){
  on(red); on(yellow); delay(1000);
  off(red); off(yellow); delay(1000);
  //restart = 0;

  newSeq();
  showSeq();
  state = ATTEMPTING;
  while (1) {
    if (state == ATTEMPTING) {
      continue;
    }
    else if (state == OK) {
      ok();
      state = ATTEMPTING;
    }
    else if (state == YOUWON) {
      youWon();
      break;
    }
    else if (state == YOULOST) {
      gameOver();
      break;
    }
  }
}

#ifdef __ASIM__
void init(void){
  setSim(UNO);
  diod(red, "red");
  diod(yellow, "yellow");
  button(bYellow, "yellow", 'y');
  button(bRed, "red", 'r');
  diodRGB(bad, good, _blue, "score");
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