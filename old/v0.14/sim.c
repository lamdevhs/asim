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
  // ^ the simulated arduino
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
Spied spiedValues[BIGN];
int spiedValuesCount = 0;
Queue displayed;

void main(void){
  nonblock(NB_ENABLE);
  init();
    // ^ user-coded function
    // initializes `sim`,
    // creates the virtual objects

  signal(SIGUSR1, iEventHandler);
  launchThreads();
}

void setSim(int id){
  int i;

  sim.id = id;

    // dummy event, solely there to prevent
    // the queue to ever be empty
  IEvent *ie = (IEvent *)malloc(sizeof(IEvent));
  Link *ieLink = (Link *)malloc(sizeof(Link));
    // TODO check null ptr

  ie->pin = NULL;
  ie->dead = 1;

  ieLink->content = (void *)ie;

  sim.ieq.in = ieLink;
  sim.ieq.out = ieLink;
  sim.ieq.size = 0;

  sim.interrupted = 0;

  // first thing in the display list
  StaticMessage *welcome =
    (StaticMessage *)malloc(sizeof(StaticMessage));
  Link *welcomeLink = (Link *)malloc(sizeof(Link));
  Printable *welcomePrintable = (Printable *)malloc(sizeof(Printable));

  setDisplayName(welcome->message, "Simulator for code Arduino.");
  welcomePrintable->object = (void *)welcome;
  welcomePrintable->printer = printStaticMessage;
  welcomeLink->content = (void *)welcomePrintable;

  displayed.in = welcomeLink;
  displayed.out = welcomeLink;

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
    sim.maxDigital = BIGN - 1; // TODO temporary values
    
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

  addToDisplayList((void *)diod, printDiod);

}

