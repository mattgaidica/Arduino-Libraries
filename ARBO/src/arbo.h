/*
  Arbo.h - Library for ARBO.
  Created by Matt Gaidica 20200103.
  Released into the public domain.
*/
#ifndef Arbo_h
#define Arbo_h

#include "Arduino.h"

class Arbo
{
  public:
    Arbo(int pin);
    void dot();
    void dash();
  private:
    int _pin;
};

#endif