#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // STDOUT_FILENO
#include <sys/ioctl.h> // ioctl, TIOCGWINSZ


#include "asim.h"

Queue displayed;
  //| ^ queue of Printable elements,
  //| filled with everything that is
  //| displayed continuously by _threadView().

char *termColors[8] = {
  "\x1B[31m",
  "\x1B[32m",
  "\x1B[33m",
  "\x1B[34m",
  "\x1B[35m",
  "\x1B[36m",
  "\x1B[37m",
  "\x1B[0m"
}; //| cf TermColor enum defined in the header

int winRowSize;
int winColSize;
  //| useful to completely fill the terminal
  //| global because _printSeparator() needs it

//| continuously print the state of the simulation to stdout
void *_threadView(void *_) {
  threadView = pthread_self();
  struct winsize winsize;
  while (1) {
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize);
    winRowSize = winsize.ws_row;
    winColSize = winsize.ws_col;
    _printView();
    usleep(DISPLAY_FREQ);
  }
}

//| add a printable object to the global var `displayed`
void _addToDisplayed(void *object, int (*printer)(void *o)){
  Printable *printable = (Printable *)malloc(sizeof(Printable));
  printable->object = object;
  printable->printer = printer;
  Link *link = (Link *)malloc(sizeof(Link));
  link->content = (void *)printable;
  link->next = NULL;
  pushInQueue(link, &displayed);
}


//| print the simulation state once
void _printView(void) {
  int curRow = 0;
    //| cursor row; used to know
    //| how many newlines to add at the end
    //| to fill the terminal entirely
  int i;
  
  printNL; curRow++;
  
  Link *link = displayed.out;
  Printable *printable;
  while(link != NULL) {
    printable = (Printable *)link->content;
    curRow += printable->printer(printable->object);
    link = link->next;
  }

  if (ardu.interrupted) printf("[[interruption]]\n");
  else printf("\n");
  curRow++;

  // fill up the remaining lines
  for (; curRow < winRowSize - 1; curRow++){
    printNL;
  }
}

//| Printable.printer for the Spy(ardu.interrupted)
//| cf arduino()
int _printIsInterrupted(int isInterrupted){
  printf(isInterrupted ? "interrupted" : "setup/loop");
  return 0;
}


void _state2Str(DigitalPin *pin, char *str){
  if (pin->value == LOW) {
    //| same representation, be it LOW-digital
    //| or 0-analog
    sprintf(str, "   ");
  }
  else if (pin->isAnalog) {
    sprintf(str, "%3d", pin->value);
  }
  else if (pin->value == HIGH){
    sprintf(str, "###");
  }
  else { //| this should never happen
    _fatalError("BUG", "_state2Str",
      "digital pin not set as analog output "
      "but with value neither HIGH nor LOW");
  }
}

int _printDiode(Diode *diode){
  char state[4];
  _state2Str(diode->pin, state);
  printf("diode\t[%s] %s", state, diode->name);
  printNL;
  return countLines(diode->name);
}

int _printButton(Button *button){
  char state[4];
  _state2Str(button->pin, state);
  printf("button\t[%s] %s\t(key %c) %s", state, button->name, button->key,
    button->isPressed ? "(pressed)" : "");
  printNL;
  return countLines(button->name);
}

int _printTricolor(Tricolor *rgbLED){
  char redState[4], greenState[4], blueState[4];
  _state2Str(rgbLED->red, redState);
  _state2Str(rgbLED->green, greenState);
  _state2Str(rgbLED->blue, blueState);

  printf("RGB\t[%s%s%s|%s%s%s|%s%s%s] %s",
    termColors[RED], redState, termColors[NONE],
    termColors[GREEN], greenState, termColors[NONE],
    termColors[BLUE], blueState, termColors[NONE],
    rgbLED->name);
  printNL;
  return countLines(rgbLED->name);
}

int _printTrafficControl(TrafficControl *traffic){
  char redState[4], greenState[4], orangeState[4];
  _state2Str(traffic->red, redState);
  _state2Str(traffic->green, greenState);
  _state2Str(traffic->orange, orangeState);

  printf("traffic\t[%s%s%s|%s%s%s|%s%s%s] %s",
    termColors[RED], redState, termColors[NONE],
    termColors[ORANGE], orangeState, termColors[NONE],
    termColors[GREEN], greenState, termColors[NONE],
    traffic->name);
  printNL;
  return countLines(traffic->name);
}

int _printShiftRegister(ShiftRegister * reg){
  printf("reg\t%s:\n", reg->name);
  if (reg->printer == NULL) {
    int lines = 1;
    if (reg->allVisible) {
      printf("( ");
      printList(reg->input, reg->size);
      printf(" )\n");
    }
    printf("# ");
    printList(reg->output, reg->size);
    printf(" #");
    
    printNL;
    return !!reg->allVisible + 1 + countLines(reg->name);
  }
  else {
    return countLines(reg->name)
      + reg->printer(reg->input, reg->output, reg->size);
  }
}

//| print a digital display in ascii art:
/*
# _    _  #
#|_|  |_| #
#|_|. |_|.#
*/
//| output[0 -> 7] represents the leftmost digit and its dot,
//| in this order:
/*
 _    0
|_|  561
|_|. 4327
*/
//| likewise for the second part of the 16-bit shift register
int _printDigitalDisplay(int *input, int *output, int size){
  // first line
  printf("# ");
  printf(output[0] ? "_" : " ");
  printf("    ");
  printf(output[0+8] ? "_" : " ");
  printf("  #");
  printNL;
  // line 2
  printf("#");
  printf(output[5] ? "|" : " ");
  printf(output[6] ? "_" : " ");
  printf(output[1] ? "|" : " ");
  printf("  ");
  printf(output[5+8] ? "|" : " ");
  printf(output[6+8] ? "_" : " ");
  printf(output[1+8] ? "|" : " ");
  printf(" #");
  printNL;
  // line 3
  printf("#");
  printf(output[4] ? "|" : " ");
  printf(output[3] ? "_" : " ");
  printf(output[2] ? "|" : " ");
  printf(output[7] ? "." : " ");
  printf(" ");
  printf(output[4+8] ? "|" : " ");
  printf(output[3+8] ? "_" : " ");
  printf(output[2+8] ? "|" : " ");
  printf(output[7+8] ? "." : " ");
  printf("#");
  printNL;
  return 3; //| number of lines printed
}

int _printSpy(Spy *spy){
  printf("spy\t%s = ", spy->name);
  if (spy->printer == NULL
    || spy->printer(*spy->pointer) == -1)
    //| ^ custom printer is NULL or failed
    //| so we merely print the value as int:
    printf("%d", *spy->pointer);
  printNL;
  return countLines(spy->name);
}



int _printStaticMessage(StaticMessage *msg){
  printf("%s\n", msg->message);
  return countLines(msg->message);
}


int _printSeparator(char *cpointer){
  int i;
  for (i = 0; i < winColSize - 1; ++i)
    printf("%c", *cpointer);
  printNL;
  return 1;
}