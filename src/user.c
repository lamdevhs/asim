#include <stdlib.h> // malloc
#include <unistd.h> // usleep
#include <string.h>
#include "asim.h"

Arduino ardu;
  // ^ the simulated arduino
Diode diodes[BIGN];
int diodeCount = 0;
Button buttons[BIGN];
int buttonCount = 0;
Tricolor rgbLEDs[BIGN];
int rgbLEDCount = 0;
TrafficControl traffics[BIGN];
int trafficCount = 0;
ShiftRegister registers[BIGN];
int registerCount = 0;
Spy spies[BIGN];
int spiesCount = 0;



void *_threadUserCode(void *_) {
  setup();
  while (1) {
    loop();
  }
}

//| =====================================
//| == Functions to be called from init()
//| == (aside from the underscored ones)

void arduino(ArduinoType type){
  int i;

  ardu.type = type;

    //| dummy event, solely there to prevent
    //| the queue to ever be empty
    //| this being a relatively scrappy way
    //| to handle access to the same queue
    //| from two different threads... better do it
    //| with mutex, in future versions...
  IEvent *ie = (IEvent *)malloc(sizeof(IEvent));
  ie->pin = NULL;
  ie->dead = 1;

  Link *ieLink = (Link *)malloc(sizeof(Link));
  ieLink->content = (void *)ie;

  ardu.ieq.in = ieLink;
  ardu.ieq.out = ieLink;
  ardu.ieq.size = 0;

  ardu.interrupted = 0;

  //|first thing in the display list
  staticMessage("Simulator for code Arduino.");
  spy(&ardu.interrupted, "user code state", _printIsInterrupted);
  separator('`');

  DigitalPin *pin;

  for(i = 0; i < BIGN; i++){
    pin = &ardu.pins[i];
    pin->mode = MODE_NONE;
    pin->value = LOW;
    pin->canAnalog = 1;
      //| will depend, in later versions
    pin->isAnalog = 0;

    pin->canInterrupt = 0;
    pin->interrMode = NO_INTERR;
    pin->interrIx = -1;
    pin->interrFun = NULL;

    pin->onChange = NULL;
    pin->onChangeArg = NULL;

    ardu.interrupts[i] = NULL;
      //| ^ no connection with the rest of the loop
      //| both arrays just have the same BIGN size
  }

  if (type == UNO) {
    ardu.minDigital = 0;
    ardu.maxDigital = 13;
    
    _defineInterrupt(2, 0);
    _defineInterrupt(3, 1);
  }
  else if (type == MEGA) {
    ardu.minDigital = 0;
    ardu.maxDigital = 53;

    _defineInterrupt(2, 0);
    _defineInterrupt(3, 1);
    _defineInterrupt(18, 5);
    _defineInterrupt(19, 4);
    _defineInterrupt(20, 3);
    _defineInterrupt(21, 2);
      //| ^ from the specs.
      //| whoever chose this correspondence...
      //| no comment.
  }
  else {
    _fatalError("USER", "arduino", "unknown type of arduino");
  }
}

void _defineInterrupt(int pinIx, int interrIx){
  // TODO check range of input values
  DigitalPin *pin = &ardu.pins[pinIx];
  ardu.interrupts[interrIx] = pin;
  ardu.interrupts[1] = &ardu.pins[3];
  pin->canInterrupt = 1;
  pin->interrIx = interrIx;
}


//| validity of the pin number based on the type
//| of arduino chosen for the simulation.
Bool _isValidDigital(int pinIx){
  return (
    ardu.minDigital <= pinIx &&
    pinIx <= ardu.maxDigital
  );
}

void _checkValidDigital(int pinIx, char *fn){
  if (!_isValidDigital(pinIx))
    _fatalError("USER", fn,
      "pin number invalid for this type of arduino");
}

//| two-state simple output
//| can receive analog values too.
void diode(int pinIx, char *name){
  if (diodeCount >= BIGN) return; // FAILURE
  _checkValidDigital(pinIx, "diode");
  
  Diode *diode = &diodes[diodeCount];
  ++diodeCount;
  strncpy(diode->name, name, STRSIZE(SIZE_NAME));
  diode->pin = &ardu.pins[pinIx];

  _addToDisplayed((void *)diode, (int (*)(void *))_printDiode);
}

//| the button state is controlled by the user
//| via keyboard: the `key` char switches its state.
//| warning: you therefore need to hit twice the key
//| to simulate a press-then-released event.
void button(int pinIx, char *name, char key) {
  if (buttonCount >= BIGN) {
    return; // FAILURE
  }
  _checkValidDigital(pinIx, "button");
  
  Button *button = &buttons[buttonCount];
  ++buttonCount;
  strncpy(button->name, name, STRSIZE(SIZE_NAME));
  button->pin = &ardu.pins[pinIx];
  button->key = key;
  button->isPressed = 0;

  _addToDisplayed((void *)button, (int (*)(void *))_printButton);
}

