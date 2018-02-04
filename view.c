#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // STDOUT_FILENO
#include <sys/ioctl.h> // ioctl, TIOCGWINSZ


#include "asim.h"

Queue displayed;

char *termColors[8] = {
  "\x1B[31m",
  "\x1B[32m",
  "\x1B[33m",
  "\x1B[34m",
  "\x1B[35m",
  "\x1B[36m",
  "\x1B[37m",
  "\x1B[0m"
};

int winRowSize;
int winColSize;



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

void _addToDisplayed(void *object, int (*printer)(void *o)){
  Printable *printable = (Printable *)malloc(sizeof(Printable));
  printable->object = object;
  printable->printer = printer;
  Link *link = (Link *)malloc(sizeof(Link));
  link->content = (void *)printable;
  link->next = NULL;
  pushInQueue(link, &displayed);
}





// *************************
// int printInterr();
// int printEv(IEvent *ie);

void _printView() { // (int row, int col){
  int curRow = 0;
  int curCol = 0;
  int i;
  
  printNL; curRow++;
  
  Link *link = displayed.out;
  Printable *printable;
  while(link != NULL) {
    printable = (Printable *)link->content;
    curRow += printable->printer(printable->object);
    link = link->next;
  }

  if (sim.interrupted) printf("[[interruption]]\n");
  else printf("\n");
  curRow++;

  // // printing diodes
  // printf("diodes:\n"); curRow++;
  // for (i = 0; i < diodeCount && curRow < row - 1; i++){
  //   printTAB;
  //   _printDiode(&diodes[i]);
  //   curRow++;
  // }

  // // printing rgb diodes
  // printf("RGB diodes:\n"); curRow++;
  // for (i = 0; i < tricolorCount && curRow < row - 1; i++){
  //   printTAB;
  //   printTricolor(&tricolors[i]);
  //   curRow++;
  // }

  // printf("registers:\n"); curRow++;
  // for (i = 0; i < registerCount && curRow < row - 1; i++){
  //   printTAB;
  //   if (registers[i].printer != NULL) {
  //     curRow += registers[i].printer(&registers[i]);
  //   }
  //   else {
  //     printRegister(&registers[i]);
  //     curRow++;
  //   }
  // }

  // printf("TrafficControl diodes:\n"); curRow++;
  // for (i = 0; i < trafficControlCount && curRow < row - 1; i++){
  //   printTAB;
  //   printTrafficControl(&trafficControls[i]);
  //   curRow++;
  // }

  // // printing buttons
  // printf("buttons:\n"); curRow++;
  // for (i = 0; i < buttonCount && curRow < row - 1; i++){
  //   printTAB;
  //   printButton(&buttons[i]);
  //   curRow++;
  // }

  // printing spy values
  // printf("spy values:\n"); curRow++;
  // for (i = 0; i < spiesCount && curRow < row - 1; i++){
  //   printTAB;
  //   printSpy(&spies[i]);
  //   curRow++;
  // }

  //   //debug ********
  // printf("\n[[DEBUG]]\n"); curRow+=2;
  // int ls = printInterr();
  // curRow += ls;


  
  // fill up the remaining lines
  for (; curRow < winRowSize - 1; curRow++){
    printNL;
  }
}






  // //DEBUG
  // int printInterr(){
  //   int ls = 0;
  //   printf("output\n"); ++ls;
  //   IEvent *ie = (IEvent *)sim.ieq.out->content;
  //   if (ie == NULL){
  //     printf("!!! NULL\n");
  //     return ++ls;
  //   }
  //   while(printEv(ie)){
  //     ++ls;
  //     ie = ie->next;
  //     if (ie == NULL){
  //       printf("NULL\n");
  //       return ++ls;
  //     }
  //   }
  //   return ++ls;
  // }

  // int printEv(Link *ieLink){
  //   IEvent *ie = (IEvent *)ieLink->content;
  //   printf("in: %d, out: %d, dead: %d, next is null: %d\n",
  //     ieLink == sim.ieq.in, ieLink == sim.ieq.out,
  //     ie->dead, ieLink->next == NULL);

  //   if (ie->next == NULL) return 0;
  //   else return 1;
  // }

  // // ---


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



int _printTricolor(Tricolor *tricolor){
  char redState[4], greenState[4], blueState[4];
  _state2Str(tricolor->red, redState);
  _state2Str(tricolor->green, greenState);
  _state2Str(tricolor->blue, blueState);

  printf("RGB\t[%s%s%s|%s%s%s|%s%s%s] %s",
    termColors[RED], redState, termColors[NONE],
    termColors[GREEN], greenState, termColors[NONE],
    termColors[BLUE], blueState, termColors[NONE],
    tricolor->name);
  printNL;
  return countLines(tricolor->name);
}



