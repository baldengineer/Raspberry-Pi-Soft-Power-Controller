#include <wiringPi.h>

int main (void) {
  wiringPiSetup();
  pinMode (7, OUTPUT);
  digitalWrite(7, LOW); 
  delay(50);
  digitalWrite(7, HIGH); 
  delay(50);

  digitalWrite(7, LOW); 
  delay(50);
  digitalWrite(7, HIGH); 
  delay(50);
  digitalWrite(7, LOW); 
  delay(50);

  digitalWrite(7, HIGH); 
  delay(50);
  digitalWrite(7, LOW); 
  delay(50);
  
  digitalWrite(7, LOW); 
  delay (500);
  return 0 ;
}