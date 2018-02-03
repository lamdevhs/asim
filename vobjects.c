#include <stdlib.h>
#include <stdio.h>
#include "asim.h"

// globals
Arduino sim;
  // ^ the simulated arduino
Diode diodes[BIGN];
int diodeCount = 0;
Button buttons[BIGN];
int buttonCount = 0;
Tricolor tricolors[BIGN];
int tricolorCount = 0;
TrafficControl trafficControls[BIGN];
int trafficControlCount = 0;
ShiftRegister registers[BIGN];
int registerCount = 0;
Spy spyValues[BIGN];
int spyValuesCount = 0;
Queue displayed;


char *termColors[8] = {
  "\x1B[31m",
  "\x1B[32m",
  "\x1B[33m",
  "\x1B[34m",
  "\x1B[35m",
  "\x1B[36m",
  "\x1B[37m",
  "\x1B[0m"
};




void arduino(ArduinoType type){
  int i;

  sim.type = type;

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

  strcpyUpTo(welcome->message, "Simulator for code Arduino.", SIZE_LINE);
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

  if (type == UNO) {
    sim.minDigital = 0;
    sim.maxDigital = 13;
    
    _defineInterrupt(2, 0);
    _defineInterrupt(3, 1);
  }
  else if (type == MEGA) {
    sim.minDigital = 0;
    sim.maxDigital = 53;

    _defineInterrupt(2, 0);
    _defineInterrupt(3, 1);
    _defineInterrupt(18, 5);
    _defineInterrupt(19, 4);
    _defineInterrupt(20, 3);
    _defineInterrupt(21, 2);
  }
}


void diode(int pinIx, char *name){
  if (diodeCount >= BIGN || !_isValidDigital(pinIx)) {
    return; // ERROR
  }
  
  Diode *diode = &diodes[diodeCount];
  ++diodeCount;
  strcpyUpTo(diode->name, name, SIZE_NAME);
  diode->pin = &sim.pins[pinIx];

  _addToDisplayed((void *)diode, (int (*)(void *))_printDiode);

}

int _printDiode(Diode *diode){
  char state[4];
  _state2Str(diode->pin, state);
  printf("diode\t[%s] %s", state, diode->name);
  printNL;
  return countLines(diode->name);
}

void button(int pinIx, char *name, char key) {
  if (buttonCount >= BIGN || !_isValidDigital(pinIx)) {
    return; // ERROR
  }
  
  Button *button = &buttons[buttonCount];
  ++buttonCount;
  strcpyUpTo(button->name, name, SIZE_NAME);
  button->pin = &sim.pins[pinIx];
  button->key = key;
  button->isPressed = 0;

  _addToDisplayed((void *)button, (int (*)(void *))_printButton);
}

int _printButton(Button *button){
  char state[4];
  _state2Str(button->pin, state);
  printf("button\t[%s] %s\t(key %c) %s", state, button->name, button->key,
    button->isPressed ? "(pressed)" : "");
  printNL;
  return countLines(button->name);
}

void tricolor(int rIx, int gIx, int bIx, char *name){
  if (tricolorCount >= BIGN ||
    !_isValidDigital(rIx) ||
    !_isValidDigital(gIx) ||
    !_isValidDigital(bIx)) {
    return; // ERROR
  }
  Tricolor *tricolor = &tricolors[tricolorCount];
  ++tricolorCount;
  strcpyUpTo(tricolor->name, name, SIZE_NAME);
  tricolor->red = &sim.pins[rIx];
  tricolor->green = &sim.pins[gIx];
  tricolor->blue = &sim.pins[bIx];

  _addToDisplayed((void *)tricolor, (int (*)(void *))_printTricolor);
}

int _printTricolor(Tricolor *tricolor){
  char redState[4], greenState[4], blueState[4];
  _state2Str(tricolor->red, redState);
  _state2Str(tricolor->green, greenState);
  _state2Str(tricolor->blue, blueState);

  printf("RGB\t[%s%s%s|%s%s%s|%s%s%s] %s",
    termColors[RED], redState, termColors[NONE],
    termColors[GREEN], greenState, termColors[NONE],
    termColors[BLUE], blueState, termColors[NONE],
    tricolor->name);
  printNL;
  return countLines(tricolor->name);
}

