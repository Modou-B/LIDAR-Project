#include <SoftwareSerial.h>
#include "TFMini.h"
#include <Servo.h>


Servo servo1;
Servo servo2;
int DegreeServo1;
int DegreeServo2;
int stop = 0;

TFMini tfmini;
SoftwareSerial SerialTFMini(2, 0);          //The only value that matters here is the first one, 2, Rx



void getTFminiData(int* distance)
{
  static char i = 0;
  char j = 0;
  int checksum = 0;
  static int rx[9];

  if (SerialTFMini.available())
  {
    rx[i] = SerialTFMini.read();
    if (rx[0] != 0x59) {
      i = 0;
    } else if (i == 1 && rx[1] != 0x59) {
      i = 0;
    } else if (i == 8) {
      for (j = 0; j < 8; j++) {
        checksum += rx[j];
      }

      if (rx[8] == (checksum % 256)) {
        *distance = rx[2] + rx[3] * 256;
      }

      i = 0;
    } else {
      i++;
    }
  }
}
 
void GetRange(){
  int distance = 0;
 
  getTFminiData(&distance);
  while (!distance)
  {
    getTFminiData(&distance);
    if (distance)
    {
      Serial.print(distance);
      Serial.print("cm\t");
    }
  }  
}
 
void setup()
{
// SERVO BGN 
servo1.attach(9);
servo2.attach(10);
Serial.begin(9600);
  
// SERVO END

// LIDAR BGN
   Serial.begin(115200);       //Initialize hardware serial port (serial debug port)
   while (!Serial);            // wait for serial port to connect. Needed for native USB port only
   Serial.println ("Initializing...");
   SerialTFMini.begin(TFMINI_BAUDRATE);    //Initialize the data rate for the SoftwareSerial port
   tfmini.begin(&SerialTFMini);            //Initialize the TF Mini sensor
// LIDAR END
}
 
void loop()
{
// SERVO BGN 
  servo2.write(100);
  servo1.write(0);
  delay((1000));

  if (!stop) {  
    for(int j = 100; j >= 30; j--) {
      servo2.write(j);
      servo1.write(0);
      delay((500));
      
      for(int i = 0; i <= 180; i++ ) {
        // LIDAR BGN
        GetRange();
        // LIDAR END
        servo1.write(i);
        delay((4));
      }
    }
    servo2.write(100);
    servo1.write(0);
    stop = 1;
  }
// SERVO END  
}