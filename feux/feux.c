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
  {{6,5,4}, 20, 18, {34, 35, 36}},
  {{9,8,7}, 21, 19, {37, 38, 39}}
};
int urgence = 2; // pin du bouton d'urgence



// ------------ valeurs
volatile RouteIx routeOuverte = Largeur;
volatile EtatFeu etatRouteOuverte = Rouge; // feu des voitures
volatile int tempsRestant = 1; // en secondes
volatile int pietonVeutTraverser = 0;

volatile int mode = JOUR;

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

void pietonEvenement(){
  RouteIx ici = (digitalRead(routes[1].btnPieton) == HIGH);
    // ^ vaut 1 si le btnPieton de la route 1 est HIGH, 0 sinon
    // pour verifier quel bouton pieton a ete appuye
  
  if (routeOuverte != ici ||
    etatRouteOuverte != Vert) return;
  // sinon:
  // la route du piéton est bien ouverte (aux voitures)
  // et le feu des voitures est vert
  tempsRestant = min(tempsRestant, PIETON_ACCEL_VERT);
  pietonVeutTraverser = 1;
}

void initJour(){
  routeOuverte = Largeur;
  etatRouteOuverte = Rouge; // feu des voitures
  changerCirculation();
  tempsRestant = 1;
  pietonVeutTraverser = 0;
}


void urgenceEvenement(){
  mode = 1 - mode;
  if (mode == JOUR) initJour();
}

void afficherDureeFeuRouge(){
  
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

void modeNuit() {
  int i;
  for (i = 0; i)
}



void setup(void){
  int i, j;
  int departFeux = 4;
  int departPieton = 20;
  int departVoitures = 18;
  int departAffichage = 34;
  for (j = 0; j < NROUTES; j++) {
    Route *route = &routes[j];
    for (i = 0; i < NCOULEURS; i++) {
      // 4,5,6 pour route j = 0, 7,8,9 pour route 1
      //route->feux[i] = 4 + 3*j + i;
      pinMode(route->feux[i], OUTPUT);
    }
    //route->btnPieton = departPieton + j;
    //route->captVoiture = departVoitures + j;
    pinMode(route->btnPieton, INPUT);
    pinMode(route->captVoiture, INPUT);
    attachInterrupt(
      digitalPinToInterrupt(route->btnPieton),
      pietonEvenement, RISING);

    for (i = 0; i < 3; i++) {
      // 34,35,36 pour route j = 0,
      // 37,38,39 pour route 1
      //route->affichage[i] = 34 + 3*j + i;
      pinMode(route->affichage[i], OUTPUT);
    }
  }

  pinMode(urgence, INPUT);
  attachInterrupt(
    digitalPinToInterrupt(urgence),
    urgenceEvenement, RISING);
  prologue();
}


void loop(void){
  if (mode == JOUR) modeJour();
  else modeNuit();
}

#ifdef __ASIM__
#define EMPTY 0

void init(void){
  setSim(UNO);
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
  button(largeur.btnPieton, "pieton largeur", 'l');
  button(hauteur.btnPieton, "pieton hauteur", 'h');
  button(largeur.captVoiture, "capteur voitures largeur", 'v');
  button(hauteur.captVoiture, "capteur voitures hauteur", 'w');
  button(urgence, "urgence", 'u');
  digitalDisplay(largeur.affichage[0], largeur.affichage[1], largeur.affichage[2],
    "affichage attente largeur");
  digitalDisplay(hauteur.affichage[0], hauteur.affichage[1], hauteur.affichage[2],
    "affichage attente hauteur");
}


#endif

//---------------
