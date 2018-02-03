#include <unistd.h>
#include "asim.h"

// functions simulating those used in codes that target arduinos


int delay(int ms){
  while(ms) {
    int ans = usleep(10*1000);
    if (ans != 0) {
      // some error happened, probably signal
      continue;
    }
    else ms -= 10;
  }
}



void pinMode(int pinIx, PinMode mode){
  if (!_isValidDigital(pinIx)) {
    return; // ERROR
  }
  DigitalPin *pin = &sim.pins[pinIx];
  if (pin->mode != MODE_NONE){
    // WARNING?
  }
  pin->mode = mode;

  if (mode == INPUT_PULLUP){
    pin->value = 1 - pin->value;
  }
}

void attachInterrupt(int interrIx, void (*interrFun)(void), InterruptMode mode){
  // check validity of mode
  if (interrIx < 0 || BIGN <= interrIx) return; // ERROR
  DigitalPin *pin = sim.interrupts[interrIx];
  if (pin == NULL) return; // ERROR
  if (!pin->canInterrupt) return; // BUG!
  pin->interrFun = interrFun;
  pin->interrMode = mode;
}


void digitalWrite(int pinIx, int value){
  if (!_isValidDigital(pinIx) || sim.pins[pinIx].mode != OUTPUT) {
    return; // ERROR? or input pullup stuff? TODO
  }
  if (value != LOW && value != HIGH) return; // ERROR? or default value? TODO
  digitalChange(&sim.pins[pinIx], value, 0);
}


int digitalRead(int pinIx){
  if (!_isValidDigital(pinIx)) {
    return -1;
  }
  PinMode mode = sim.pins[pinIx].mode;
  if (mode != INPUT && mode != INPUT_PULLUP){
    return -1; //error?
  }
  return sim.pins[pinIx].value;
}

void analogWrite(int pinIx, int value){
  if (!_isValidDigital(pinIx) || sim.pins[pinIx].mode != OUTPUT) {
    return; // ERROR
  }
  DigitalPin *pin = &sim.pins[pinIx];
  if (!pin->canAnalog) {
    return; // ERROR
  }
  pin->value = min(max(0, value), 255);
  pin->isAnalog = 1;
}

int digitalPinToInterrupt(int pinIx){
  if (!_isValidDigital(pinIx)) return -1; //ERROR
  DigitalPin *pin = &sim.pins[pinIx];
  if (!pin->canInterrupt) return -1; //ERROR
  return pin->interrIx;
}









