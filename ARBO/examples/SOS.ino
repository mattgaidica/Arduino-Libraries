#include <Arbo.h>

Arbo arbo(13);

void setup()
{
}

void loop()
{
  arbo.dot(); arbo.dot(); arbo.dot();
  arbo.dash(); arbo.dash(); arbo.dash();
  arbo.dot(); arbo.dot(); arbo.dot();
  delay(3000);
}
