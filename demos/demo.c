// compilation:
// gcc -pthread -D __ASIM__ <filename>.c path/to/src/*.c

#ifdef __ASIM__
  #include "../src/asim.h"
#endif


// -------------- pins
enum {
  blinkingDiode, // 0
  ___, // 1
  
  pullupButton, // 2
  normalButton, // 3

  // traffic light
  traffGreen,
  traffOrange,
  traffRed,
  

  // diod RBG
  r, g, b,

  // simple register
  rgVal, rgPush, rgSend,

  // digital display
  ddVal, ddPush, ddSend
};


enum {
  Open,
  Closing,
  Closed
};

volatile int spiedInt = 0;
volatile int trafficState = Open;

void setTraffic(){
  digitalWrite(traffGreen + trafficState, HIGH);
  digitalWrite(traffGreen + ((trafficState + 1) % 3), LOW);
  digitalWrite(traffGreen + ((trafficState + 2) % 3), LOW);
}

#ifdef __ASIM__

int trafficStatePrinter(int value){
  switch(value) {
    SYMCASE(Open);
    SYMCASE(Closing);
    SYMCASE(Closed);
    default: return -1;
  }
}

void init(void){
  arduino(MEGA);

  separator(' ');
  staticMessage("A blinking diod:");
  diode(blinkingDiode, "blinking");

  separator(' ');
  staticMessage("An RGB LED, lit using analog values:");
  rgbLED(r, g, b, "multicolor light");

  separator(' ');
  staticMessage("traffic lights:");
  traffic(traffRed, traffOrange, traffGreen, "road 66");

  separator(' ');
  staticMessage("a shift register, showing both the visible (second line)\n"
    "and invisible, 'temporary' memory:");
  shiftRegister(rgVal, rgPush, rgSend, "8-bit register", 8, NULL, 1);

  separator(' ');
  staticMessage("a digital display controlled by a shift register:");
  digitalDisplay(ddVal, ddPush, ddSend, "2-digit digital display");

  separator(' ');
  staticMessage("values of user-defined integers can be displayed "
    "in real time:");
  spy(&spiedInt, "spied int", NULL);

  separator(' ');
  staticMessage("a custom printer function can be provided by the user\n"
    "like here with this variable holding the traffic lights' state:");
  spy(&trafficState, "traffic state", trafficStatePrinter);

  separator(' ');
  staticMessage("Buttons are simulated using keyboard keys:");
  button(normalButton, "normal", 'n');
  button(pullupButton, "pull-up", 'p');
  staticMessage("The normal button increments by 10 the spied int.");
  staticMessage("The pullup button changes the state of the traffic lights.");

  separator(' ');
  staticMessage("Note: if nothing of the above is colored, it means somehow\n"
    "the code to write with colors does not work for your terminal.");
}
#endif

void incrementSpiedInt(){
  spiedInt += 10;
}

void changeTraffic(){
  trafficState = (trafficState + 1) % 3;
  setTraffic();
}

void setup(void){
  pinMode(blinkingDiode, OUTPUT);
  pinMode(pullupButton, INPUT_PULLUP);
  pinMode(normalButton, INPUT);
  // diode rgb
  pinMode(r, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(b, OUTPUT);
  // traffic
  pinMode(traffRed, OUTPUT);
  pinMode(traffOrange, OUTPUT);
  pinMode(traffGreen, OUTPUT);
  // register
  pinMode(rgPush, OUTPUT);
  pinMode(rgVal, OUTPUT);
  pinMode(rgSend, OUTPUT);
  // digital display
  pinMode(ddVal, OUTPUT);
  pinMode(ddPush, OUTPUT);
  pinMode(ddSend, OUTPUT);

  pinMode(ddSend, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(normalButton),
    incrementSpiedInt, RISING);
  attachInterrupt(digitalPinToInterrupt(pullupButton),
    changeTraffic, FALLING);

  setTraffic();
}

#define FRQ 200
void blink(int pin){
  
  
}

void analogBlink(int pin){
  analogWrite(pin, 42);
  delay(FRQ);
  analogWrite(pin, 0);
  delay(FRQ);
}

int nums[10][8] = {
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




void loop(void){
  pushSeq(rgVal, rgPush, rgSend, nums[spiedInt % 10], 8);
  pushSeq(ddVal, ddPush, ddSend, nums[spiedInt % 10], 8);
  pushSeq(ddVal, ddPush, ddSend, nums[(spiedInt % 100) / 10], 8);

  digitalWrite(blinkingDiode, HIGH);
  analogWrite(r, 42);
  delay(FRQ);
  digitalWrite(blinkingDiode, LOW);
  analogWrite(r, 0);
  analogWrite(g, 42);
  delay(FRQ);
  analogWrite(g, 0);
  analogWrite(b, 42);
  delay(FRQ);
  analogWrite(b, 0);

  spiedInt++;
  
}
