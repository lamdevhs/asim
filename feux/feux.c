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


#define NCOULEURS 3
#define NROUTES 2

#define PIETON_ACCEL_VERT 2
  // ^ a combien se reduit la duree du feu lorsqu'un pieton
  // appuie sur le bouton pour traverser

typedef struct route {
  int feux[NCOULEURS]; // Rouge, Orange Vert
  int btnPieton;
  int captVoiture;
  int affichage[3]; // 0 is value, 1 is push, 2 is send
} Route;



Route routes[NROUTES] = {
  {{10,9,8}, 2, 4, {28, 26, 24}},
  {{7,6,5}, 3, 11, {50, 48, 46}}
};
int urgence = 18; // pin du bouton d'urgence



// ------------ valeurs
volatile RouteIx routeOuverte = Largeur;
volatile EtatFeu etatRouteOuverte = Rouge; // feu des voitures
volatile int tempsRestant = 1; // en secondes
volatile int pietonVeutTraverser = 0;

int jour = 1;
volatile int changementMode = 0;

void changerFeux(RouteIx route, EtatFeu etat){
  int *feux = routes[route].feux;
  int i;
  for (i = 0; i < NCOULEURS; i++){
    digitalWrite(feux[i], LOW);
  }
  if (etat != Off) digitalWrite(feux[etat], HIGH);
}

void changerCirculation(){
  changerFeux(1 - routeOuverte, Rouge);
  changerFeux(routeOuverte, etatRouteOuverte);
}

#define FREQ 200

int prologue(){
  int couleur;
  for (couleur = 0; couleur < NCOULEURS; couleur++){
    changerFeux(Largeur, couleur);
    delay(FREQ);
    changerFeux(Largeur, Off);
    delay(FREQ);
    changerFeux(Hauteur, couleur);
    delay(FREQ);
    changerFeux(Hauteur, Off);
    delay(FREQ);
  }
}

void pietonEvenement(RouteIx ici){
  if (routeOuverte != ici ||
    etatRouteOuverte != Vert || !jour) return;
  // sinon:
  // on est en journee
  // la route du piéton est bien ouverte (aux voitures)
  // et le feu des voitures est vert
  tempsRestant = min(tempsRestant, PIETON_ACCEL_VERT);
  pietonVeutTraverser = 1;
}

void pietonEvH(){ pietonEvenement(Hauteur); }
void pietonEvL(){ pietonEvenement(Largeur); }

void initJour(){
  routeOuverte = Largeur;
  etatRouteOuverte = Rouge; // feu des voitures
  changerCirculation();
  tempsRestant = 1;
  pietonVeutTraverser = 0;
}


void urgenceEvenement(){
  changementMode = 1;
}



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
  {1,0,1,1,1,1,0,0}, // G
  {0,0,1,1,1,0,1,0}, // o
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


void afficherDureeFeuRouge(){
  Route *r = &routes[1 - routeOuverte];
  pushSeq(r->affichage[0],
    r->affichage[1],
    r->affichage[2], nums[tempsRestant / 10], 8);
  pushSeq(r->affichage[0],
    r->affichage[1],
    r->affichage[2], nums[tempsRestant % 10], 8);
}

void effacerAffichage(){
  int i;
  for (i = 0; i < NROUTES; i++){
    Route *r = &routes[i];
    pushSeq(r->affichage[0],
      r->affichage[1],
      r->affichage[2], nums[10], 8);
    pushSeq(r->affichage[0],
      r->affichage[1],
      r->affichage[2], nums[11], 8);
  }
}