//| i think this virtual object is rather self-descriptive.
void rgbLED(int rIx, int gIx, int bIx, char *name){
  if (rgbLEDCount >= BIGN){
    return; // FAILURE
  }
  _checkValidDigital(rIx, "rgbLED");
  _checkValidDigital(gIx, "rgbLED");
  _checkValidDigital(bIx, "rgbLED");

  Tricolor *rgbLED = &rgbLEDs[rgbLEDCount];
  ++rgbLEDCount;
  strncpy(rgbLED->name, name, STRSIZE(SIZE_NAME));
  rgbLED->red = &ardu.pins[rIx];
  rgbLED->green = &ardu.pins[gIx];
  rgbLED->blue = &ardu.pins[bIx];

  _addToDisplayed((void *)rgbLED, (int (*)(void *))_printTricolor);
}

//| represents traffic lights, those evil things that torment
//| cars and pedestrians alike.
//| similar to the rgbLED, but with a slightly different visual
//| representation.
void traffic(int rIx, int oIx, int gIx, char *name){
  if (trafficCount >= BIGN) {
    return; // FAIL
  }
  _checkValidDigital(rIx, "traffic");
  _checkValidDigital(oIx, "traffic");
  _checkValidDigital(gIx, "traffic");

  TrafficControl *traffic = &traffics[trafficCount];
  ++trafficCount;
  strncpy(traffic->name, name, STRSIZE(SIZE_NAME));
  traffic->red = &ardu.pins[rIx];
  traffic->orange = &ardu.pins[oIx];
  traffic->green = &ardu.pins[gIx];

  _addToDisplayed((void *)traffic, (int (*)(void *))_printTrafficControl);
}

//| valIx is for the pin that presents a new value
//| pushIx will make use of DigitalPin.onChange to
//| push the value of valIx in the hidden memory,
//| shifting the whole of it, erasing the last value
//| in a FIFO manner.
//| sendIx sends the hidden, temporary memory into the
//| visible one.
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
  reg->value = &ardu.pins[valIx];
  reg->push = &ardu.pins[pushIx];
  reg->send = &ardu.pins[sendIx];
  reg->size = size;
  reg->input = (int *)malloc(size*sizeof(int));
  reg->output = (int *)malloc(size*sizeof(int));
    // ^ TODO make sure they're zero-initialized
  reg->allVisible = allVisible;
  reg->printer = printer;

  reg->push->onChange = _registerPush;
  reg->push->onChangeArg = reg;
  reg->send->onChange = _registerSend;
  reg->send->onChangeArg = reg;

  _addToDisplayed((void *)reg, (int (*)(void *))_printShiftRegister);
}

void _registerPush(void *reg, DigitalPin *pin, int oldVal){
  ShiftRegister *r = (ShiftRegister *)reg;
  if (oldVal == LOW && pin->value == HIGH) {
    shiftList(r->input, r->size, r->value->value);
  }
}
void _registerSend(void *reg, DigitalPin *pin, int oldVal){
  ShiftRegister *r = (ShiftRegister *)reg;
  if (oldVal == LOW && pin->value == HIGH) {
    copyList(r->input, r->output, r->size);
  }
}

//| special kind of shiftRegister, used to simulate a 2-digit,
//| 2-dotted display, which will then look like:
//| #      _  #
//| #|_|   _| #
//| #  |. |_ .#
//| cf _printDigitalDisplay()
void digitalDisplay(int valIx, int pushIx, int sendIx, char *name){
  shiftRegister(
    valIx, pushIx, sendIx, name,
    16, _printDigitalDisplay, 0);
}

//| allows the user to view a real-time value of some
//| custom, user-defined int variable, with the option
//| to provide a custom printer.
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

//| prints a static (never changing) message.
//| everything will be printed in the order defined by
//| the user-written init().
void staticMessage(char *content){
    // first thing in the display list
  StaticMessage *smessage = (StaticMessage *)malloc(sizeof(StaticMessage));
  strncpy(smessage->message, content, STRSIZE(SIZE_LINE));
  _addToDisplayed((void *)smessage, (int (*)(void *))_printStaticMessage);
}

//| prints a line full of one character
//| repeated ad nauseam
void separator(char c){
  if (c == 0) c = '-';
  char *cpointer = malloc(sizeof(char));
  *cpointer = c;
  _addToDisplayed((void *)cpointer, (int (*)(void *))_printSeparator);
}



// ==============================================
// == Functions to be called from setup(), loop()


