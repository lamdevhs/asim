#include <string.h>
#include <stdio.h>

#include <sys/ioctl.h> // ioctl, TIOCGWINSZ
#include <unistd.h> // usleep
#include <pthread.h> // pthread_t

#include "sim.h"



// globals
Arduino sim;
Diod diods[BIGN];
int diodCount = 0;
Button buttons[BIGN];
int buttonCount = 0;

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
    sim.maxDigital = 10;
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

void button(int pinIx, char *name, char key) {
  if (!checkDigital(pinIx) || buttonCount >= BIGN) {
    return; // ERROR
  }
  
  Button *button = &buttons[buttonCount];
  ++buttonCount;
  setDisplayName(button->name, name);
  button->pin = &sim.pins[pinIx];
  button->key = key;
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

int digitalRead(int pinIx){
  if (!checkDigital(pinIx) || sim.pins[pinIx].mode != INPUT){
    return -1; //error
  }
  return sim.pins[pinIx].value;
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
  pthread_create(&tid, NULL, threadListener, NULL);
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
  
  printNL; curRow++;

  // printing diods
  printf("diods:\n"); curRow++;
  for (i = 0; i < diodCount && curRow < row - 1; i++){
    printTAB;
    printDiod(&diods[i]);
    curRow++;
  }

  // printing buttons
  printf("buttons:\n"); curRow++;
  for (i = 0; i < buttonCount && curRow < row - 1; i++){
    printTAB;
    printButton(&buttons[i]);
    curRow++;
  }
  
  // fill screen
  for (; curRow < row - 1; curRow++){
    printNL;
  }
}

void printDiod(Diod *diod){
  printf("%s [%c]", diod->name, digital2Char(diod->pin->value));
  printNL;
}

void printButton(Button *button){
  printf("%s (key: %c) [%c]", button->name,
    button->key,
    digital2Char(button->pin->value));
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



void *threadListener(void *_){
  int i;
  char c;
  Button *button;
  while (1) {
    while(!kbhit());
    c = fgetc(stdin);
    
    for (i = 0; i < buttonCount; i++){
      button = &buttons[i];
      if (button->key == c) {
        button->pin->value = 1 - button->pin->value;
      }
    }
  }
}




// -----------------
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

Bool kbhit(void)
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