void modeJour(){
  changerCirculation();
  if (tempsRestant != 0) {
    // il reste du credit pour l'etat en cours
    if (etatRouteOuverte == Vert)
      afficherDureeFeuRouge();

    delay(1000);
    positivePred(tempsRestant);
    return;
  }
  else if (etatRouteOuverte == Rouge) {
    // change de route ouverte:
    effacerAffichage();
    routeOuverte = 1 - routeOuverte;
    etatRouteOuverte = Vert;
    tempsRestant = 5;
  }
  else if (etatRouteOuverte == Vert) { // fin de vert
    // check cars:
    if (digitalRead(routes[1 - routeOuverte].captVoiture) == HIGH
      // ^ une voiture attend sur l'autre voie
      || pietonVeutTraverser) {
      etatRouteOuverte = Orange;
      tempsRestant = 1;
      pietonVeutTraverser = 0;
    }
    else // sinon, on reste vert pour encore au moins 1 sec
      tempsRestant = 1;
  }
  else if (etatRouteOuverte == Orange) { // fin de orange
    etatRouteOuverte = Rouge;
    tempsRestant = 1;
  }
}

void initNuit(){
  int i,j;
  effacerAffichage();
  for (j = 0; j < NROUTES; j++) {
    Route *route = &routes[j];
    for (i = 0; i < NCOULEURS; i++) {
      digitalWrite(route->feux[i], LOW);
    }
  }
}

#define FreqNuit 1000
void modeNuit() {
  digitalWrite(routes[Hauteur].feux[Orange], HIGH);
  digitalWrite(routes[Largeur].feux[Orange], HIGH);
  delay(FreqNuit);
  digitalWrite(routes[Hauteur].feux[Orange], LOW);
  digitalWrite(routes[Largeur].feux[Orange], LOW);
  delay(FreqNuit);
}



void setup(void){
  int i, j, k;
  int departFeux = 4;
  int departPieton = 20;
  int departVoitures = 18;
  int departAffichage = 34;
  for (j = 0; j < NROUTES; j++) {
    Route *route = &routes[j];
    for (i = 0; i < NCOULEURS; i++) {
      pinMode(route->feux[i], OUTPUT);
    }
    pinMode(route->btnPieton, INPUT);
    pinMode(route->captVoiture, INPUT);

    for (k = 0; k < 3; k++){
      pinMode(route->affichage[k], OUTPUT);
    }
  }

  attachInterrupt(
    digitalPinToInterrupt(routes[Hauteur].btnPieton),
    pietonEvH, RISING);
  attachInterrupt(
    digitalPinToInterrupt(routes[Largeur].btnPieton),
    pietonEvL, RISING);

  pinMode(urgence, INPUT);
  attachInterrupt(
    digitalPinToInterrupt(urgence), urgenceEvenement,
    FALLING);
  prologue();
}


void loop(void){
  if (changementMode) {
    changementMode = 0;
    jour = 1-jour;
    if (jour) initJour();
    else initNuit();
  }
  if (jour) modeJour();
  else modeNuit();
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
  digitalDisplay(largeur.affichage[0], largeur.affichage[1],
    largeur.affichage[2], "afficheur largeur");
  traffic(
    hauteur.feux[Rouge],
    EMPTY,
    hauteur.feux[Vert],
    "feux pieton largeur");
  button(largeur.btnPieton, "pieton largeur", 'l');
  button(largeur.captVoiture, "capteur voitures largeur", 'v');



  
  separator('-');
  traffic(
    hauteur.feux[Rouge],
    hauteur.feux[Orange],
    hauteur.feux[Vert],
    "feux hauteur");
  digitalDisplay(hauteur.affichage[0], hauteur.affichage[1],
    hauteur.affichage[2], "afficheur hauteur");
  traffic(
    largeur.feux[Rouge],
    EMPTY,
    largeur.feux[Vert],
    "feux pieton hauteur");
  button(hauteur.btnPieton, "pieton hauteur", 'h');
  button(hauteur.captVoiture, "capteur voitures hauteur", 'w');
  button(urgence, "urgence", 'u');

  separator('-');
  spy(&routeOuverte, "route ouverte", NULL);
  spy(&etatRouteOuverte, "etat route", NULL);
  spy(&tempsRestant, "temps restant", NULL);
  spy(&pietonVeutTraverser, "pieton veut traverser", NULL);
  
  separator('-');
  spy(&changementMode, "changementMode", NULL);
  spy(&jour, "jour", NULL);

}


#endif

//---------------