//| usleep() gets prematurely stopped if a signal
//| is sent to the process/thread that uses it.
//| so we cut the desired delay into small intervals.
//| when an error is detected, the interval is redone.
//| obviously it means the time spent waiting will
//| depend on external factors, but it'll always be
//| at least equal to `ms` millisecs.
int delay(int ms){
  while(ms) {
    int ans = usleep(10*1000);
    if (ans != 0) {
      //| some error impeded usleep's normal behavior,
      //| probably signal()
      continue;
    }
    else ms -= 10;
  }
}

//| the INPUT_PULLUP mode is handled in this
//| very limited fashion: the default state
//| becomes HIGH, and gets LOW when the button
//| is being pressed.
//| (this sim doesn't have any other INPUTs so far.)
void pinMode(int pinIx, PinMode mode){
  _checkValidDigital(pinIx, "pinMode");

  DigitalPin *pin = &ardu.pins[pinIx];
  if (pin->mode != MODE_NONE){
    // attempting to set the mode
    // of a pin a second time
    // WARNING?
  }
  if (mode < OUTPUT || mode >= MODE_NONE) {
    _fatalError("USER", "pinMode",
      "mode given in arg is invalid");
  }
  pin->mode = mode;

  if (mode == INPUT_PULLUP){
    pin->value = 1 - pin->value;
  }
}

void attachInterrupt(int interrIx, void (*interrFun)(void), InterruptMode mode){
  if (mode < LOW || mode >= NO_INTERR)
    _fatalError("USER", "attachInterrupt",
      "invalid interrupt mode");
  if (interrIx < 0 || BIGN <= interrIx) 
    _fatalError("USER", "attachInterrupt",
      "invalid interrupt index");
  
  DigitalPin *pin = ardu.interrupts[interrIx];
  if (pin == NULL)
    _fatalError("USER", "attachInterrupt",
      "invalid interrupt index");
  
  if (!pin->canInterrupt) // should never happen
    _fatalError("BUG", "attachInterrupt",
      "interrupt associated to pin that can't interrupt");
  
  if (pin->mode != INPUT_PULLUP && pin->mode != INPUT)
    // could be a mere warning... i'm not sure...
    _fatalError("USER", "attachInterrupt",
      "pin mode is not set as INPUT or INPUT_PULLUP");
  
  pin->interrFun = interrFun;
  pin->interrMode = mode;
}

void _checkPinModeSet(DigitalPin *pin, char *fn){
  if (pin->mode == MODE_NONE) {
    _fatalError("USER", fn, "pin mode not set");
  }
}

void digitalWrite(int pinIx, int value){
  _checkValidDigital(pinIx, "digitalWrite");
  DigitalPin *pin = &ardu.pins[pinIx];
  _checkPinModeSet(pin, "digitalWrite");
    

  if (pin->mode == INPUT || pin->mode == INPUT_PULLUP) {
    //| should issue a warning, this is weird behavior overall
    //| but that's apparently part of the specs:
    //| digitalWrite over an INPUT changes the pullup state of it
    //| aka default, off state is HIGH
    //| not sure if it can cause the triggering of an interrupt..
    //| into doubt, let's say it does:
    _digitalChangeValue(pin, value, 0);
  }
  _digitalChangeValue(pin, value, 0);
}

//| still don't know if you can read from an output pin...
//| into doubt, it's a fatal error, there :P
int digitalRead(int pinIx){
  _checkValidDigital(pinIx, "digitalRead");
  DigitalPin *pin = &ardu.pins[pinIx];
  _checkPinModeSet(pin, "digitalRead");

  if (pin->mode != INPUT && pin->mode != INPUT_PULLUP)
    _fatalError("USER", "digitalRead",
      "pin mode is not set as INPUT or INPUT_PULLUP");
  return pin->value;
}


void analogWrite(int pinIx, int value){
  _checkValidDigital(pinIx, "analogWrite");
  DigitalPin *pin = &ardu.pins[pinIx];
  _checkPinModeSet(pin, "analogWrite");

  if (pin->mode != OUTPUT) {
    _fatalError("USER", "analogWrite",
      "pin mode is not set as OUTPUT");
  }
  if (!pin->canAnalog) {
    _fatalError("USER", "analogWrite",
      "this digital pin cannot be used as analog output");
  }
  pin->value = min(max(0, value), 255);
  pin->isAnalog = 1;
}

int digitalPinToInterrupt(int pinIx){
  _checkValidDigital(pinIx, "digitalPinToInterrupt");
  DigitalPin *pin = &ardu.pins[pinIx];
  if (!pin->canInterrupt)
    _fatalError("USER", "digitalPinToInterrupt",
      "this digital pin cannot be used as interrupt");
  return pin->interrIx;
}
