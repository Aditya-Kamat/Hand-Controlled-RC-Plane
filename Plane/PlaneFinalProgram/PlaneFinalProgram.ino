#include <Wire.h> // for I2C communication
#include <Servo.h>

int motor_Roll_1 = 9;
int motor_Roll_2 = 6;
int motor_Pitch = 5;
int motor_Yaw = 4;

int pot = A4;
int prop = 3;
int val, throttle;

Servo ESC;

long accelX, accelY; // raw accelerometer values
float gForceX, gForceY; // values in terms of acceleration due to gravity
float angX, angY, angZ; // angle from respective axes

Servo ServoRoll_1;
Servo ServoRoll_2;
Servo ServoPitch;
Servo ServoYaw;

//////////////////////////////////////////// Prototype Functions /////////////////////////////////////////////////////

void setupMPU();                         //begins I2C communication
void setupServos();                      //attaches all servos and esc
void recordAccelRegs();                  // takes values from accelerometer
void processAccelValues();               // turn raw values into usable ones
void gToAng(float gForce, float ang);    // converts processed data to angles
void setPitch();                         // sets the pitch of plane
void setYawAndRoll();                    // set the yaw and roll for turning of plane
void controlMotor();                     // sets the speed of propeller

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() 
{
  Serial.begin(9600);
  setupServos();
  Wire.begin();
  setupMPU();
}

void loop() 
{
  recordAccelRegs();
  processAccelValues();
  setPitch();
  setYawAndRoll();
  controlMotor();
  delay(100);
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

void setupServos()
{
  ServoRoll_1.attach(motor_Roll_1);
  ServoRoll_2.attach(motor_Roll_2);
  ServoPitch.attach(motor_Pitch);
  ServoYaw.attach(motor_Yaw);
  ESC.attach(prop);
}

void controlMotor()
{
  val = analogRead(pot);
  throttle = map(val, 0, 782, 0, 180);
  Serial.print("Throttle is at: ");
  Serial.println(throttle);
  ESC.write(throttle);
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

void gToAng(float gForce, float ang, Servo Serv)
{
  if(gForce < 0)
  {
    gForce *= -100;
    int gForceInt = gForce;
    ang = map(gForceInt, 0 , 90, 0,  40);
    Serv.write(90-ang);
    Serial.print(90-ang);
  }
  else
  {
    gForce *= 100;
    int gForceInt = gForce;
    ang = map(gForce, 0 , 90, 90,  140);
    Serv.write(ang);
    Serial.print(ang);
  }  
  delay(10);
}

void setPitch()
{
  Serial.println("Pitch Angle: ");
  gToAng(gForceY, angY, ServoPitch);
}

void setYawAndRoll()
{
  Serial.println("Roll Right Angle: ");
  gToAng(gForceX, angX, ServoRoll_1);
  Serial.println("Roll Left Angle: ");
  gToAng(gForceX, -1*angX, ServoRoll_2);
  Serial.println("Yaw Angle: ");
  gToAng(gForceX, -1*angX, ServoYaw);
}
