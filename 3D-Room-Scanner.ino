#include <SoftwareSerial.h>
#include "TFMini.h"
#include <Servo.h>

const int minServo1MicroDegree = 544;
const int minServo2MicroDegree = 1500;

int servo1MicroDegree;
int servo2MicroDegree;

Servo servo1;
Servo servo2;
int DegreeServo1;
int DegreeServo2;

int stop = 1;                               // 0 = Start script, 1 = Stop script

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
 
void GetRange(int x_cord, int y_cord){
  int distance = 0;
 
  getTFminiData(&distance);
  while (!distance)
  {
    getTFminiData(&distance);
    if (distance)
    {
      // Serial.println(distance);

      Serial.print(distance);
      Serial.print(",");
      Serial.print(x_cord);
      Serial.print(",");
      Serial.println(y_cord);


      // Serial.print("cm\t");
    }
  }  
}
 
void setup()
{
  if (stop == 0) {
    servo1.attach(9);
    servo2.attach(10);
    Serial.begin(9600);
    Serial.begin(115200);                   //Initialize hardware serial port (serial debug port)

    while (!Serial);                        // wait for serial port to connect. Needed for native USB port only
    Serial.println ("Initializing...");
    SerialTFMini.begin(TFMINI_BAUDRATE);    //Initialize the data rate for the SoftwareSerial port
    tfmini.begin(&SerialTFMini);            //Initialize the TF Mini sensor
  } 
}
 
void loop() 
{
  // --- write ---
  servo2.write(100);
  servo1.write(0);
  delay((1000));

  if (!stop) {  
    for(int j = 100; j >= 30; j--) {
      servo2.write(j);
      servo1.write(0);
      delay((200));
      
      for(int i = 0; i <= 180; i++ ) {
        GetRange(j,i);
        servo1.write(i);
        delay((4));
      }
    }
    servo2.write(100);
    servo1.write(0);
    stop = 1;
  }

  // --- writeMicroseconds ---
  // int y_cord = 101;
  // int x_cord = 0;
  // servo2.writeMicroseconds(1500);
  // servo1.writeMicroseconds(544);
  // delay((1000));

  // if (!stop) {  
  //   for(int j = 1500; j >= 800; j -=5) {
  //     if (j < 800) {
  //       j = 800;
  //     }

  //     if (y_cord > 30) {
  //       y_cord--;
  //     }

  //     servo2.writeMicroseconds(j);
  //     servo1.writeMicroseconds(0);
  //     delay((200));
  //     x_cord = 0;
  //     for(int i = 544; i <= 2389; i +=5) {
  //       if (i > 2389) {
  //         i = 2389;
  //       }

  //       if (x_cord < 180) {
  //         x_cord++; 
  //       }

  //       GetRange(y_cord,x_cord);
  //       servo1.writeMicroseconds(i);
  //       delay((4));
  //     }
  //   }
  //   servo2.writeMicroseconds(1500);
  //   servo1.writeMicroseconds(544);
  //   stop = 1;
  // }


}