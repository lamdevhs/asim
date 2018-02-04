#ifndef FILE_ASIM
#define FILE_ASIM

#include <pthread.h> // pthread_t
#include <stdio.h>
  // for using printf in SYMCASE macro

#define dbgprintf printf

#define BIGN 50
#define SIZE_NAME 100
#define SIZE_LINE 200
#define STRSIZE(nb) (nb * sizeof(char))
#define DISPLAY_FREQ 50*1000

#define NWARN_MSG 25




typedef int Bool;

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

typedef enum {
  UNO,
  MEGA
} ArduinoType;

typedef enum {
  OUTPUT,
  INPUT,
  INPUT_PULLUP,
    // ^ will amount to setting HIGH as off state
  INTERRUPT,
  MODE_NONE
} PinMode;

typedef enum {
  LOW,
  HIGH,
  FALLING,
  RISING,
  CHANGE,
  NO_INTERR
} InterruptMode;

#define LOW 0
#define HIGH 1
  // ^ for clarity's sake.
  // obviously redundant since C
  // has no type safety

// -------- Queue
typedef struct link
{
  void *content;
  struct link *next;
} Link;

typedef struct queue
{
  Link *in; // where elements are added
  Link *out; // from where elements are treated/deleted
  int size;
} Queue;
  // FIFO link-based queue


typedef struct digitalPin {
  PinMode mode;
  int value; // HIGH or LOW or 0-255
  Bool isAnalog; // is the current value analog?
  Bool canAnalog; // can be used as analog output?
  Bool canInterrupt; // can be used as interrupt?
  int interrIx; // interrupt index in Arduino.interrupts
  InterruptMode interrMode;
  void (*interrFun)(void); // function to call upon interruption

  void (*onChange)(void *arg, struct digitalPin *pin, int oldVal);
    // ^ optional function to be called whenever the value changes
    // useful for the (virtual) shift registers
  void *onChangeArg; // argument for onChange
} DigitalPin;

typedef struct interruptEvent
{
  DigitalPin *pin; // pin that triggered the event
  Bool dead; // signals that the event has been taken care of
} IEvent;
  // interrupt event; mallocated


// --------- virtual objects, cf vobjects.c
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
  pthread_t userCodeThread;
    // ^ used to send it a pthread signal
    // to simulate the interruptions
  Bool interrupted;
    // ^ is userCodeThread currently treating an interruption?

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

typedef struct tricolor
{
  char name[SIZE_NAME];
  DigitalPin *red;
  DigitalPin *green;
  DigitalPin *blue;
} Tricolor;

typedef struct trafficControl
{
  char name[SIZE_NAME];
  DigitalPin *red;
  DigitalPin *orange;
  DigitalPin *green;
} TrafficControl;
  // ^ virtual trafficControl light

typedef struct reg
{
  char name[SIZE_NAME];
  DigitalPin *value; // value to be pushed
  DigitalPin *push;
    // pushes the value from the 'left'
    // in the hidden memory
  DigitalPin *send;
    // ^ sends the hidden memory
    // to the visible memory

  // ^^ implemented via DigitalPin.onChange

  int size; // in bits

  int *output; // visible/displayed memory
  int *input; // hidden memory
    // ^ malloc'ed arrays

  Bool allVisible;
    // set to 1 so the hidden memory
    // also be visible
  
  int (*printer)(int *in, int *out, int size);
    // optional custom displayer
    // can be provided by the user
} ShiftRegister;

typedef struct spy
{
  char name[SIZE_NAME];
  int *pointer;
  int (*printer)(int value);

} Spy;
#define SYMCASE(sym) case sym: printf(#sym); break
 // ^ to be used with enumerations, a switch, and Spy.printer

typedef struct printable {
  void *object;
  int (*printer)(void *object);
} Printable;

typedef struct staticMessage {
  char message[SIZE_LINE];
} StaticMessage;


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

// main.c
  // this file mostly contains the stuff
  // that i couldn't classify...
  void main(void);
  void _launchThreads(void);


// asim.c
  // ^ functions simulating those used
  // in code that targets arduinos
  int delay(int ms);
  void pinMode(int pinIx, PinMode mode);
  void attachInterrupt(int interrIx, void (*interrFun)(void), InterruptMode mode);
  void digitalWrite(int pinIx, int value);
  int digitalRead(int pinIx);
  void analogWrite(int pinIx, int value);
  int digitalPinToInterrupt(int pinIx);