void addToDisplayList(void *object, int (*printer)(void *o)){
  Printable *printable = (Printable *)malloc(sizeof(Printable));
  printable->object = object;
  printable->printer = printer;
  Link *link = (Link *)malloc(sizeof(Link));
  link->content = (void *)printable;
  link->next = NULL;
  pushInQueue(link, &displayed);
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

int spy(int *pointer, char *name){
  if (spiedValuesCount >= BIGN) return -1; // FAIL
  if (pointer == NULL) return -1; // ERROR
  int ix = spiedValuesCount;
  Spied *spied = &spiedValues[ix];
  ++spiedValuesCount;
  spied->pointer = pointer;
  setDisplayName(spied->name, name);
  spied->printer = NULL;
  //spied->symbolLimit = 0;
  return ix;
}

void spyWithPrinter(int *pointer, char *name, int (*printer)(int val)){
  int ix = spy(pointer, name);
  if (ix < 0 || ix >= spiedValuesCount) return; //FAIL/ERROR/BUG
  if (printer == NULL) return; //ERROR
  Spied *spied = &spiedValues[ix];
  int i, nextStart = 0;
  // for (i = 0; i < symbolLimit; i++) {
  //   nextStart = copyToSpace(spied->symbols[i], symbols + nextStart);
  //   if (nextStart == -1) break;
  // }
  //spied->symbols = symbols;
  // spied->symbolLimit = symbolLimit;
  spied->printer = printer;
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

// int copyToSpace(char *dest, char *src){
//   int len = firstSpace(src);
//   if (len == 0) return -1; //end of src
//   int i;
//   for (i = 0; i < SIZE_NAME - 1; i++) {
//     if (i >= len) {
//       dest[i] = '\0';
//     }
//     else {
//       dest[i] = src[i];
//     }
//   }
//   dest[SIZE_NAME - 1] = '\0';
//   return len + 1;
// }

// int firstSpace(char *src){
//   int i, len = strlen(src);
//   for (i = 0; i < len; i++){
//     if (src[i] == ' ') return i;
//   }
//   return len;
// }



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

// *************************
// int printInterr();
// int printEv(IEvent *ie);

void printDisplay(int row, int col){
  int curRow = 0;
  int curCol = 0;
  int i;
  
  printNL; curRow++;
  
  Link *link = displayed.out;
  Printable *printable;
  while(link != NULL) {
    printable = (Printable *)link->content;
    curRow += printable->printer(printable->object);
    link = link->next;
  }

  if (sim.interrupted) printf("[[interruption]]\n");
  else printf("\n");
  curRow++;

  // // printing diods
  // printf("diods:\n"); curRow++;
  // for (i = 0; i < diodCount && curRow < row - 1; i++){
  //   printTAB;
  //   printDiod(&diods[i]);
  //   curRow++;
  // }

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

  // printing spied values
  printf("spied values:\n"); curRow++;
  for (i = 0; i < spiedValuesCount && curRow < row - 1; i++){
    printTAB;
    printSpied(&spiedValues[i]);
    curRow++;
  }

  //   //debug ********
  // printf("\n[[DEBUG]]\n"); curRow+=2;
  // int ls = printInterr();
  // curRow += ls;


  
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

  // //DEBUG
  // int printInterr(){
  //   int ls = 0;
  //   printf("output\n"); ++ls;
  //   IEvent *ie = (IEvent *)sim.ieq.out->content;
  //   if (ie == NULL){
  //     printf("!!! NULL\n");
  //     return ++ls;
  //   }
  //   while(printEv(ie)){
  //     ++ls;
  //     ie = ie->next;
  //     if (ie == NULL){
  //       printf("NULL\n");
  //       return ++ls;
  //     }
  //   }
  //   return ++ls;
  // }

  // int printEv(Link *ieLink){
  //   IEvent *ie = (IEvent *)ieLink->content;
  //   printf("in: %d, out: %d, dead: %d, next is null: %d\n",
  //     ieLink == sim.ieq.in, ieLink == sim.ieq.out,
  //     ie->dead, ieLink->next == NULL);

  //   if (ie->next == NULL) return 0;
  //   else return 1;
  // }

  // // ---

int printDiod(void *o){
  Diod *diod = (Diod *)o;
  char state[4];
  state2Str(diod->pin, state);
  printf("[diod] %s [%s]", diod->name, state);
  printNL;
  return 1;
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

void printSpied(Spied *spied){
  if (spied->printer == NULL)
    printf("%s = %d", spied->name, *spied->pointer);
  else {
    printf("%s = ", spied->name);
    if (spied->printer(*spied->pointer) == -1){
      printf("%d", *spied->pointer);
    }
  }
  printNL;
  // }
  // else {
  //   //TODO check spied->pointer didn't get null
  //   int val = *spied->pointer;
  //   if (val >= 0 && val < spied->symbolLimit) {
  //     printf("%s = %s", spied->name, spied->symbols[val]);
  //   }
  // }
}

int printStaticMessage(void *o){
  StaticMessage *msg = (StaticMessage *)o;
  printf("%s\n", msg->message);
  return 1; // TODO depends on presence of \n's in the message
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
  
  // int mainColor = getMainColor(
  //     diodRGB->red->value,
  //     diodRGB->green->value,
  //     diodRGB->blue->value);
  // char *mainColorName = colorNames[mainColor];
  // int  mixColor =
  //   getMix(
  //     mainColor,
  //     diodRGB->red->value,
  //     diodRGB->green->value,
  //     diodRGB->blue->value);
  // char *mixColorName = colorNames[mixColor];
  printf("%s [%s %s %s] *%3s %5s %4s*",
    diodRGB->name,
    redState, greenState, blueState,
    (diodRGB->red->value != 0) ? "RED" : "",
    (diodRGB->green->value != 0) ? "GREEN" : "",
    (diodRGB->blue->value != 0) ? "BLUE" : ""
  );
    // mainColorName, mixColorName);
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
  setup();
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

void pushInQueue(Link *link, Queue *queue){
  if (queue->in == NULL) queue->out = link;
  else queue->in->next = link;
  queue->in = link;
  ++queue->size;
}

void addInterrupt(DigitalPin *pin){
  if (!pin->canInterrupt) return; // BUG!

  IEvent *ie = (IEvent *)malloc(sizeof(IEvent));
    // ^ this variable name already being cursed
    // i won't bother checking for null pointer
    // TODO
  Link *ieLink = (Link *)malloc(sizeof(Link));

  ie->pin = pin;
  ie->dead = 0;
  
  ieLink->content = (void *)ie;
  ieLink->next = NULL;
  pushInQueue(ieLink, &sim.ieq);
  pthread_kill(sim.loopThread, SIGUSR1);
}

void iEventHandler(int _){
  sim.interrupted = 1;
  while(1) {
    Link *ieLink = sim.ieq.out;
    IEvent *ie = (IEvent *)ieLink->content;
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
      if (ieLink->next != NULL) {
        // physically delete it only if
        // ieq won't end up empty bc of it
        sim.ieq.out = ieLink->next;
        free(ie);
        free(ieLink);
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
