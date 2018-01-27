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

  // ^ since not relevant to the simulator

typedef struct digitalPin {
  PinMode mode;
  int value; // HIGH or LOW
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



// functions
void setSim(int type);
void diod(int pinIx, char *name);
void loop(void);



void delay(int ms);

void pinMode(int pinIx, PinMode mode);
void digitalWrite(int pinIx, int value);

// internals
Bool checkDigital(int pinIx);
void setDisplayName(char *dest, char *src);

void launchThreads(void);
  void *threadDisplay(void *_);
    void printDisplay(int row, int col);
    void printDiod(Diod *diod);
    char digital2Char(int value);
  void *threadLoop(void *_);

#define printNL printf("\n")

#endif // FILE_ASIM // wrapper