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

void main(){
  //sim = (Mem *)create_shared_memory(sizeof(Mem));
  sim = (Mem *)malloc(sizeof(Mem));
  strcpy(sim->b, "hiya");
  int n = fork();
  if (n == 0) {
    while (1) {
      printf("%s\n", sim->b);
      usleep(500*1000);
    }
  }
  else {
    int child = n;
    usleep(3000*1000);
    strcpy(sim->b, "loo");
    kill(child, SIGTSTP);
    kill(child, SIGCONT);
    while (1) {
      kill(child, SIGTSTP);
      usleep(2000*1000);
      kill(child, SIGCONT);
      usleep(2000*1000);
    }
  }
}