// vobjects.c
  // functions to set up / define virtual objects
  void arduino(ArduinoType type);

  void diode(int pinIx, char *name);
  void button(int pinIx, char *name, char key);
  void tricolor(int rIx, int gIx, int bIx, char *name);
  void trafficControl(int rIx, int oIx, int gIx, char *name);
  void shiftRegister(
    int valIx, int pushIx, int sendIx, char *name, int size,
    int (*printer)(int *in, int *out, int size), Bool allVisible
  );
    void digitalDisplay(int valIx, int pushIx, int sendIx, char *name);
  void spy(int *pointer, char *name, int (*printer)(int val));
  void staticMessage(char *message);
   // void spyWithPrinter(int *pointer, char *name, int (*printer)(int value));

  // individual printers (displayers) for those objects:

  void separation(char c);
  int _printSeparation(char *cpointer);
  
  int _printDiode(Diode *diode);
  int _printButton(Button *button);
  int _printTrafficControl(TrafficControl *trafficControl);
  int _printTricolor(Tricolor *tricolor);
  int _printShiftRegister(ShiftRegister * reg);
    int _printDigitalDisplay(int *input, int *output, int size);
  int _printSpy(Spy *spy);

  int _printStaticMessage(StaticMessage *msg);

  void printTricolor(Tricolor *tricolor);
    int getMainColor(int r, int g, int b);
    int getMix(int mainColor, int r, int g, int b);


  Bool _isValidDigital(int pinIx);
  void _defineInterrupt(int pinIx, int interrId);
  void _state2Str(DigitalPin *pin, char* str);

// display.c
  extern int winRowSize;
  extern int winColSize;
  void *_threadView(void *_);
  void _printView(); //(int row, int col);
  void _addToDisplayed(void *object, int (*printer)(void *o));

// tools.c
  void pushInQueue(Link *link, Queue *queue);
  void strcpyUpTo(char *dest, char *src, int sup);
  int countLines(char *str);
        
  
  void *_threadUserCode(void *_);
  /*
  void *threadInterruptions(void *_);
  */
    void _digitalChangeValue(DigitalPin *pin, int newValue, int fromListener);
    #define SWITCH 2
    void addInterrupt(DigitalPin *pin);
    void _interruptSignalHandler(int _);

  void *_threadListener(void *_);


void registerPush(void *reg, DigitalPin *pin, int oldVal);
void registerSend(void *reg, DigitalPin *pin, int oldVal);

void shiftList(int *xs, int size, int val);
void copyList(int *xs, int *into, int size);
void printList(int *xs, int size);

// #define NB_ENABLE 0
// #define NB_DISABLE 1
void _noReturnNeededKBEvent(int state);

#define printNL printf("\n")
#define printTAB printf("\t")
#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)


// globals
extern Arduino sim;
   // ^ the simulated arduino
extern Diode diodes[BIGN];
extern int diodeCount;
extern Button buttons[BIGN];
extern int buttonCount;
extern Tricolor tricolors[BIGN];
extern int tricolorCount;
extern TrafficControl trafficControls[BIGN];
extern int trafficControlCount;
extern ShiftRegister registers[BIGN];
extern int registerCount;
extern Spy spies[BIGN];
extern int spiesCount;
extern Queue displayed;

extern char *termColors[8];

extern int winRowSize;
extern int winColSize;

extern pthread_t threadView;
  

void _killThreadView(void);
void _fatalError(char *type, char *fn, char *message);

#endif // FILE_ASIM // wrapper




// TODO:
// make so pinMode checks the expected value of the mode based on what kind
// of object the pin is bound to via init();
// * allow tricolor to only have two or even one color, without
// having to give dummy pin numbers
// use static to try to avoid polluting the namespace of client code
// what happens if analog is sent to an INPUT connected to an interrupt?



// cases:
  // calling write/read/analog for wrong types
  // defining a button/diode/etc for wrong type
  // handle pullup thing better
  // delay when sim.interrupted

// check:
  // what happens when you try to set the value
  // of something connected to a button
  // (if it's INPUT, in theory, it changes the pullup option value)

/* ideas:
  * pause button (but can still use buttons to modify state,
  and therefore execute interruptions)
  * freeze button (stop everything)
  * print to display anything
  * print static titles in between virtual objects
  * print value of some variable in real time (with pointer)
    * maybe permit to give an array of symbolic strings
    * to replace the dry int version
    * esp for booleans
  * change display to reflect any order the use wants
  * allows adding separations in the display
  * make the display of all virtual objects generic:
  typedef struct visible {
    void *object;
    void (*printer)(void *object);
  } Visible;

  * use malloc instead of static sizes of displayed text
  * the functions used to create virtual objects must NOT work
    outside of the init function!
*/


/* security:
   * test for char *name pointers to be null

*/

/*
- user.c
- event.c
- error.c
- view.c
- tools.c
- main.c (includes variables and threads)
*/