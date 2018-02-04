#ifndef FILE_ASIM
#define FILE_ASIM

//| one header to rule them all...

#include <pthread.h> //| pthread_t
#include <stdio.h>
  //| ^ for using printf in macro SYMCASE


// NOTE: aside from the functions in "tools.c",
// internal functions all start with an underscore
// (aside from main() of course)
// global variable don't follow that rule
// but none of them is meant to be directly
// accessed by the user anyway.


// user-written (`abstract`) functions
  void setup(void);
  void loop(void);

  void init(void);
    // ^ called before the rest,
    // allows the user to set up
    // the simulated arduino and
    // define other virtual objects
    // cf user.c




typedef int Bool;

#define dbgprintf printf

//| general constant quantities
  #define BIGN 100
    //| ^ the amount of pins/diodes/buttons/etc
    //| that gets statically allocated
  #define SIZE_NAME 100
    //| ^ max size for the name given by the user
    //| when creating a diode/button/etc in init().
    //| cf user.c
  #define SIZE_LINE 200
    //| ^ max size for static messages
    //| aka messages constantly printed by _threadView()
  #define DISPLAY_FREQ 50*1000
    //| ^ cf _threadView()

//| shortcuts
  #define STRSIZE(nb) (nb * sizeof(char))
  #define printNL printf("\n")
  #define printTAB printf("\t")
  #define min(a, b) (a < b ? a : b)
  #define max(a, b) (a > b ? a : b)

//| various constant values
  typedef enum {
   RED,
   GREEN,
   ORANGE,
   BLUE,
   MAGENTA,
   CYAN,
   WHITE,
   NONE
  } TermColor;
    //| cf view.c
    //| esp the global variable `char *termColors[8]`

  typedef enum {
    UNO,
    MEGA
  } ArduinoType;
    //| ^ cf arduino()
    //| this list might grow in the future
    //| to include more types of arduinos

  typedef enum {
    OUTPUT,
    INPUT,
    INPUT_PULLUP,
      // ^ will amount to setting HIGH as off state
    MODE_NONE
  } PinMode;
    //| ^ cf pinMode()

  typedef enum {
    LOW,
    HIGH,
    FALLING,
    RISING,
    CHANGE,
    NO_INTERR
  } InterruptMode;
   //| ^ cf attachInterrupt()

  #define LOW 0
  #define HIGH 1
    //| ^ for clarity's sake.
    //| obviously redundant since C
    //| has no type safety

//| Queue
  typedef struct link
  {
    void *content;
    struct link *next;
  } Link;

  typedef struct queue
  {
    Link *in; //| where elements are added
    Link *out; //| from where elements are treated/deleted
    int size;
  } Queue;
  //| FIFO link-based queue



//| =================================
//| main structures of the simulation

typedef struct digitalPin {
  PinMode mode;
  int value; //| HIGH or LOW or 0-255
  Bool isAnalog; //| is the current value analog?
  Bool canAnalog; //| can be used as analog output?
  Bool canInterrupt; //| can be used as interrupt?
  int interrIx; //| interrupt index in Arduino.interrupts
  InterruptMode interrMode;
  void (*interrFun)(void); //| function to call upon interruption

  void (*onChange)(void *arg, struct digitalPin *pin, int oldVal);
    //| ^ optional function to be called whenever the value changes
    //| useful for the (virtual) shift registers
  void *onChangeArg; //| argument for onChange()
} DigitalPin;

typedef struct interruptEvent
{
  DigitalPin *pin;
    //| pin that triggered the event
  Bool dead;
    //| signals that the event has been taken care of
} IEvent;
  //| interrupt event; malloc()ated
  //| to be queued, waiting to be processed
  //| by _interruptSignalHandler()

typedef struct arduino {
  ArduinoType type;
  DigitalPin pins[BIGN];
  int minDigital; // smallest valid digital pin number
  int maxDigital; // biggest [...]
  DigitalPin *interrupts[BIGN];
    // ^ pointers for pins which are valid interrupts
    // the index of the items is meant to match
    // the interrupt number to give to attachInterrupt()
  Queue ieq;
    // ^ interrupt events queue
  Bool interrupted;
    // ^ is threadUserCode currently treating an interruption?
} Arduino;

typedef struct diode
{
  char name[SIZE_NAME];
  DigitalPin *pin;
} Diode;

typedef struct button
{
  char name[SIZE_NAME];
  Bool isPressed;
  DigitalPin *pin;
  char key;
    // ^ keyboard key to press
    // to change the button state
} Button;

typedef struct rgbLED
{
  char name[SIZE_NAME];
  DigitalPin *red;
  DigitalPin *green;
  DigitalPin *blue;
} Tricolor;

typedef struct traffic
{
  char name[SIZE_NAME];
  DigitalPin *red;
  DigitalPin *orange;
  DigitalPin *green;
} TrafficControl;
  // ^ virtual traffic light

typedef struct reg
{
  char name[SIZE_NAME];
  DigitalPin *value; // value to be pushed
  DigitalPin *push;
    // ^ pushes the value from the 'left'
    // in the hidden memory
  DigitalPin *send;
    // ^ sends the hidden memory
    // to the visible memory

  // ^^ behavior implemented via
  // DigitalPin.onChange() and _digitalChangeValue()

  int size; // in bits. one real int per virtual bit

  int *output; // visible/displayed memory
  int *input; // hidden memory
    // ^ malloc'ed arrays

  Bool allVisible;
    // option for the user to give to shiftRegister()
    // to make _threadView() display the hidden memory too
  
  int (*printer)(int *in, int *out, int size);
    // optional custom displayer
    // can be provided by the user via shiftRegister()
} ShiftRegister;

