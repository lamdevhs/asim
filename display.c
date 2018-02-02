#include <stdlib.h>
#include <unistd.h> // STDOUT_FILENO
#include <sys/ioctl.h> // ioctl, TIOCGWINSZ

#include "asim.h"

void *threadDisplay(void *_) {
  while (1) {
    struct winsize winsize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize);
    printDisplay(winsize.ws_row, winsize.ws_col);
    usleep(DISPLAY_FREQ);
  }
}

void addToDisplayList(void *object, int (*printer)(void *o)){
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

void printDisplay(int row, int col){
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

  // // printing diods
  // printf("diods:\n"); curRow++;
  // for (i = 0; i < diodCount && curRow < row - 1; i++){
  //   printTAB;
  //   printDiod(&diods[i]);
  //   curRow++;
  // }

  // printing rgb diods
  printf("RGB diods:\n"); curRow++;
  for (i = 0; i < diodRGBCount && curRow < row - 1; i++){
    printTAB;
    printDiodRGB(&diodRGBs[i]);
    curRow++;
  }

  printf("registers:\n"); curRow++;
  for (i = 0; i < registerCount && curRow < row - 1; i++){
    printTAB;
    if (registers[i].printer != NULL) {
      curRow += registers[i].printer(&registers[i]);
    }
    else {
      printRegister(&registers[i]);
      curRow++;
    }
  }

  printf("Traffic diods:\n"); curRow++;
  for (i = 0; i < trafficCount && curRow < row - 1; i++){
    printTAB;
    printTraffic(&traffics[i]);
    curRow++;
  }

  // printing buttons
  printf("buttons:\n"); curRow++;
  for (i = 0; i < buttonCount && curRow < row - 1; i++){
    printTAB;
    printButton(&buttons[i]);
    curRow++;
  }

  // printing spied values
  printf("spied values:\n"); curRow++;
  for (i = 0; i < spiedValuesCount && curRow < row - 1; i++){
    printTAB;
    printSpied(&spiedValues[i]);
    curRow++;
  }

  //   //debug ********
  // printf("\n[[DEBUG]]\n"); curRow+=2;
  // int ls = printInterr();
  // curRow += ls;


  
  // fill screen
  for (; curRow < row - 1; curRow++){
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