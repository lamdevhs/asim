#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/ioctl.h> // ioctl, TIOCGWINSZ
#include <unistd.h> // usleep
#include <pthread.h> // pthread_t, pthread_self

#include <signal.h> // signal, SIGUSR1

#include "sim.h"



// globals
Arduino sim;
Diod diods[BIGN];
int diodCount = 0;
Button buttons[BIGN];
int buttonCount = 0;
DiodRGB diodRGBs[BIGN];
int diodRGBCount = 0;
Traffic traffics[BIGN];
int trafficCount = 0;
Register registers[BIGN];
int registerCount = 0;


int waaa = 0;
int ieh = 0;

void setSim(int id){
  int i;

  sim.id = id;

    // dummy event, solely there to prevent
    // the queue to ever be empty
  IEvent *ie = (IEvent *)malloc(sizeof(IEvent));
    // TODO check null ptr

  ie->pin = NULL;
  ie->next = NULL;
  ie->dead = 1;

  sim.ieq.in = ie;
  sim.ieq.out = ie;
  sim.ieq.size = 0;

  sim.interrupted = 0;

  DigitalPin *pin;
  for(i = 0; i < BIGN; i++){
    pin = &sim.pins[i];
    pin->mode = MODE_NONE;
    pin->value = LOW;
    pin->canAnalog = 1; // will depend
    pin->isAnalog = 0;

    pin->canInterrupt = 0; // will depend
    pin->interrMode = NO_INTERR;
    pin->interrIx = -1;
    pin->interrFun = NULL;

    pin->onChange = NULL;
    pin->onChangeArg = NULL;

    sim.interrupts[i] = NULL;
  }

  if (id == UNO) {
    sim.minDigital = 0;
    sim.maxDigital = 30; // TOD temporary values
    
    defineInterrupt(2, 0);
    defineInterrupt(3, 1);
    defineInterrupt(18, 2);
    defineInterrupt(19, 3);
    defineInterrupt(20, 4);
    defineInterrupt(21, 5);
  }
}

