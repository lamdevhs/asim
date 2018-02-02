#ifndef FILE_ASIM
#define FILE_ASIM

#define BIGN 50
#define SIZE_NAME 10
#define DISPLAY_FREQ 50*1000

// constants


#define UNO 0


// types

typedef int Bool;

typedef enum {
  OUTPUT,
  INPUT,
  INPUT_PULLUP, // will amount to setting HIGH as initial state
  INTERRUPT,
  MODE_NONE
} PinMode;

typedef enum {
  LOW,
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
  InterruptMode interruptMode;
  void (*interrFun)(void);
} DigitalPin;

typedef struct arduino {
  int id;
  int val;
  DigitalPin pins[BIGN];
  DigitalPin *canInterrupt[BIGN];
  int minDigital;
  int maxDigital;
  DigitalPin *interrupts[BIGN];
  int nextInterrupt;
  int freeInterrupt;
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

typedef struct simulation
{
  Arduino ard;
  Diod diods[BIGN];
  int diodCount;
  Button buttons[BIGN];
  int buttonCount;
  DiodRGB diodRGBs[BIGN];
  int diodRGBCount;
} Simulation;



// functions
void setSim(int type);

void diod(int pinIx, char *name);
void button(int pinIx, char *name, char key);
void diodRGB(int rIx, int gIx, int bIx, char *name);

void loop(void);

void delay(int ms);

void pinMode(int pinIx, PinMode mode);

void attachInterrupt(int interrIx, void (*interrFun)(void), InterruptMode mode);
void digitalWrite(int pinIx, int value);
int digitalRead(int pinIx);
void analogWrite(int pinIx, int value);


// internals
Bool checkDigital(int pinIx);
void setDisplayName(char *dest, char *src);

void launchThreads(void);
  void *threadDisplay(void *_);
    void printDisplay(int row, int col);
      void printDiod(Diod *diod);
      void printButton(Button *button);
      void printDiodRGB(DiodRGB *diodRGB);
      int getMainColor(int r, int g, int b);
      int getMix(int mainColor, int r, int g, int b);
      void state2Str(DigitalPin *pin, char* str);
  
  void *threadLoop(void *_);
  
  void *threadInterruptions(void *_);
    void digitalChange(DigitalPin *pin, int newValue);
    #define SWITCH 2
    void addInterrupt(DigitalPin *pin);

  void *threadListener(void *_);


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
// cases:
  // calling write/read/analog for wrong types
  // defining a button/diod/etc for wrong type
  // 