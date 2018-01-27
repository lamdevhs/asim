#include <string.h>
#include <stdio.h>

#include <sys/ioctl.h> // ioctl, TIOCGWINSZ
#include <unistd.h> // usleep
#include <pthread.h> // pthread_t

#include "sim.h"



// globals
PinMode INPUT_PULLUP = INPUT;
Arduino sim;
Diod diods[BIGN];
int diodCount = 0;

void setSim(int id){
  int i;

  sim.id = id;

  DigitalPin *pin;
  for(i = 0; i < BIGN; i++){
    pin = &sim.pins[i];
    pin->mode = MODE_NONE;
    pin->value = LOW;
  }

  if (id == UNO) {
    sim.minDigital = 0;
    sim.maxDigital = 0;
  }
}

void diod(int pinIx, char *name){
  if (!checkDigital(pinIx) || diodCount >= BIGN) {
    return; // ERROR
  }
  
  Diod *diod = &diods[diodCount];
  ++diodCount;
  setDisplayName(diod->name, name);
  diod->pin = &sim.pins[pinIx];
}



void delay(int ms){
  usleep(ms * 1000);
}



void pinMode(int pinIx, PinMode mode){
  if (!checkDigital(pinIx)) {
    return; // ERROR
  }
  DigitalPin *pin = &sim.pins[pinIx];
  if (pin->mode != MODE_NONE){
    // WARNING
  }
  pin->mode = mode;
}

void digitalWrite(int pinIx, int value){
  if (!checkDigital(pinIx) || sim.pins[pinIx].mode != OUTPUT) {
    return; // ERROR
  }
  DigitalPin *pin = &sim.pins[pinIx];
  int prevValue = pin->value;
  pin->value = value;
}



Bool checkDigital(int pinIx){
  return (
    sim.minDigital <= pinIx &&
    pinIx <= sim.maxDigital
  );
}

void setDisplayName(char *dest, char *src){
  int len = strlen(src);
  int i;
  for (i = 0; i < SIZE_NAME - 1; i++) {
    if (i >= len) {
      dest[i] = '\0';
    }
    else {
      dest[i] = src[i];
    }
  }

  dest[SIZE_NAME - 1] = '\0';
}



void launchThreads(void){
  pthread_t tid;
  pthread_create(&tid, NULL, threadDisplay, NULL);
  pthread_create(&tid, NULL, threadLoop, NULL);
  pthread_exit(NULL);
  return;
}

void *threadDisplay(void *_) {
  while (1) {
    struct winsize winsize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize);
    printDisplay(winsize.ws_row, winsize.ws_col);
    usleep(DISPLAY_FREQ);
  }
}


void printDisplay(int row, int col){
  int curRow = 0;
  int curCol = 0;
  int i;

  // printing diods
  for (i = 0; i < diodCount && curRow < row - 1; i++){
    printDiod(&diods[i]);
    curRow++;
  }
  
  // fill screen
  for (; curRow < row - 1; curRow++){
    printNL;
  }
}

void printDiod(Diod *diod){
  printf("{diod} '%s': [%c]", diod->name, digital2Char(diod->pin->value));
  printNL;
}

char digital2Char(int value){
  if (value == HIGH){
    return '#';
  }
  return ' ';
}




void *threadLoop(void *_) {
  while (1) {
    loop();
    //usleep(1000 + DISPLAY_FREQ);
  }
}