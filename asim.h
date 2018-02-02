#ifndef FILE_ASIM
#define FILE_ASIM

#include <pthread.h> // pthread_t
#include <stdio.h>
  // for using printf in SYMCASE macro

#define dbgprintf printf

#define BIGN 50
#define SIZE_NAME 100
#define SIZE_LINE 200
#define DISPLAY_FREQ 50*1000

typedef int Bool;

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
  pthread_t loopThread;
    // ^ used to send it a pthread signal
  Bool interrupted;
    // ^ is loopThread currently treating an interruption?
} Arduino;

typedef struct diod
{
  char name[SIZE_NAME];
  DigitalPin *pin;
} Diod;

typedef struct button
{
  char name[SIZE_NAME];
  Bool isPressed;
  DigitalPin *pin;
  char key;
    // ^ keyboard key to press
    // to change the button state
} Button;

typedef struct diodRGB
{
  char name[SIZE_NAME];
  DigitalPin *red;
  DigitalPin *green;
  DigitalPin *blue;
} DiodRGB;

typedef struct traffic
{
  char name[SIZE_NAME];
  DigitalPin *red;
  DigitalPin *yellow;
  DigitalPin *green;
} Traffic;
  // ^ virtual traffic light

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
  
  int (*printer)(struct reg * reg); // special displayer
} Register;
  // ^ shift register

typedef struct spied
{
  char name[SIZE_NAME];
  int *pointer;
  int (*printer)(int value);

} Spied;
#define SYMCASE(sym) case sym: printf(#sym); break
 // ^ to be used with enumerations, a switch, and Spied.printer

typedef struct printable {
  void *object;
  int (*printer)(void *object);
} Printable;

typedef struct staticMessage {
  char message[SIZE_LINE];
} StaticMessage;


void pushInQueue(Link *link, Queue *queue);

// functions
void main(void);
void setup(void);
void loop(void);
void init(void);

void arduino(ArduinoType type);
void defineInterrupt(int pinIx, int interrId);

void diod(int pinIx, char *name);
void button(int pinIx, char *name, char key);
void diodRGB(int rIx, int gIx, int bIx, char *name);
void traffic(int rIx, int yIx, int gIx, char *name);
int shiftRegister(int valIx, int pushIx, int sendIx, char *name, int size, Bool allVisible);
  void digitalDisplay(int valIx, int pushIx, int sendIx, char *name);
int spy(int *pointer, char *name);
  void spyWithPrinter(int *pointer, char *name, int (*printer)(int value));

void addToDisplayList(void *object, int (*printer)(void *o));

int delay(int ms);

void pinMode(int pinIx, PinMode mode);

void attachInterrupt(int interrIx, void (*interrFun)(void), InterruptMode mode);
void digitalWrite(int pinIx, int value);
int digitalRead(int pinIx);
void analogWrite(int pinIx, int value);
int digitalPinToInterrupt(int pinIx);


// internals
Bool checkDigital(int pinIx);
void setDisplayName(char *dest, char *src);

void launchThreads(void);
  void *threadDisplay(void *_);
    void printDisplay(int row, int col);
      void state2Str(DigitalPin *pin, char* str);
      
      int printDiod(void *diod);
      void printButton(Button *button);
      void printTraffic(Traffic *traffic);
      void printRegister(Register * reg);
        int printDigitalDisplay(Register * reg);
      
      void printSpied(Spied *spied);

      int printStaticMessage(void *o);

      void printDiodRGB(DiodRGB *diodRGB);
        int getMainColor(int r, int g, int b);
        int getMix(int mainColor, int r, int g, int b);

        
  
  void *threadLoop(void *_);
  /*
  void *threadInterruptions(void *_);
  */
    void digitalChange(DigitalPin *pin, int newValue, int fromListener);
    #define SWITCH 2
    void addInterrupt(DigitalPin *pin);
    void iEventHandler(int _);

  void *threadListener(void *_);


void registerPush(void *reg, DigitalPin *pin, int oldVal);
void registerSend(void *reg, DigitalPin *pin, int oldVal);

void shiftList(int *xs, int size, int val);
void copyList(int *xs, int *into, int size);
void printList(int *xs, int size);

#define NB_ENABLE 0
#define NB_DISABLE 1
void nonblock(int state);
Bool kbhit(void);

#define printNL printf("\n")
#define printTAB printf("\t")
#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)


// globals
extern Arduino sim;
   // ^ the simulated arduino
extern Diod diods[BIGN];
extern int diodCount;
extern Button buttons[BIGN];
extern int buttonCount;
extern DiodRGB diodRGBs[BIGN];
extern int diodRGBCount;
extern Traffic traffics[BIGN];
extern int trafficCount;
extern Register registers[BIGN];
extern int registerCount;
extern Spied spiedValues[BIGN];
extern int spiedValuesCount;
extern Queue displayed;




#endif // FILE_ASIM // wrapper




// TODO:
// make so pinMode checks the expected value of the mode based on what kind
// of object the pin is bound to via init();
// * allow diodRGB to only have two or even one color, without
// having to give dummy pin numbers
// use static to try to avoid polluting the namespace of client code
// what happens if analog is sent to an INPUT connected to an interrupt?



// cases:
  // calling write/read/analog for wrong types
  // defining a button/diod/etc for wrong type
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