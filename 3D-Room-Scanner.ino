#include <SoftwareSerial.h>
#include "TFMini.h"
#include <Servo.h>

const double minHorizontalServoMicroDegree = 544; // get data tomorrow
const double maxHorizontalServoMicroDegree = 2000; // get data tomorrow
const double minVerticalServoMicroDegree = 800; // get data tomorrow
const double maxVerticalServoMicroDegree = 1500; //get data tomorrow

double horizontalMicroDegree;
double verticalMicroDegree;
double horizontalDegree;
double verticalDegree;

Servo horizontalServo;
Servo verticalServo;

int startScript = 0; // 1 = Start script, 0 = Stop script

// Lidar port allocation
TFMini tfmini;
SoftwareSerial SerialTFMini(2, 0);

void getTFminiData(int* distance)
{
  static char i = 0;
  char j = 0;
  int checksum = 0;
  static int rx[9];

  if (SerialTFMini.available()) {
    rx[i] = SerialTFMini.read();

    if (rx[0] != 0x59) {
      i = 0;
    } 
    else if (i == 1 && rx[1] != 0x59) {
      i = 0;
    } 
    else if (i == 8) {
      for (j = 0; j < 8; j++) {
        checksum += rx[j];
      }

      if (rx[8] == (checksum % 256)) {
        *distance = rx[2] + rx[3] * 256;
      }

      i = 0;
    } 
    else {
      i++;
    }
  }
}
 
void GetRange(double horizontalDegree, double verticalDegree)
{
  int distance = 0;
  getTFminiData(&distance);

  while (!distance) {
    getTFminiData(&distance);

    if (distance) {
      Serial.print(distance);
      Serial.print(",");
      Serial.print(horizontalDegree);
      Serial.print(",");
      Serial.println(verticalDegree);
    }
  }  
}

void setup()
{
  if (stop == 0) {
    horizontalServo.attach(9);
    verticalServo.attach(10);
    Serial.begin(9600);
    Serial.begin(115200);                   //Initialize hardware serial port (serial debug port)

    while (!Serial);                        // wait for serial port to connect. Needed for native USB port only
    Serial.println("Initializing...");
    SerialTFMini.begin(TFMINI_BAUDRATE);    //Initialize the data rate for the SoftwareSerial port
    tfmini.begin(&SerialTFMini);            //Initialize the TF Mini sensor
  } 
}
 
void loop() 
{
  
  /* FIRST GET MICROSECONDS DATA TOMORROW
  if (startScript) {
    horizontalServo.write(0)
    delay(400)
    Serial.println("HORIZONTAL MIN MICROSECONDS = ",horizontalServo.readMicroSeconds());
    delay(100)
    horizontalServo.write(180)
    delay(400)
    Serial.println("HORIZONTAL MAX MICROSECONDS = ",horizontalServo.readMicroSeconds());
    delay(100)
    verticalServo.write(100)
    delay(400)
    Serial.println("VERTICAL MAX MICROSECONDS = ",verticalServo.readMicroSeconds());
    delay(100)
    verticalServo.write(30)
    delay(400)
    Serial.println("VERTICAL MIN MICROSECONDS = ",verticalServo.readMicroSeconds());
    delay(100)
  }
  */


  /*if (startScript) {  
  // --- Set starting positions ---
  verticalServo.writeMicroseconds(maxVerticalServoMicroDegree);
  horizontalServo.writeMicroseconds(minHorizontalServoMicroDegree);
  delay((1000));

  for(verticalMicroDegree = maxVerticalServoMicroDegree; verticalMicroDegree >= minVerticalServoMicroDegree; verticalMicroDegree -= 5) {
    verticalServo.writeMicroseconds(verticalMicroDegree);
    horizontalServo.writeMicroseconds(minHorizontalServoMicroDegree);
    delay((200));
    
    for(horizontalMicroDegree = minHorizontalServoMicroDegree; horizontalMicroDegree <= maxHorizontalServoMicroDegree; horizontalMicroDegree += 5) {
      horizontalDegree = (horizontalMicroDegree - minHorizontalServoMicroDegree) / 10;
      verticalDegree = (verticalMicroDegree - minVerticalServoMicroDegree) / 10;
      
      horizontalServo.writeMicroseconds(horizontalMicroDegree);
      
      delay((4));
      GetRange(horizontalDegree, verticalDegree);
      delay((2));
    }
  }
  
  startScript = 0;
  }*/
}