void trafficControl(int rIx, int oIx, int gIx, char *name){
  if (trafficControlCount >= BIGN ||
    !_isValidDigital(rIx) ||
    !_isValidDigital(oIx) ||
    !_isValidDigital(gIx)) {
    return; // ERROR
  }
  TrafficControl *trafficControl = &trafficControls[trafficControlCount];
  ++trafficControlCount;
  strcpyUpTo(trafficControl->name, name, SIZE_NAME);
  trafficControl->red = &sim.pins[rIx];
  trafficControl->orange = &sim.pins[oIx];
  trafficControl->green = &sim.pins[gIx];

  _addToDisplayed((void *)trafficControl, (int (*)(void *))_printTrafficControl);
}

int _printTrafficControl(TrafficControl *trafficControl){
  char redState[4], greenState[4], orangeState[4];
  _state2Str(trafficControl->red, redState);
  _state2Str(trafficControl->green, greenState);
  _state2Str(trafficControl->orange, orangeState);

  printf("traffic\t[%s%s%s|%s%s%s|%s%s%s] %s",
    termColors[RED], redState, termColors[NONE],
    termColors[ORANGE], orangeState, termColors[NONE],
    termColors[GREEN], greenState, termColors[NONE],
    trafficControl->name);
  printNL;
  return countLines(trafficControl->name);
}

void shiftRegister(
  int valIx, int pushIx, int sendIx, char *name, int size,
  int (*printer)(ShiftRegister *), Bool allVisible
){
  if (registerCount >= BIGN ||
    !_isValidDigital(valIx) ||
    !_isValidDigital(pushIx) ||
    !_isValidDigital(sendIx)) {
    return; // ERROR
  }
  int ix = registerCount;
  ShiftRegister *reg = &registers[ix];
  ++registerCount;
  strcpyUpTo(reg->name, name, SIZE_NAME);
  reg->value = &sim.pins[valIx];
  reg->push = &sim.pins[pushIx];
  reg->send = &sim.pins[sendIx];
  reg->size = size;
  reg->input = (int *)malloc(size*sizeof(int));
  reg->output = (int *)malloc(size*sizeof(int));
    // ^ TODO make sure they're zero-initialized
  reg->allVisible = allVisible;
  reg->printer =
    (printer != NULL)
    ? printer
    : _printShiftRegister;

  reg->push->onChange = registerPush;
  reg->push->onChangeArg = reg;
  reg->send->onChange = registerSend;
  reg->send->onChangeArg = reg;

  _addToDisplayed((void *)reg, (int (*)(void *))reg->printer);
}

int _printShiftRegister(ShiftRegister * reg){
  int lines = 1;
  if (reg->allVisible) {
    printf("((");
    printList(reg->input, reg->size);
    printf("))\n");
  }
  printf("//");
  printList(reg->output, reg->size);
  printf("//");
  printf(" %s", reg->name);
  printNL;
  return !!reg->allVisible + countLines(reg->name);
}

void digitalDisplay(int valIx, int pushIx, int sendIx, char *name){
  shiftRegister(
    valIx, pushIx, sendIx, name,
    16, _printDigitalDisplay, 0);
}

int _printDigitalDisplay(ShiftRegister * reg){
  printf("digital\t%s:\n", reg->name);
  // first line
  printf("# ");
  if (reg->output[0]) printf("_");
  else printf(" ");
  printf("    ");
  if (reg->output[0 + 8]) printf("_");
  else printf(" ");
  printf("  #");
  printNL;
  // line 2
  printf("#");
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
  printf(" #");
  printNL;
  // line 3
  printf("#");
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
  printf("#");
  printNL;

  return 3 + countLines(reg->name); // number of lines printed
}

