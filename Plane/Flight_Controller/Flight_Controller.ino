#include <Servo.h>
#include <nRF24L01.h>
#include <SPI.h>
#include <RF24.h>

RF24 radio(7,8);

byte address[6] = "1Node";

int motor_Roll_1 = 9;
int motor_Roll_2 = 6;
int motor_Pitch = 5;
int motor_Yaw = 4;
int prop = 3;

Servo ESC;
Servo ServoRoll_1;
Servo ServoRoll_2;
Servo ServoPitch;
Servo ServoYaw;

byte values[5]; //{roll, pitch, yaw, throttle, mode(0 for gyro, 1 for joystick)}  

//////////////////////////////////////////// Prototype Functions /////////////////////////////////////////////////////

void setupServos();                      // attaches all servos and esc
void controlG();                         // controls all movements (gyroscope)
void controlJ();                         // controls all movements (joystick)
void setupReceiver();                    // sets up receiver

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() 
{
  setupServos();
  setupReceiver();
}

void loop() 
{
  if(radio.available())
    radio.read(values, sizeof(values));
    
  switch(values[4])
  {
    case 0: controlG();
    break;
    case 1: controlJ();
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setupServos()
{
  ServoRoll_1.attach(motor_Roll_1);
  ServoRoll_2.attach(motor_Roll_2);
  ServoPitch.attach(motor_Pitch);
  ServoYaw.attach(motor_Yaw);
  ESC.attach(prop);
}

void setupReceiver()
{
  radio.begin();
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(0, address);
  radio.startListening();
}

void controlG()
{
  ESC.write(values[3]);
  ServoRoll_1.write(values[0]);
  ServoRoll_2.write(-1*values[0]);
  ServoPitch.write(values[1]);
  ServoYaw.write(values[0]);
}

void controlJ()
{
  ESC.write(values[3]);
  ServoRoll_1.write(values[0]);
  ServoRoll_2.write(-1*values[0]);
  ServoPitch.write(values[1]);
  ServoYaw.write(values[2]);
}