void defineInterrupt(int pinIx, int interrIx){
  // TODO check range of input values
  DigitalPin *pin = &sim.pins[pinIx];
  sim.interrupts[interrIx] = pin;
  sim.interrupts[1] = &sim.pins[3];
  pin->canInterrupt = 1;
  pin->interrIx = interrIx;
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
  button->isPressed = 0;
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

void traffic(int rIx, int yIx, int gIx, char *name){
  if (trafficCount >= BIGN ||
    !checkDigital(rIx) ||
    !checkDigital(yIx) ||
    !checkDigital(gIx)) {
    return; // ERROR
  }
  Traffic *traffic = &traffics[trafficCount];
  ++trafficCount;
  setDisplayName(traffic->name, name);
  traffic->red = &sim.pins[rIx];
  traffic->yellow = &sim.pins[yIx];
  traffic->green = &sim.pins[gIx];
}

int mkRegister(int valIx, int pushIx, int sendIx, char *name, int size, int help){
  if (registerCount >= BIGN ||
    !checkDigital(valIx) ||
    !checkDigital(pushIx) ||
    !checkDigital(sendIx)) {
    return -1; // ERROR
  }
  int ix = registerCount;
  Register *reg = &registers[ix];
  ++registerCount;
  setDisplayName(reg->name, name);
  reg->value = &sim.pins[valIx];
  reg->push = &sim.pins[pushIx];
  reg->send = &sim.pins[sendIx];
  reg->size = size;
  reg->input = (int *)malloc(size*sizeof(int));
  reg->output = (int *)malloc(size*sizeof(int));
    // ^ TODO make sure they're zero-initialized
  reg->help = help;
  reg->printer = NULL;

  reg->push->onChange = registerPush;
  reg->push->onChangeArg = reg;
  reg->send->onChange = registerSend;
  reg->send->onChangeArg = reg;
  return ix;
}

void digitalDisplay(int valIx, int pushIx, int sendIx, char *name){
  int ix = mkRegister(valIx, pushIx, sendIx, name, 16, 0);
  if (ix < 0) return; // ERROR
  Register *reg = &registers[ix];
  reg->printer = printDigitalDisplay;
}



int delay_original(int ms){
  return usleep(ms * 1000);
}

int delay(int ms){
  while(ms) {
    int ans = usleep(10*1000);
    if (ans != 0) {
      // some error happened, probably signal
      continue;
    }
    else ms -= 10;
  }
}



void pinMode(int pinIx, PinMode mode){
  if (!checkDigital(pinIx)) {
    return; // ERROR
  }
  DigitalPin *pin = &sim.pins[pinIx];
  if (pin->mode != MODE_NONE){
    // WARNING?
  }
  pin->mode = mode;

  if (mode == INPUT_PULLUP){
    pin->value = 1 - pin->value;
  }
}

void attachInterrupt(int interrIx, void (*interrFun)(void), InterruptMode mode){
  // check validity of mode
  if (interrIx < 0 || BIGN <= interrIx) return; // ERROR
  DigitalPin *pin = sim.interrupts[interrIx];
  if (pin == NULL) return; // ERROR
  if (!pin->canInterrupt) return; // BUG!
  pin->interrFun = interrFun;
  pin->interrMode = mode;
}


void digitalWrite(int pinIx, int value){
  if (!checkDigital(pinIx) || sim.pins[pinIx].mode != OUTPUT) {
    return; // ERROR? or input pullup stuff? TODO
  }
  if (value != LOW && value != HIGH) return; // ERROR? or default value? TODO
  digitalChange(&sim.pins[pinIx], value, 0);
}


int digitalRead(int pinIx){
  if (!checkDigital(pinIx)) {
    return -1;
  }
  PinMode mode = sim.pins[pinIx].mode;
  if (mode != INPUT && mode != INPUT_PULLUP){
    return -1; //error?
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

int digitalPinToInterrupt(int pinIx){
  if (!checkDigital(pinIx)) return -1; //ERROR
  DigitalPin *pin = &sim.pins[pinIx];
  if (!pin->canInterrupt) return -1; //ERROR
  return pin->interrIx;
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


int printInterr();
int printEv(IEvent *ie);

void printDisplay(int row, int col){
  int curRow = 0;
  int curCol = 0;
  int i;

  printNL; curRow++;
  
  //debug
  int ls = printInterr();
  curRow += ls;

  if (sim.interrupted) printf("[[interruption]]\n");
  else printf("\n");
  curRow++;
  
  printf("waaa: %d\n", waaa); curRow++;

  // printing diods
  printf("diods:\n"); curRow++;
  for (i = 0; i < diodCount && curRow < row - 1; i++){
    printTAB;
    printDiod(&diods[i]);
    curRow++;
  }

  // printing rgb diods
  printf("RGB diods:\n"); curRow++;
  for (i = 0; i < diodRGBCount && curRow < row - 1; i++){
    printTAB;
    printDiodRGB(&diodRGBs[i]);
    curRow++;
  }

  printf("registers:\n"); curRow++;
  for (i = 0; i < registerCount && curRow < row - 1; i++){
    printTAB;
    if (registers[i].printer != NULL) {
      curRow += registers[i].printer(&registers[i]);
    }
    else {
      printRegister(&registers[i]);
      curRow++;
    }
  }

  printf("Traffic diods:\n"); curRow++;
  for (i = 0; i < trafficCount && curRow < row - 1; i++){
    printTAB;
    printTraffic(&traffics[i]);
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


void state2Str(DigitalPin *pin, char *str){
  if (pin->value == LOW) {
    sprintf(str, "   ");
  }
  else if (pin->isAnalog) {
    sprintf(str, "%3d", pin->value);
  }
  else if (pin->value == HIGH){
    sprintf(str, "###");
  }
  else {
    sprintf(str, "???");
    // bug!
  }
}

//DEBUG
int printInterr(){
  int ls = 0;
  printf("output\n"); ++ls;
  IEvent *ie = sim.ieq.out;
  if (ie == NULL){
    printf("!!! NULL\n");
    return ++ls;
  }
  while(printEv(ie)){
    ++ls;
    ie = ie->next;
    if (ie == NULL){
      printf("NULL\n");
      return ++ls;
    }
  }
  return ++ls;
}

int printEv(IEvent *ie){
  printf("in: %d, out: %d, dead: %d, next is null: %d\n",
    ie == sim.ieq.in, ie == sim.ieq.out,
    ie->dead, ie->next == NULL);

  if (ie->next == NULL) return 0;
  else return 1;
}

// ---

void printDiod(Diod *diod){
  char state[4];
  state2Str(diod->pin, state);
  printf("%s [%s]", diod->name, state);
  printNL;
}

void printButton(Button *button){
  char state[4];
  state2Str(button->pin, state);
  printf("%s (key: %c) [%s] %s", button->name, button->key, state,
    button->isPressed ? "(pressed)" : "");
  printNL;
}

void printTraffic(Traffic *traffic){
  printf("%s ->", traffic->name);
  if (traffic->red->value != 0)
    printf(" RED ");
  if (traffic->yellow->value != 0)
    printf(" ORANGE ");
  if (traffic->green->value != 0)
    printf(" GREEN ");
  printf("\n");
}

void printRegister(Register * reg){
  printf("%s: ", reg->name);
  if (reg->help) {
    printList(reg->input, reg->size);
    printf(" -> ");
  }
  printList(reg->output, reg->size);
  printf("\n");
}


int printDigitalDisplay(Register * reg){
  printf("%s:\n", reg->name);
  // first line
  printTAB;
  printf(" ");
  if (reg->output[0]) printf("_");
  else printf(" ");
  printf("    ");
  if (reg->output[0 + 8]) printf("_");
  else printf(" ");
  printNL;
  // line 2
  printTAB;
  if (reg->output[5]) printf("|");
  else printf(" ");
  if (reg->output[6]) printf("_");
  else printf(" ");
  if (reg->output[1]) printf("|");
  else printf(" ");
  printf("  ");
  if (reg->output[5+8]) printf("|");
  else printf(" ");
  if (reg->output[6+8]) printf("_");
  else printf(" ");
  if (reg->output[1+8]) printf("|");
  else printf(" ");
  printNL;
  // line 3
  printTAB;
  if (reg->output[4]) printf("|");
  else printf(" ");
  if (reg->output[3]) printf("_");
  else printf(" ");
  if (reg->output[2]) printf("|");
  else printf(" ");
  if (reg->output[7]) printf(".");
  else printf(" ");
  printf(" ");
  if (reg->output[4+8]) printf("|");
  else printf(" ");
  if (reg->output[3+8]) printf("_");
  else printf(" ");
  if (reg->output[2+8]) printf("|");
  else printf(" ");
  if (reg->output[7+8]) printf(".");
  else printf(" ");
  printNL;

  return 4; // number of lines printed
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





void *threadLoop(void *_) {
  sim.loopThread = pthread_self();
  while (1) {
    loop();
    //usleep(1000 + DISPLAY_FREQ);
  }
}

/*
void *threadInterruptions(void *_){
  while (1){
    continue;
    if (sim.nextInterrupt < 0) {
      sim.interrupted = 0;
      continue;
    }
    else {
      sim.interrupted = 1;
      if (sim.nextInterrupt) {
        DigitalPin *pin = sim.interrupts[sim.nextInterrupt];
        pin->interrFun();
      }
      else {} // BUG!

      sim.nextInterrupt = (sim.nextInterrupt + 1) % BIGN;
      if (sim.nextInterrupt == sim.freeInterrupt) {
        sim.nextInterrupt = -1;
        sim.interrupted = 0;
      }
    }
  }
}
*/

// interruptions
void digitalChange(DigitalPin *pin, int newValue, int fromListener){
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
    // triggered from loopThread

  if (pin->canInterrupt) {
    int changed = (newValue != oldValue);
    InterruptMode mode = pin->interrMode;

    if (mode == NO_INTERR) return;

    else if (mode == LOW || mode == HIGH){
      if (newValue == mode) addInterrupt(pin);
    }
    else if (!changed) return;

    else if (mode == RISING) {
      if (newValue == HIGH) addInterrupt(pin);
    }
    else if (mode == FALLING) {
      if (newValue == LOW) addInterrupt(pin);
    }
    else if (mode == CHANGE) addInterrupt(pin);

    else return; // BUG wrong value
    //pthread_kill(sim.loopThread, SIGUSR1);
  }
}

void addInterrupt(DigitalPin *pin){
  if (!pin->canInterrupt) return; // BUG!

  IEvent *ie = (IEvent *)malloc(sizeof(IEvent));
    // ^ this variable name already being cursed
    // i won't bother checking for null pointer
    // TODO

  ie->pin = pin;
  ie->next = NULL;
  ie->dead = 0;
  sim.ieq.in->next = ie;
  sim.ieq.in = ie;
    // in theory sim.ieq.in is always meant to be filled
    // by at least one event
  ++sim.ieq.size;

  pthread_kill(sim.loopThread, SIGUSR1);

  //printf("addInterrupt: %d %d\n", sim.freeInterrupt, sim.nextInterrupt); //&&&&&&&
  
}

void iEventHandler(int _){
  sim.interrupted = 1;
  while(1) {
    IEvent *ie = sim.ieq.out;
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
        --sim.ieq.size;
      }
    }
    
    
    if (ie->dead) {
      if (ie->next != NULL) {
        // physically delete it only if
        // ieq won't end up empty bc of it
        sim.ieq.out = ie->next;
        free(ie);
        continue;
      }
      else { // we took care of all events
        break;
      }
    }
  }
  sim.interrupted = 0;
}


void *threadListener(void *_){
  int i;
  char c;
  Button *button;
  //int pinIx;
  while (1) {
    //while(!kbhit());
    c = fgetc(stdin);
    printf("<%c>", c);
    
    for (i = 0; i < buttonCount; i++){
      button = &buttons[i];
      if (button->key == c) {
        //pinIx = button->pin - sim.pins;
        digitalChange(button->pin, SWITCH, 1);
        button->isPressed = 1 - button->isPressed;
      }
    }
  }
}



void registerPush(void *reg, DigitalPin *pin, int oldVal){
  Register *r = (Register *)reg;
  if (oldVal == LOW && pin->value == HIGH) {
    shiftList(r->input, r->size, r->value->value);
  }
}
void registerSend(void *reg, DigitalPin *pin, int oldVal){
  Register *r = (Register *)reg;
  if (oldVal == LOW && pin->value == HIGH) {
    copyList(r->input, r->output, r->size);
  }
}

void shiftList(int *xs, int size, int val){
  int i;
  for (i = 1; i < size; i++){
    xs[size - i] = xs[size - i - 1];
  }
  xs[0] = val;
}

void copyList(int *xs, int *into, int size){
  int i;
  for (i = 0; i < size; i++){
    into[i] = xs[i];
  }
}

void printList(int *xs, int size){
  int i;
  for (i = 0; i < size - 1; i++){
    printf("%d ", xs[i]);
  }
  printf("%d", xs[size - 1]);
}





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
