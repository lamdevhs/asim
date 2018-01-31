#define __ASIM__

#include <stdio.h>
#ifdef __ASIM__
  #include "sim.h"
  #include <signal.h> // signal, SIGUSR1

extern int waaa;
#include "code.h"

#endif

#define RED 0
#define ORANGE 1
#define GREEN 2
#define OFF 3

#define HORIZON 0
#define VERTICAL 1

#ifndef min
#define min(a, b) ((a < b) ? a : b)
#endif
int feux[][3] = {
  {6,5,4}, {9,8,7}
};

int pietons[2] = {20,21};

int voitures[2] = {18,19};

volatile int currentTraffic = HORIZON;
volatile int currentState = RED;
volatile int currentTimer = 0;



int setTraffic(int *feu, int state){
  int i = 0;
  for (i = 0; i < 3; i++){
    digitalWrite(feu[i], LOW);
  }
  if (state != OFF) digitalWrite(feu[state], HIGH);
}

int setBothTraffic(int traffic, int state){
  setTraffic(feux[1 - traffic], RED);
  setTraffic(feux[traffic], state);
}

void setDayTraffic(){
  setBothTraffic(currentTraffic, currentState);
}
#define FREQ 200

int sig(){
  int i;
  for (i = 0; i < 3; i++){
    setTraffic(feux[HORIZON], RED);
    delay(FREQ);
    setTraffic(feux[HORIZON], OFF);
    delay(FREQ);
    setTraffic(feux[VERTICAL], RED);
    delay(FREQ);
    setTraffic(feux[VERTICAL], OFF);
    delay(FREQ);
  }
}

void pietonInterrupt(int which){
  setBothTraffic(which, GREEN);
  return;

  if (currentTraffic != which) return;
  if (currentState != GREEN) return;

  currentTimer = min(currentTimer, 0);
}

void pietonInterrH(){
  pietonInterrupt(HORIZON);
}

void pietonInterrV(){
  pietonInterrupt(VERTICAL);
}

void (*(interrPietons[2]))() = {
  pietonInterrH,
  pietonInterrV
};


void setup(void){

  int i, j;
  for (j = 0; j < 2; j++) {
    for (i = 0; i < 3; i++) {
      pinMode(feux[j][i], OUTPUT);
      pinMode(feux[j][i], OUTPUT);
    }
    pinMode(pietons[j], INPUT);
    pinMode(voitures[j], INPUT);
    attachInterrupt(digitalPinToInterrupt(pietons[j]), interrPietons[j], RISING);
  }
}

void day(){
  if (currentTimer != 0) {
    --currentTimer;
    delay(1000);
    return;
  }
  else if (currentState == RED) {
    currentState = GREEN;
    currentTraffic = 1 - currentTraffic;
    currentTimer = 5;
  }
  else if (currentState == GREEN) {
    // check cars:
    if (digitalRead(voitures[1 - currentTraffic]) == HIGH) {
      currentState = ORANGE;
      currentTimer = 1;
    }
    else currentTimer = 1;
  }
  else if (currentState == ORANGE) {
    currentState = RED;
    currentTimer = 1;
  }
  setDayTraffic();
}

int firstTime = 1;
void loop(void){
  if (firstTime){
    firstTime = 0;
    sig();
  }
  //day();
}

#ifdef __ASIM__
void init(void){
  setSim(UNO);
  traffic(6, 5, 4, "feuH");
  traffic(9, 8, 7, "feuV");
  traffic(9, 0, 7, "pietonH");
  traffic(6, 0, 4, "pietonV");
  button(pietons[0], "pietonH", 'h');
  button(pietons[1], "pietonV", 'v');
  button(voitures[0], "voitureH", 'a');
  button(voitures[1], "voitureV", 'z');
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
