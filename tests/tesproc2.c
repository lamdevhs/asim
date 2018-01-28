#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h> // usleep, fork

#include <stdlib.h>
#include <sys/mman.h>

void* create_shared_memory(size_t size) {
  // Our memory buffer will be readable and writable:
  int protection = PROT_READ | PROT_WRITE;

  // The buffer will be shared (meaning other processes can access it), but
  // anonymous (meaning third-party processes cannot obtain an address for it),
  // so only this process and its children will be able to use it:
  int visibility = MAP_ANONYMOUS | MAP_SHARED;

  // The remaining parameters to `mmap()` are not important for this use case,
  // but the manpage for `mmap` explains their purpose.
  return mmap(NULL, size, protection, visibility, 0, 0);
}

typedef struct mem {
  int a;
  char b[100];
} Mem;

Mem *sim;

void hand(int signum){
  printf("received %d\n", getpid());
  int a = 0;
  int c = getchar();
  //while (1) {a++;}
}

void main(){
  //sim = (Mem *)create_shared_memory(sizeof(Mem));
  sim = (Mem *)malloc(sizeof(Mem));
  strcpy(sim->b, "hiya");

  signal(SIGTERM, hand);
  printf("%d\n", getpid());
  int j = 0;
  while (1){printf("%d\n", j++);usleep(100*1000);}
  int n = fork();
  if (n == 0) {
    usleep(1000*1000);
    printf("%d\n", getpid());
  }
  else {
    n = fork();
    if (n == 0) {
      usleep(1000*1000);
      printf("%d\n", getpid());
    }
    else {
      n = fork();
      if (n == 0) {
        usleep(1000*1000);
        printf("%d\n", getpid());
      }
    }
  }
  if (n == 0) while(1) {printf("%d\n", getpid()); usleep(1000*1000);}
}