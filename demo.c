// compilation:
// gcc -pthread -D __ASIM__ sim.c <filename>.c

#ifdef __ASIM__
  #include "sim.h"

extern Arduino sim;
#endif

// -------------- pins
enum {
  blinkingDiod, // 0
  yellowDiod,

  // interrupts
  incrementButton, // 2
  yellowButton, // 3
  
  
  pullupButton,
  normalButton,

  // register
  rgValue,
  rgPush,
  rgSave,

  // traffic light
  traffRed,
  traffOrange,
  traffGreen,

  // diod RBG
  rgbRed,
  rgbGreen,
  rgbBlue
};

volatile int xvar = 0;
volatile int yellowState = 0;

#ifdef __ASIM__

enum {
  zero,
  one,
  two,
  three,
  four,
  five,
  six,
  seven,
  eight,
  nine
};

int digitName(int value){
  switch(value % 10) {
    SYMCASE(zero);
    SYMCASE(one);
    SYMCASE(two);
    SYMCASE(three);
    SYMCASE(four);
    SYMCASE(five);
    SYMCASE(six);
    SYMCASE(seven);
    SYMCASE(eight);
    SYMCASE(nine);
  }
  return 1;
}

void init(void){
  setSim(UNO);
  diod(blinkingDiod, "blinking diod");
  diod(yellowDiod, "yellow (with some imagination)");
  button(yellowButton,
    "this button is yellow too! guess what it does!", 'y');
  button(pullupButton, "this button is pullup", 'p');
  button(normalButton, "this button is *not* pullup", 'n');
  button(incrementButton,
    "increment xvar by 10 via interruption, on FALLING", 'i');
  diodRGB(rgbRed, rgbGreen, rgbBlue, "multicolor diod");
  traffic(traffRed, traffOrange, traffGreen, "roadlights");
  digitalDisplay(rgValue, rgPush, rgSave,
    "value of xvar (mod 100), updated each second");
  spy(&xvar, "real time value of xvar");
  spyWithPrinter(&xvar, "real time text value of xvar (mod 10)",
    digitName);
}
#endif

int nums[][8] = {
  {1,1,1,1,1,1,0,0}, // 0
  {0,1,1,0,0,0,0,0}, // 1
  {1,1,0,1,1,0,1,0}, // 2
  {1,1,1,1,0,0,1,0}, // 3
  {0,1,1,0,0,1,1,0}, // 4
  {1,0,1,1,0,1,1,0}, // 5
  {1,0,1,1,1,1,1,0}, // 6
  {1,1,1,0,0,0,0,0}, // 7
  {1,1,1,1,1,1,1,0}, // 8
  {1,1,1,1,0,1,1,0}, // 9
};

void pushSeq(int rg1, int rg2, int rg3, int *seq, int size){
  digitalWrite(rg2, LOW);
  int i;
  for (i = size - 1; i >= 0; i--){
    digitalWrite(rg1, seq[i]);
    digitalWrite(rg2, HIGH);
    digitalWrite(rg2, LOW);
  }
  digitalWrite(rg3, LOW);
  digitalWrite(rg3, HIGH);
  digitalWrite(rg3, LOW);
}

void printXvar(){
  // push unit digit
  pushSeq(rgValue, rgPush, rgSave, nums[xvar % 10], 8);

  // push tens digit
  pushSeq(rgValue, rgPush, rgSave, nums[(xvar % 100) / 10], 8);
}


void incrementXVar(){
  xvar += 10;
  printXvar();
}

void changeYellowDiod(){
  yellowState = 1 - yellowState;
  digitalWrite(yellowDiod, yellowState);
}

void setup(void){
  pinMode(blinkingDiod, OUTPUT);
  pinMode(yellowDiod, OUTPUT);
  pinMode(yellowButton, INPUT);
  attachInterrupt(digitalPinToInterrupt(yellowButton),
    changeYellowDiod, RISING);
  pinMode(pullupButton, INPUT_PULLUP);
  pinMode(normalButton, INPUT);
  pinMode(incrementButton, INPUT);
  attachInterrupt(digitalPinToInterrupt(incrementButton),
    incrementXVar, FALLING);

  pinMode(rgbRed, OUTPUT);
  pinMode(rgbGreen, OUTPUT);
  pinMode(rgbBlue, OUTPUT);

  pinMode(traffRed, OUTPUT);
  pinMode(traffOrange, OUTPUT);
  pinMode(traffGreen, OUTPUT);

  pinMode(rgValue, OUTPUT);
  pinMode(rgPush, OUTPUT);
  pinMode(rgSave, OUTPUT);
}

int rgbState = 0;
int analogVal = 0;
void changeRGB(){
  analogWrite(rgbState + rgbRed, analogVal);
  digitalWrite(((rgbState + 1) % 3) + rgbRed, LOW);
  digitalWrite(((rgbState + 2) % 3) + rgbRed, LOW);
  rgbState = (rgbState + 1) % 3;
  analogVal = (analogVal + 10) % 256;
}

int traffState = 0;
void changeTraffic(){
  digitalWrite(traffState + traffRed, HIGH);
  digitalWrite(((traffState + 1) % 3) + traffRed, LOW);
  digitalWrite(((traffState + 2) % 3) + traffRed, LOW);
  traffState = (traffState + 1) % 3;
}


int diodState = 0;
void blinkDiod(){
  digitalWrite(blinkingDiod, ++diodState % 2);
}


void loop(void){
  changeRGB();
  blinkDiod();
  printXvar();
  changeTraffic();
  delay(1000);
  xvar++;
}




