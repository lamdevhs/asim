#ifndef FILE_ASIM
#define FILE_ASIM

#include <pthread.h>
#include <stdio.h>
  // for using printf in SYMCASE macro

#define dbgprintf printf

#define BIGN 50
#define SIZE_NAME 100
#define SIZE_LINE 200
#define DISPLAY_FREQ 50*1000

// constants


#define UNO 0


// types

typedef int Bool;


typedef struct link
{
  void *content;
  struct link *next;
} Link;

typedef struct queue
{
  Link *in; // where elements are added
  Link *out; // where elements are treated
  int size;
} Queue;



typedef enum {
  OUTPUT,
  INPUT,
  INPUT_PULLUP, // will amount to setting HIGH as initial state
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

typedef struct digitalPin {
  PinMode mode;
  int value; // HIGH or LOW or 0-255
  Bool isAnalog; // is the current value analog?
  Bool canAnalog;
  Bool canInterrupt;
  int interrIx;
  InterruptMode interrMode;
  void (*interrFun)(void);

  void (*onChange)(void *arg, struct digitalPin *pin, int oldVal);
  void *onChangeArg;
} DigitalPin;


typedef struct ievent
{
  DigitalPin *pin;
  //struct ievent *next;
  Bool dead; // signals the event has been taken care of
} IEvent;



typedef struct arduino {
  int id;
  int val;
  DigitalPin pins[BIGN];
  int minDigital;
  int maxDigital;
  DigitalPin *interrupts[BIGN];
  Queue ieq;
  pthread_t loopThread;
  Bool interrupted;
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

typedef struct reg
{
  char name[SIZE_NAME];
  DigitalPin *value;
  DigitalPin *push;
  DigitalPin *send;
  int size;
  int *output;
  int *input;

  Bool help; // set to 1 if you want to display input too
  int (*printer)(struct reg * reg); // special display
} Register;

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

void setSim(int type);
void defineInterrupt(int pinIx, int interrId);

void diod(int pinIx, char *name);
void button(int pinIx, char *name, char key);
void diodRGB(int rIx, int gIx, int bIx, char *name);
void traffic(int rIx, int yIx, int gIx, char *name);
int mkRegister(int valIx, int pushIx, int sendIx, char *name, int size, int help);
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