void spy(int *pointer, char *name, int (*printer)(int val)){
  if (spyValuesCount >= BIGN) return; // FAIL
  if (pointer == NULL) return; // ERROR
  int ix = spyValuesCount;
  Spy *spy = &spyValues[ix];
  ++spyValuesCount;
  spy->pointer = pointer;
  strcpyUpTo(spy->name, name, SIZE_NAME);
  spy->printer = printer;

  _addToDisplayed((void *)spy, (int (*)(void *))_printSpy);
}

int _printSpy(Spy *spy){
  printf("spy\t");
  if (spy->printer == NULL)
    printf("%s = %d", spy->name, *spy->pointer);
  else {
    printf("%s = ", spy->name);
    if (spy->printer(*spy->pointer) == -1){
      printf("%d", *spy->pointer);
    }
  }
  printNL;
  return countLines(spy->name);
  // }
  // else {
  //   //TODO check spy->pointer didn't get null
  //   int val = *spy->pointer;
  //   if (val >= 0 && val < spy->symbolLimit) {
  //     printf("%s = %s", spy->name, spy->symbols[val]);
  //   }
  // }
}

// void spyWithPrinter(int *pointer, char *name, int (*printer)(int val)){
//   int ix = spy(pointer, name);
//   if (ix < 0 || ix >= spyValuesCount) return; //FAIL/ERROR/BUG
//   if (printer == NULL) return; //ERROR
//   Spy *spy = &spyValues[ix];
//   int i, nextStart = 0;
//   // for (i = 0; i < symbolLimit; i++) {
//   //   nextStart = copyToSpace(spy->symbols[i], symbols + nextStart);
//   //   if (nextStart == -1) break;
//   // }
//   //spy->symbols = symbols;
//   // spy->symbolLimit = symbolLimit;
//   spy->printer = printer;
// }






void _state2Str(DigitalPin *pin, char *str){
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

















int printStaticMessage(void *o){
  StaticMessage *msg = (StaticMessage *)o;
  printf("%s\n", msg->message);
  return 1; // TODO depends on presence of \n's in the message
}


// void printTricolor(Tricolor *tricolor){
//   char redState[4], greenState[4], blueState[4];
//   _state2Str(tricolor->red, redState);
//   _state2Str(tricolor->green, greenState);
//   _state2Str(tricolor->blue, blueState);
  
//   printf("%s [%s %s %s] *%3s %5s %4s*",
//     tricolor->name,
//     redState, greenState, blueState,
//     (tricolor->red->value != 0) ? "RED" : "",
//     (tricolor->green->value != 0) ? "GREEN" : "",
//     (tricolor->blue->value != 0) ? "BLUE" : ""
//   );
//   printNL;
// }





void separation(char c){
  if (c == 0) c = '-';
  char *cpointer = malloc(sizeof(char));
  *cpointer = c;
  _addToDisplayed((void *)cpointer, (int (*)(void *))_printSeparation);
}

int _printSeparation(char *cpointer){
  int i;
  for (i = 0; i < winColSize - 1; ++i)
    printf("%c", *cpointer);
  printNL;
  return 1;
}





void registerPush(void *reg, DigitalPin *pin, int oldVal){
  ShiftRegister *r = (ShiftRegister *)reg;
  if (oldVal == LOW && pin->value == HIGH) {
    shiftList(r->input, r->size, r->value->value);
  }
}
void registerSend(void *reg, DigitalPin *pin, int oldVal){
  ShiftRegister *r = (ShiftRegister *)reg;
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

void _defineInterrupt(int pinIx, int interrIx){
  // TODO check range of input values
  DigitalPin *pin = &sim.pins[pinIx];
  sim.interrupts[interrIx] = pin;
  sim.interrupts[1] = &sim.pins[3];
  pin->canInterrupt = 1;
  pin->interrIx = interrIx;
}



Bool _isValidDigital(int pinIx){
  return (
    sim.minDigital <= pinIx &&
    pinIx <= sim.maxDigital
  );
}