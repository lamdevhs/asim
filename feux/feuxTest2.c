#include <stdio.h>
#ifdef __ASIM__
  #include "../src/asim.h"

#endif

#ifndef min
#define min(a, b) ((a < b) ? a : b)
#endif

#define positivePred(a) (a = (a <= 0) ? 0 : a - 1)


typedef enum etatFeu {
  Rouge,
  Orange,
  Vert,
  Off
} EtatFeu ;

typedef enum routeIx {
  Largeur, // gauche-droite
  Hauteur // haut-bas
} RouteIx;


#define JOUR 1
#define NUIT 0

#define NCOULEURS 3
#define NROUTES 2

#define PIETON_ACCEL_VERT 1
  // ^ a combien se reduit la duree du feu lorsqu'un pieton
  // appuie sur le bouton pour traverser

typedef struct route {
  int feux[NCOULEURS]; // Rouge, Orange Vert
  int btnPieton;
  int captVoiture;
  int affichage[3]; // 0 is value, 1 is push, 2 is send
} Route;



Route routes[NROUTES] = {
  {{10,9,8}, 2, 11, {34, 35, 36}},
  {{7,6,5}, 3, 4, {37, 38, 39}}
};
int urgence = 2; // pin du bouton d'urgence





void changerFeux(RouteIx route, EtatFeu etat){
  int *feux = routes[route].feux;
  int i;
  for (i = 0; i < NCOULEURS; i++){
    digitalWrite(feux[i], LOW);
  }
  if (etat != Off) digitalWrite(feux[etat], HIGH);
}



#ifdef __ASIM__
#define EMPTY 0

void init(void){
  arduino(MEGA);
  Route largeur = routes[Largeur];
  Route hauteur = routes[Hauteur];
  traffic(
    largeur.feux[Rouge],
    largeur.feux[Orange],
    largeur.feux[Vert],
    "feux largeur");
  traffic(
    hauteur.feux[Rouge],
    hauteur.feux[Orange],
    hauteur.feux[Vert],
    "feux hauteur");
  traffic(
    largeur.feux[Rouge],
    EMPTY,
    largeur.feux[Vert],
    "feux pieton hauteur");
  traffic(
    hauteur.feux[Rouge],
    EMPTY,
    hauteur.feux[Vert],
    "feux pieton largeur");
  button(hauteur.btnPieton, "pieton hauteur", 'h');
  button(largeur.btnPieton, "pieton largeur", 'l');
  button(hauteur.captVoiture, "capteur hauteur", 'v');
  button(largeur.captVoiture, "capteur largeur", 'w');
}


#endif
volatile int val = 0;
//---------------
void pietonEv(RouteIx route){
  digitalWrite(routes[route].feux[Vert], 
    1 - digitalRead(routes[route].feux[Vert]));
}

 void largeurPietonEv(){
  pietonEv(Largeur);
 }

 void hauteurPietonEv(){
  pietonEv(Hauteur);
 }


void setup(void){
  int i, j;
  for (j = 0; j < NROUTES; j++) {
    Route *route = &routes[j];
    for (i = 0; i < NCOULEURS; i++) {
      pinMode(route->feux[i], OUTPUT);
    }
    pinMode(route->btnPieton, INPUT);
    pinMode(route->captVoiture, INPUT);
  }
  attachInterrupt(digitalPinToInterrupt(routes[Hauteur].btnPieton),
    hauteurPietonEv, RISING);
  attachInterrupt(digitalPinToInterrupt(routes[Largeur].btnPieton),
    largeurPietonEv, RISING);
}
#define FREQ 500
void blink(int pin, int n){
  int i;
  for (i = 0; i < n; i++){
    digitalWrite(pin, HIGH);
    delay(FREQ);
    digitalWrite(pin, LOW);
    delay(FREQ);
  }
}

void loop(void){
  int i, j;
  for (j = 0; j < NROUTES; j++) {
    Route *route = &routes[j];
    digitalWrite(route->feux[Vert],
      digitalRead(route->captVoiture));
  }
}