int _printTrafficControl(TrafficControl *trafficControl){
  char redState[4], greenState[4], orangeState[4];
  _state2Str(trafficControl->red, redState);
  _state2Str(trafficControl->green, greenState);
  _state2Str(trafficControl->orange, orangeState);

  printf("traffic\t[%s%s%s|%s%s%s|%s%s%s] %s",
    termColors[RED], redState, termColors[NONE],
    termColors[ORANGE], orangeState, termColors[NONE],
    termColors[GREEN], greenState, termColors[NONE],
    trafficControl->name);
  printNL;
  return countLines(trafficControl->name);
}



int _printShiftRegister(ShiftRegister * reg){
  printf("reg\t%s:\n", reg->name);
  if (reg->printer == NULL) {
    int lines = 1;
    if (reg->allVisible) {
      printf("((");
      printList(reg->input, reg->size);
      printf("))\n");
    }
    printf("//");
    printList(reg->output, reg->size);
    printf("//");
    
    printNL;
    return !!reg->allVisible + 1 + countLines(reg->name);
  }
  else {
    return reg->printer(reg->input, reg->output, reg->size);
  }
}



int _printDigitalDisplay(int *input, int *output, int size){
  // first line
  printf("# ");
  if (output[0]) printf("_");
  else printf(" ");
  printf("    ");
  if (output[0 + 8]) printf("_");
  else printf(" ");
  printf("  #");
  printNL;
  // line 2
  printf("#");
  if (output[5]) printf("|");
  else printf(" ");
  if (output[6]) printf("_");
  else printf(" ");
  if (output[1]) printf("|");
  else printf(" ");
  printf("  ");
  if (output[5+8]) printf("|");
  else printf(" ");
  if (output[6+8]) printf("_");
  else printf(" ");
  if (output[1+8]) printf("|");
  else printf(" ");
  printf(" #");
  printNL;
  // line 3
  printf("#");
  if (output[4]) printf("|");
  else printf(" ");
  if (output[3]) printf("_");
  else printf(" ");
  if (output[2]) printf("|");
  else printf(" ");
  if (output[7]) printf(".");
  else printf(" ");
  printf(" ");
  if (output[4+8]) printf("|");
  else printf(" ");
  if (output[3+8]) printf("_");
  else printf(" ");
  if (output[2+8]) printf("|");
  else printf(" ");
  if (output[7+8]) printf(".");
  else printf(" ");
  printf("#");
  printNL;

  return 3; // number of lines printed
}



int _printSpy(Spy *spy){
  printf("spy\t");
  if (spy->printer == NULL)
    printf("%s = %d", spy->name, *spy->pointer);
  else {
    printf("%s = ", spy->name);
    if (spy->printer(*spy->pointer) == -1){
      printf("%d", *spy->pointer);
    }
  }
  printNL;
  return countLines(spy->name);
  // }
  // else {
  //   //TODO check spy->pointer didn't get null
  //   int val = *spy->pointer;
  //   if (val >= 0 && val < spy->symbolLimit) {
  //     printf("%s = %s", spy->name, spy->symbols[val]);
  //   }
  // }
}

// void spyWithPrinter(int *pointer, char *name, int (*printer)(int val)){
//   int ix = spy(pointer, name);
//   if (ix < 0 || ix >= spiesCount) return; //FAIL/ERROR/BUG
//   if (printer == NULL) return; //ERROR
//   Spy *spy = &spies[ix];
//   int i, nextStart = 0;
//   // for (i = 0; i < symbolLimit; i++) {
//   //   nextStart = copyToSpace(spy->symbols[i], symbols + nextStart);
//   //   if (nextStart == -1) break;
//   // }
//   //spy->symbols = symbols;
//   // spy->symbolLimit = symbolLimit;
//   spy->printer = printer;
// }






void _state2Str(DigitalPin *pin, char *str){
  if (pin->value == LOW) {
    sprintf(str, "   ");
  }
  else if (pin->isAnalog) {
    sprintf(str, "%3d", pin->value);
  }
  else if (pin->value == HIGH){
    sprintf(str, "###");
  }
  else {
    sprintf(str, "???");
    // bug!
  }
}

















int _printStaticMessage(StaticMessage *msg){
  printf("%s\n", msg->message);
  return countLines(msg->message); // TODO depends on presence of \n's in the message
}


// void printTricolor(Tricolor *tricolor){
//   char redState[4], greenState[4], blueState[4];
//   _state2Str(tricolor->red, redState);
//   _state2Str(tricolor->green, greenState);
//   _state2Str(tricolor->blue, blueState);
  
//   printf("%s [%s %s %s] *%3s %5s %4s*",
//     tricolor->name,
//     redState, greenState, blueState,
//     (tricolor->red->value != 0) ? "RED" : "",
//     (tricolor->green->value != 0) ? "GREEN" : "",
//     (tricolor->blue->value != 0) ? "BLUE" : ""
//   );
//   printNL;
// }







int _printSeparation(char *cpointer){
  int i;
  for (i = 0; i < winColSize - 1; ++i)
    printf("%c", *cpointer);
  printNL;
  return 1;
}


