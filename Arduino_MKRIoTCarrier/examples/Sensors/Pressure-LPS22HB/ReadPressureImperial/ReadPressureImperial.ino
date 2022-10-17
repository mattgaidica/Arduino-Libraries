/*
  LPS22HB - Read Pressure Imperial

  This example reads data from the on-board LPS22HB sensor of the
  Nano 33 BLE Sense and prints the pressure sensor value in imperial
  units to the Serial Monitor once a second.

  The circuit:
  - Arduino Nano 33 BLE Sense

  This example code is in the public domain.
*/

#include <Arduino_MKRIoTCarrier.h>
MKRIoTCarrier carrier;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  carrier.noCase();
  if (!carrier.begin()) {
    Serial.println("Failed to initialize!");
    while (1);
  }
}

void loop() {
  // Passing PSI to readPressure(...)
  // allows you to read the sensor values in imperial units
  float pressure = carrier.Pressure.readPressure(PSI);

  // print the sensor value
  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" psi");

  // print an empty line
  Serial.println();

  // wait 1 second to print again
  delay(1000);
}
