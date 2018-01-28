#ifndef FILE_ASIM
#define FILE_ASIM

#define BIGN 50
#define SIZE_NAME 10
#define DISPLAY_FREQ 50*1000

// constants
#define LOW 0
#define HIGH 1

#define UNO 0


// types

typedef int Bool;

typedef enum {
  OUTPUT,
  INPUT,
  INTERRUPT,
  MODE_NONE
} PinMode;
#define INPUT_PULLUP INPUT

  // ^ since not relevant to the simulator

typedef struct digitalPin {
  PinMode mode;
  int value; // HIGH or LOW
  Bool isAnalog;
  Bool canInterrupt;
  Bool canAnalog;
} DigitalPin;

typedef struct arduino {
  int id;
  int val;
  DigitalPin pins[BIGN];
  int minDigital;
  int maxDigital;
} Arduino;

typedef struct diod
{
  char name[SIZE_NAME];
  DigitalPin *pin;
} Diod;

typedef struct button
{
  char name[SIZE_NAME];
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



// functions
void setSim(int type);

void diod(int pinIx, char *name);
void button(int pinIx, char *name, char key);
void diodRGB(int rIx, int gIx, int bIx, char *name);

void loop(void);



void delay(int ms);

void pinMode(int pinIx, PinMode mode);
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