/*
  Arbo.cpp - Library for flashing Arbo code.
  Created by David A. Mellis, November 2, 2007.
  Released into the public domain.
*/

#include "Arduino.h"
#include "Arbo.h"

Arbo::Arbo(int pin)
{
  pinMode(pin, OUTPUT);
  _pin = pin;
}

void Arbo::dot()
{
  digitalWrite(_pin, HIGH);
  delay(250);
  digitalWrite(_pin, LOW);
  delay(250);  
}

void Arbo::dash()
{
  digitalWrite(_pin, HIGH);
  delay(1000);
  digitalWrite(_pin, LOW);
  delay(250);
}