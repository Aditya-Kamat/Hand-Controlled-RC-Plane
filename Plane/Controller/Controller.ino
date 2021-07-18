#include <Wire.h> // for I2C communication
#include <nRF24L01.h>
#include <SPI.h>
#include <RF24.h>

RF24 radio(7,8);

byte address[6] = "1Node";

int pot = A4;
int type = 7;

long accelX, accelY; // raw accelerometer values
float gForceX, gForceY; // values in terms of acceleration due to gravity
byte angX, angY, angZ; // angle from respective axes
byte mode;
byte throttle;

//////////////////////////////////////////// Prototype Functions /////////////////////////////////////////////////////

void setupMPU();                         // begins I2C communication
void recordAccelRegs();                  // takes values from accelerometer
void processAccelValues();               // turn raw values into usable ones
void gToAng(float gForce, byte *ang);    // converts processed data to angles
void setupTransmitter();                 // sets up transmitter
void controlMotor(byte* throttle);       // controls motor speed

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() 
{
  Serial.begin(9600);
  Wire.begin();
  pinMode(type, INPUT);
  setupMPU();
  setupTransmitter();
}

void loop() 
{
  recordAccelRegs();
  processAccelValues();
  
  if(digitalRead(type) == HIGH)
    mode = 1;
  else
    mode = 0;
    
  switch(mode)
  {
     case 0: gToAng(gForceX, angX);
             gToAng(gForceY, angY);
     break;
     case 1: 
  }
  
  controlMotor(&throttle);
  
  byte values[5] = {angX, angY, angZ, throttle, mode}; //{roll, pitch, yaw, throttle, mode(0 for gyro, 1 for joystick)}
  byte x = radio.write(values, sizeof(values));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

void setupTransmitter()
{
  radio.begin();
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(address);
  radio.stopListening();
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
  gForceX = accelX / 16384.0; //based on datasheet
  gForceY = accelY / 16384.0; 
}

void gToAng(float gForce, byte *ang)
{
  if(gForce < 0)
  {
    gForce *= -100;
    int gForceInt = gForce;
    *ang = map(gForceInt, 0 , 90, 0,  40);
    Serial.print(90-*ang);
  }
  else
  {
    gForce *= 100;
    int gForceInt = gForce;
    *ang = map(gForceInt, 0 , 90, 90,  140);
    Serial.print(*ang);
  }
}

void controlMotor(byte* throttle)
{
  int val = analogRead(pot);
  *throttle = map(val, 0, 782, 0, 180);
}
