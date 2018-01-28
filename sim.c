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
DiodRGB diodRGBs[BIGN];
int diodRGBCount = 0;

void setSim(int id){
  int i;

  sim.id = id;

  DigitalPin *pin;
  for(i = 0; i < BIGN; i++){
    pin = &sim.pins[i];
    pin->mode = MODE_NONE;
    pin->value = LOW;
    pin->isAnalog = 0;
    pin->canInterrupt = 0;
    pin->canAnalog = 1; // will depend
  }

  if (id == UNO) {
    sim.minDigital = 0;
    sim.maxDigital = 10;
  }
}

void diod(int pinIx, char *name){
  if (diodCount >= BIGN || !checkDigital(pinIx)) {
    return; // ERROR
  }
  
  Diod *diod = &diods[diodCount];
  ++diodCount;
  setDisplayName(diod->name, name);
  diod->pin = &sim.pins[pinIx];
}

void button(int pinIx, char *name, char key) {
  if (buttonCount >= BIGN || !checkDigital(pinIx)) {
    return; // ERROR
  }
  
  Button *button = &buttons[buttonCount];
  ++buttonCount;
  setDisplayName(button->name, name);
  button->pin = &sim.pins[pinIx];
  button->key = key;
}

void diodRGB(int rIx, int gIx, int bIx, char *name){
  if (diodRGBCount >= BIGN ||
    !checkDigital(rIx) ||
    !checkDigital(gIx) ||
    !checkDigital(bIx)) {
    return; // ERROR
  }
  DiodRGB *diodRGB = &diodRGBs[diodRGBCount];
  ++diodRGBCount;
  setDisplayName(diodRGB->name, name);
  diodRGB->red = &sim.pins[rIx];
  diodRGB->green = &sim.pins[gIx];
  diodRGB->blue = &sim.pins[bIx];
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
  pin->value = value;
  pin->isAnalog = 0;
}

int digitalRead(int pinIx){
  if (!checkDigital(pinIx) || sim.pins[pinIx].mode != INPUT){
    return -1; //error
  }
  return sim.pins[pinIx].value;
}

void analogWrite(int pinIx, int value){
  if (!checkDigital(pinIx) || sim.pins[pinIx].mode != OUTPUT) {
    return; // ERROR
  }
  DigitalPin *pin = &sim.pins[pinIx];
  if (!pin->canAnalog) {
    return; // ERROR
  }
  pin->value = min(max(0, value), 255);
  pin->isAnalog = 1;
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
  printf("RGB diods:\n"); curRow++;
  for (i = 0; i < diodRGBCount && curRow < row - 1; i++){
    printTAB;
    printDiodRGB(&diodRGBs[i]);
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
  char state[4];
  state2Str(diod->pin, state);
  printf("%s [%s]", diod->name, state);
  printNL;
}

void printButton(Button *button){
  char state[4];
  state2Str(button->pin, state);
  printf("%s (key: %c) [%s]", button->name, button->key, state);
  printNL;
}

char colorNames[][8] = {
  "white",
  "red", "green", "blue",
  "yellow", "magenta", "cyan"
};

#define WWHI 0
#define RRED 1
#define GGRE 2
#define BBLU 3
#define YYEL 4
#define MMAG 5
#define CCYA 6


void printDiodRGB(DiodRGB *diodRGB){
  char redState[4], greenState[4], blueState[4];
  state2Str(diodRGB->red, redState);
  state2Str(diodRGB->green, greenState);
  state2Str(diodRGB->blue, blueState);
  
  int mainColor = getMainColor(
      diodRGB->red->value,
      diodRGB->green->value,
      diodRGB->blue->value);
  char *mainColorName = colorNames[mainColor];
  int  mixColor =
    getMix(
      mainColor,
      diodRGB->red->value,
      diodRGB->green->value,
      diodRGB->blue->value);
  char *mixColorName = colorNames[mixColor];
  printf("%s [R:%s G:%s B:%s] = *%7s -> %7s*",
    diodRGB->name,
    redState, greenState, blueState,
    mainColorName, mixColorName);
  printNL;
}

int getMainColor(int r, int g, int b){
  if (r == g && g == b) return WWHI;
  if (r == g && g > b) return YYEL;
  if (b == g && g > r) return CCYA;
  if (r == b && b > g) return MMAG;
  if (r > b) {
    if (r > g) return RRED;
    else // then g > r > b
      return GGRE;
  }
  // then b >= r
  if (b > g) // then b > (g & r)
    return BBLU;
  // then g > b >= r
  return GGRE;
}

int getMix(int mainColor, int r, int g, int b){
  if (mainColor == WWHI) return WWHI;
  if (mainColor == RRED){
    if (g == b) return RRED;
    if (g > b) return YYEL;
    else return MMAG;
  }
  if (mainColor == GGRE){
    if (r == b) return GGRE;
    if (r > b) return YYEL;
    else return CCYA;
  }
  if (mainColor == BBLU){
    if (r == g) return BBLU;
    if (r > g) return MMAG;
    else return CCYA;
  }
  return mainColor;
    // ^ if mainColor in cyan/yellow/magenta
}

void state2Str(DigitalPin *pin, char *str){
  if (pin->value == LOW) {
    sprintf(str, "   ");
  }
  else if (pin->isAnalog) {
    sprintf(str, "%3d", pin->value);
  }
  else { //if (pin->value == HIGH){
    sprintf(str, "###");
  }
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
  int pinIx;
  while (1) {
    while(!kbhit());
    c = fgetc(stdin);
    
    for (i = 0; i < buttonCount; i++){
      button = &buttons[i];
      if (button->key == c) {
        pinIx = button->pin - sim.pins;
        button->pin->value = (button->pin->value == 0);
        button->pin->isAnalog = 0; // theoretically not useful
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