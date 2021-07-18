#include <Servo.h>

int pot = A0;
int motor = 8;

Servo Servo1;

void setup() 
{
  Serial.begin(9600);
  Servo1.attach(motor);
  delay(1000);
}

void loop() 
{
  int val = analogRead(pot);
  int ang = map(val, 0, 1023, 1000, 2000);
  Serial.println(val);
  Serial.println(ang);
  Servo1.write(ang);
}
