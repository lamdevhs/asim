#include <stdlib.h>
#include "asim.h"

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



Bool checkDigital(int pinIx){
  return (
    sim.minDigital <= pinIx &&
    pinIx <= sim.maxDigital
  );
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


void printDiodRGB(DiodRGB *diodRGB){
  char redState[4], greenState[4], blueState[4];
  state2Str(diodRGB->red, redState);
  state2Str(diodRGB->green, greenState);
  state2Str(diodRGB->blue, blueState);
  
  printf("%s [%s %s %s] *%3s %5s %4s*",
    diodRGB->name,
    redState, greenState, blueState,
    (diodRGB->red->value != 0) ? "RED" : "",
    (diodRGB->green->value != 0) ? "GREEN" : "",
    (diodRGB->blue->value != 0) ? "BLUE" : ""
  );
  printNL;
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