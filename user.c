#include <stdlib.h> // malloc
#include <unistd.h> // usleep
#include <string.h>
#include "asim.h"

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
Spy spies[BIGN];
int spiesCount = 0;

void *_threadUserCode(void *_) {
  sim.userCodeThread = pthread_self();
  setup();
  while (1) {
    loop();
  }
}

// =====================================
// == Functions to be called from init()

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
  staticMessage("Simulator for code Arduino.");
  // StaticMessage *welcome =
  //   (StaticMessage *)malloc(sizeof(StaticMessage));
  // Link *welcomeLink = (Link *)malloc(sizeof(Link));
  // Printable *welcomePrintable = (Printable *)malloc(sizeof(Printable));

  // strncpy(welcome->message, "Simulator for code Arduino.", STRSIZE(SIZE_LINE));
  // welcomePrintable->object = (void *)welcome;
  // welcomePrintable->printer = printStaticMessage;
  // welcomeLink->content = (void *)welcomePrintable;

  // displayed.in = welcomeLink;
  // displayed.out = welcomeLink;

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
  else {
    _fatalError("USER", "arduino", "unknown type of arduino");
  }
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

void _checkValidDigital(int pinIx, char *fn){
  if (!_isValidDigital(pinIx))
    _fatalError("USER", fn,
      "pin number invalid for this type of arduino");
}


void diode(int pinIx, char *name){
  if (diodeCount >= BIGN) return; // FAILURE
  _checkValidDigital(pinIx, "diode");
  
  Diode *diode = &diodes[diodeCount];
  ++diodeCount;
  strncpy(diode->name, name, STRSIZE(SIZE_NAME));
  diode->pin = &sim.pins[pinIx];

  _addToDisplayed((void *)diode, (int (*)(void *))_printDiode);
}

void button(int pinIx, char *name, char key) {
  if (buttonCount >= BIGN) {
    return; // FAILURE
  }
  _checkValidDigital(pinIx, "button");
  
  Button *button = &buttons[buttonCount];
  ++buttonCount;
  strncpy(button->name, name, STRSIZE(SIZE_NAME));
  button->pin = &sim.pins[pinIx];
  button->key = key;
  button->isPressed = 0;

  _addToDisplayed((void *)button, (int (*)(void *))_printButton);
}

void tricolor(int rIx, int gIx, int bIx, char *name){
  if (tricolorCount >= BIGN){
    return; // FAILURE
  }
  _checkValidDigital(rIx, "tricolor");
  _checkValidDigital(gIx, "tricolor");
  _checkValidDigital(bIx, "tricolor");

  Tricolor *tricolor = &tricolors[tricolorCount];
  ++tricolorCount;
  strncpy(tricolor->name, name, STRSIZE(SIZE_NAME));
  tricolor->red = &sim.pins[rIx];
  tricolor->green = &sim.pins[gIx];
  tricolor->blue = &sim.pins[bIx];

  _addToDisplayed((void *)tricolor, (int (*)(void *))_printTricolor);
}

void trafficControl(int rIx, int oIx, int gIx, char *name){
  if (trafficControlCount >= BIGN) {
    return; // FAIL
  }
  _checkValidDigital(rIx, "trafficControl");
  _checkValidDigital(oIx, "trafficControl");
  _checkValidDigital(gIx, "trafficControl");

  TrafficControl *trafficControl = &trafficControls[trafficControlCount];
  ++trafficControlCount;
  strncpy(trafficControl->name, name, STRSIZE(SIZE_NAME));
  trafficControl->red = &sim.pins[rIx];
  trafficControl->orange = &sim.pins[oIx];
  trafficControl->green = &sim.pins[gIx];

  _addToDisplayed((void *)trafficControl, (int (*)(void *))_printTrafficControl);
}

void shiftRegister(
  int valIx, int pushIx, int sendIx, char *name, int size,
  int (*printer)(int *in, int *out, int), Bool allVisible
){
  if (registerCount >= BIGN) {
    return; // ERROR
  }
  _checkValidDigital(valIx, "shiftRegister");
  _checkValidDigital(pushIx, "shiftRegister");
  _checkValidDigital(sendIx, "shiftRegister");

  int ix = registerCount;
  ShiftRegister *reg = &registers[ix];
  ++registerCount;
  strncpy(reg->name, name, STRSIZE(SIZE_NAME));
  reg->value = &sim.pins[valIx];
  reg->push = &sim.pins[pushIx];
  reg->send = &sim.pins[sendIx];
  reg->size = size;
  reg->input = (int *)malloc(size*sizeof(int));
  reg->output = (int *)malloc(size*sizeof(int));
    // ^ TODO make sure they're zero-initialized
  reg->allVisible = allVisible;
  reg->printer = printer;

  reg->push->onChange = registerPush;
  reg->push->onChangeArg = reg;
  reg->send->onChange = registerSend;
  reg->send->onChangeArg = reg;

  _addToDisplayed((void *)reg, (int (*)(void *))_printShiftRegister);
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

void digitalDisplay(int valIx, int pushIx, int sendIx, char *name){
  shiftRegister(
    valIx, pushIx, sendIx, name,
    16, _printDigitalDisplay, 0);
}

void spy(int *pointer, char *name, int (*printer)(int val)){
  if (spiesCount >= BIGN) return; // FAIL
  if (pointer == NULL) return; // ERROR
  int ix = spiesCount;
  Spy *spy = &spies[ix];
  ++spiesCount;
  spy->pointer = pointer;
  strncpy(spy->name, name, STRSIZE(SIZE_NAME));
  spy->printer = printer;

  _addToDisplayed((void *)spy, (int (*)(void *))_printSpy);
}

void staticMessage(char *content){
    // first thing in the display list
  StaticMessage *smessage = (StaticMessage *)malloc(sizeof(StaticMessage));
  strncpy(smessage->message, content, STRSIZE(SIZE_LINE));
  _addToDisplayed((void *)smessage, (int (*)(void *))_printStaticMessage);
}

void separation(char c){
  if (c == 0) c = '-';
  char *cpointer = malloc(sizeof(char));
  *cpointer = c;
  _addToDisplayed((void *)cpointer, (int (*)(void *))_printSeparation);
}



// ==============================================
// == Functions to be called from setup(), loop()

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
  _checkValidDigital(pinIx, "pinMode");
  
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
  if (!_isValidDigital(pinIx) || sim.pins[pinIx].mode != OUTPUT) {
    return; // ERROR? or input pullup stuff? TODO
  }
  if (value != LOW && value != HIGH) return; // ERROR? or default value? TODO
  _digitalChangeValue(&sim.pins[pinIx], value, 0);
}


int digitalRead(int pinIx){
  if (!_isValidDigital(pinIx)) {
    return -1;
  }
  PinMode mode = sim.pins[pinIx].mode;
  if (mode != INPUT && mode != INPUT_PULLUP){
    return -1; //error?
  }
  return sim.pins[pinIx].value;
}

void analogWrite(int pinIx, int value){
  if (!_isValidDigital(pinIx) || sim.pins[pinIx].mode != OUTPUT) {
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
  if (!_isValidDigital(pinIx)) return -1; //ERROR
  DigitalPin *pin = &sim.pins[pinIx];
  if (!pin->canInterrupt) return -1; //ERROR
  return pin->interrIx;
}
