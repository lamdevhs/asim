#include <stdlib.h>
#include <unistd.h> // STDOUT_FILENO
#include <sys/ioctl.h> // ioctl, TIOCGWINSZ

#include "asim.h"

int winRowSize;
int winColSize;

void *_threadDisplay(void *_) {
  struct winsize winsize;
  while (1) {
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize);
    winRowSize = winsize.ws_row;
    winColSize = winsize.ws_col;
    _printDisplay();
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

void _printDisplay() { // (int row, int col){
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