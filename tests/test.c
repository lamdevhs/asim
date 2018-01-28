#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <termios.h>
 
#define NB_ENABLE 0
#define NB_DISABLE 1

// Let us create a global variable to change it in threads
int g = 0;
char c = '-';

void nonblock(int state)
{
    struct termios ttystate;
 
    //get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);
 
    if (state==NB_ENABLE)
    {
        //turn off canonical mode
        ttystate.c_lflag &= ~ICANON;
        //minimum of number input read.
        ttystate.c_cc[VMIN] = 1;
    }
    else if (state==NB_DISABLE)
    {
        //turn on canonical mode
        ttystate.c_lflag |= ICANON;
    }
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
 
}

int kbhit(void)
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

// The function to be executed by all threads
void *myThreadFun(void *vargp)
{
    // Store the value argument passed to this thread
    int myid = (int)vargp;
 
    // Let us create a static variable to observe its changes
    static int s = 0;
 
    // Change static and global variables
    //++s; ++g;
    // Print the argument, static and global variables
    int k = 0;
    char red[] = "R";
    char bl[] = " ";
    char kkk[] = "B";
    char *now;
    char *prev;
    while (true) {
        //printf("Thread ID: %d, Static: %d, Global: %d\n", myid, ++s, ++g);
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

        //printf ("lines %d\n", w.ws_row);
        //printf ("columns %d\n", w.ws_col);
        int i, j;
        printf("\n");
        for (i = 1; i < w.ws_row - 1; i++){
            printf(" ");
            for (j = 1; j < w.ws_col; j++){
                printf("a");
            }
            printf("\n");
        }
        prev = now; if (prev != bl) {
            prev = kkk;
        }
        now = k % 2 == 0 ? bl: red;
        printf("%d   [%s  %s]\n", g, prev, now);
        k++;
        //printf("%c", c);
        usleep(100000);
    }
}

void *myThreadFun2(void *vargp)
{
    // Store the value argument passed to this thread
    int myid = (int)vargp;
 
    // Let us create a static variable to observe its changes
    static int s = 0;
    while (true) {g++; sleep(1);}
    // Change static and global variables
    ++s; ++g;
    //int a = kbhit();
    //printf("%d", a);
    while (true) {
        while(kbhit() == 0){}
        c = fgetc(stdin);
        // Print the argument, static and global variables
        //printf("%c", c);printf("%c", c);printf("%c", c);printf("%c", c);printf("%c", c);
    }
    printf("Thread ID: %d, Static: %d, Global: %d\n", myid, ++s, ++g);
}

void *myThreadFun3(void *vargp)
{
    // Store the value argument passed to this thread
    int myid = (int)vargp;
    //int a = kbhit();
    //printf("%d", a);
    while (true) {
        while(kbhit() == 0){}
        c = fgetc(stdin);
        if (c == 'a') {
            g = 0;
        }
        else {
            g += 10000;
        }
        // Print the argument, static and global variables
        //printf("%c", c);printf("%c", c);printf("%c", c);printf("%c", c);printf("%c", c);
    }
    //printf("Thread ID: %d, Static: %d, Global: %d\n", myid, ++s, ++g);
}
 
int main()
{
    int i;
    pthread_t tid;
    char ss[] = "wawa";
    char cc[] = "blablabla";
    if (strlen(cc) > strlen(ss)){
        cc[strlen(ss)] = '\0';
    }
    strcpy(ss, cc);
    printf("%d   %s %c ", strlen("test"), ss);
    return;

    nonblock(NB_ENABLE);
    pthread_create(&tid, NULL, myThreadFun2, (void *)2);
    pthread_create(&tid, NULL, myThreadFun, (void *)1);
    pthread_create(&tid, NULL, myThreadFun3, (void *)3);
    //pthread_create(&tid, NULL, myThreadFun2, (void *)2);
    pthread_exit(NULL);
    return 0;
    // Let us create three threads
    for (i = 0; i < 3; i++)
        pthread_create(&tid, NULL, myThreadFun, (void *)i);
 
    
    return 0;
}