typedef struct spy
{
  char name[SIZE_NAME];
  int *pointer;
  int (*printer)(int value);
} Spy;
  //| allows real-time display of the value
  //| of some user variable's value,
  //| with optional custom printer

#define SYMCASE(sym) case sym: printf(#sym); break
  // ^ to be used with enumerations, a switch, and Spy.printer:
  /*
    enum {
      Foo
      Bar
      Blah
    };
    int myCustomSpyPrinter(int value){
      switch(value) {
        SYMCASE(Foo);
        SYMCASE(Bar);
        SYMCASE(Blah);
        default: return -1;
      }
    }
  */

typedef struct staticMessage {
  char message[SIZE_LINE];
} StaticMessage;
  //| static messages, to be printed as is by _threadView()
  //| (not related to the notion of static memory)

typedef struct printable {
  void *object;
  int (*printer)(void *object);
} Printable;
  //| the *object will be a *Button, *Diode, *Spy, etc
  //| its printer will be _printButton(), _printDiode(), etc.
  //| cf view.c, esp _addToDisplayed()




//| ======================================
//| global, static variables and functions

//| ==== main.c
  extern pthread_t threadView;
    // ^ used in errors.c
  extern pthread_t threadUserCode;
    // ^ used to send USR1 to the corresponding thread
    // to simulate the interruptions
  extern pthread_t threadListener;

  void main(void);
  void _launchThreads(void);
  void _noReturnNeededKBEvent(Bool yes);

//| ==== user.c
  extern Arduino ardu;
     // ^ the simulated arduino
  extern Diode diodes[BIGN];
  extern int diodeCount;
  extern Button buttons[BIGN];
  extern int buttonCount;
  extern Tricolor rgbLEDs[BIGN];
  extern int rgbLEDCount;
  extern TrafficControl traffics[BIGN];
  extern int trafficCount;
  extern ShiftRegister registers[BIGN];
  extern int registerCount;
  extern Spy spies[BIGN];
  extern int spiesCount;

  void *_threadUserCode(void *_);

  //| functions to be called from init()
  void arduino(ArduinoType type);
  void diode(int pinIx, char *name);
  void button(int pinIx, char *name, char key);
  void rgbLED(int rIx, int gIx, int bIx, char *name);
  void traffic(int rIx, int oIx, int gIx, char *name);
  void shiftRegister(
    int valIx, int pushIx, int sendIx, char *name, int size,
    int (*printer)(int *in, int *out, int), Bool allVisible
  );
  void digitalDisplay(int valIx, int pushIx, int sendIx, char *name);
  void spy(int *pointer, char *name, int (*printer)(int val));
  void staticMessage(char *content);
  void separator(char c);

  //| internal stuff
  void _defineInterrupt(int pinIx, int interrIx);
  Bool _isValidDigital(int pinIx);
  void _checkValidDigital(int pinIx, char *fn);
  void _registerPush(void *reg, DigitalPin *pin, int oldVal);
  void _registerSend(void *reg, DigitalPin *pin, int oldVal);

  void _checkPinModeSet(DigitalPin *pin, char *fn);

  //| functions to be called from setup() or loop()
  int delay(int ms);
  void pinMode(int pinIx, PinMode mode);
  void attachInterrupt(
    int interrIx, void (*interrFun)(void), InterruptMode mode);
  void digitalWrite(int pinIx, int value);
  int digitalRead(int pinIx);
  void analogWrite(int pinIx, int value);
  int digitalPinToInterrupt(int pinIx);

//| ==== view.c
  extern Queue displayed;
  extern char *termColors[8];
  extern int winRowSize;
  extern int winColSize;

  void *_threadView(void *_);
  void _addToDisplayed(void *object, int (*printer)(void *o));
  void _printView(void);
  int _printIsInterrupted(int isInterrupted);
  void _state2Str(DigitalPin *pin, char *str);

  //| various specific Printable.printers (cf _addToDisplayed()):
  int _printDiode(Diode *diode);
  int _printButton(Button *button);
  int _printTricolor(Tricolor *rgbLED);
  int _printTrafficControl(TrafficControl *traffic);
  int _printShiftRegister(ShiftRegister * reg);
  int _printDigitalDisplay(int *input, int *output, int size);
  int _printSpy(Spy *spy);
  int _printStaticMessage(StaticMessage *msg);
  int _printSeparator(char *cpointer);

//| ==== events.c
  void *_threadListener(void *_);

  #define SWITCH 2
    //| ^ used as `newValue` argument for _digitalChangeValue()
    //| has the obvious result of setting the pin value to (oldValue == 0).
  void _digitalChangeValue(DigitalPin *pin, int newValue, int fromListener);
  void _addInterrupt(DigitalPin *pin);
  void _interruptSignalHandler(int _);
    

//| ==== tools.c
  void pushInQueue(Link *link, Queue *queue);
  void copyList(int *xs, int *into, int size);
  void printList(int *xs, int size);
  void shiftList(int *xs, int size, int val);
  int countLines(char *str);

//| ==== errors.c
  void _killThreadView(void);
  void _fatalError(char *type, char *fn, char *message);

#endif // FILE_ASIM // total wrapper
