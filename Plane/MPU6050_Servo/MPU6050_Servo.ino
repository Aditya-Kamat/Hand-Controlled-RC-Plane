#include <Wire.h> // for I2C communication
#include <Servo.h>

int motorX = 8;
int motorY = 7;
int rst = 6;
long accelX, accelY;
float gForceX, gForceY;
float angX, angY;

Servo ServoX;
Servo ServoY;

void setupMCU();
void recordAccelRegs();
void processAccelValues();
void printValues();
void gToAng(float gForce, float ang);

void setup() 
{
  Serial.begin(9600);
  ServoX.attach(motorX);
  ServoY.attach(motorY);
  pinMode(rst, OUTPUT);
  digitalWrite(rst, LOW);
  Wire.begin();
  setupMPU();
}

void loop() 
{
  recordAccelRegs();
  processAccelValues();
  gToAng(gForceX, angX, ServoX);
  gToAng(gForceY, angY, ServoY);
  
  if(Serial.available() > 0)
  {
    digitalWrite(rst, HIGH);
    delay(200);
    digitalWrite(rst, LOW);    
  }
}

void setupMPU()
{
  Wire.beginTransmission(0b1101000); //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet sec. 9.2)
  Wire.write(0x6B); //Accessing the register 6B - Power Management (Sec. 4.28)
  Wire.write(0b00000000); //Setting SLEEP register to 0. (Required; see Note on p. 9)
  Wire.endTransmission();  
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5) 
  Wire.write(0b00000000); //Setting the accel to +/- 2g
  Wire.endTransmission(); 
}

void recordAccelRegs() 
{
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,6); //Request Accel Registers (3B - 40)
  while(Wire.available() < 6);
  accelX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  accelY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  processAccelValues();
}

void processAccelValues()
{
  gForceX = accelX / 16384.0;
  gForceY = accelY / 16384.0; 
}

void gToAng(float gForce, float ang, Servo Serv)
{
  if(gForce < 0)
  {
    gForce *= -100;
    int gForceInt = gForce;
    ang = map(gForceInt, 0 , 100, 0,  50);
    Serv.write(90-ang);
    Serial.print(90-ang);
    Serial.println();
  }
  else
  {
    gForce *= 100;
    int gForceInt = gForce;
    ang = map(gForce, 0 , 100, 90,  140);
    Serv.write(ang);
    Serial.print(ang);
    Serial.println();
  }  
}
