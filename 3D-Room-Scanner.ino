#include <SoftwareSerial.h>
#include "TFMini.h"
#include <Servo.h>

const double minHorizontalServoMicroDegree = 544; // get data tomorrow
const double maxHorizontalServoMicroDegree = 2400; // get data tomorrow
const double minVerticalServoMicroDegree = 853; // get data tomorrow
const double maxVerticalServoMicroDegree = 1575; //get data tomorrow

double horizontalMicroDegreeT;
double verticalMicroDegree;
double horizontalDegree;
double verticalDegree;

Servo horizontalServo;
Servo verticalServo;

int startScript = 0; // 1 = Start script, 0 = Stop script

// Lidar port allocation
TFMini tfmini;
SoftwareSerial SerialTFMini(2, 0);


void getPrizeLoop()
{
  double horizontalStep = (maxHorizontalServoMicroDegree - minHorizontalServoMicroDegree) / 180;
  double verticalStep = (maxVerticalServoMicroDegree - minVerticalServoMicroDegree) / 70;

  double horizontalInterval = horizontalStep / 2;
  double verticalInterval = verticalStep / 2;
  if (startScript == 1) {  
  // --- Set starting positions ---
  verticalServo.writeMicroseconds(maxVerticalServoMicroDegree);
  horizontalServo.writeMicroseconds(minHorizontalServoMicroDegree);
  delay((1000));

  for(double verticalMicroDegreeT = maxVerticalServoMicroDegree; verticalMicroDegreeT >= minVerticalServoMicroDegree; verticalMicroDegreeT -= verticalInterval) {
    verticalServo.writeMicroseconds(verticalMicroDegreeT);
    horizontalServo.writeMicroseconds(minHorizontalServoMicroDegree);
    delay((200));
    
    for(double horizontalMicroDegreeT = minHorizontalServoMicroDegree; horizontalMicroDegreeT <= maxHorizontalServoMicroDegree; horizontalMicroDegreeT += horizontalInterval) {
      horizontalDegree = (horizontalMicroDegreeT - minHorizontalServoMicroDegree) / horizontalStep;
      verticalDegree = (verticalMicroDegreeT - minVerticalServoMicroDegree) / verticalStep;
      
      horizontalServo.writeMicroseconds(horizontalMicroDegreeT);
      
      delay((4));
      GetRange(horizontalDegree, verticalDegree);
      delay((8));
    }
  }
  startScript = 0;
  }

}

void getFastLoop()
{
  // --- write ---
  verticalServo.write(100);
  horizontalServo.write(0);
  delay((1000));

  if (startScript) {  
    for(int j = 100; j >= 30; j--) {
      verticalServo.write(j);
      horizontalServo.write(0);
      delay((200));
      
      for(int i = 0; i <= 180; i++ ) {
        GetRange(j,i);
        horizontalServo.write(i);
        delay((4));
      }
    }
    verticalServo.write(100);
    horizontalServo.write(0);
    startScript = 0;
  }
}

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
 
void GetRange(double horizontalDegreeTest, double verticalDegreeTest)
{
  int distance = 0;
  getTFminiData(&distance);

  while (!distance) {
    getTFminiData(&distance);

    if (distance) {
      Serial.print(distance);
      Serial.print(",");
      Serial.print(horizontalDegreeTest);
      Serial.print(",");
      Serial.println(verticalDegreeTest);
      // Serial.print(",");
      // Serial.println("Ende");
    }
  }  
}

void setup()
{
  if (startScript == 1) {
    horizontalServo.attach(9);
    verticalServo.attach(10);
    Serial.begin(9600);
    Serial.begin(115200);                   //Initialize hardware serial port (serial debug port)

    while (!Serial);                        // wait for serial port to connect. Needed for native USB port only
    // Serial.println("Initializing...");
    SerialTFMini.begin(TFMINI_BAUDRATE);    //Initialize the data rate for the SoftwareSerial port
    tfmini.begin(&SerialTFMini);            //Initialize the TF Mini sensor
  } 
}
 
void loop() 
{
  getFastLoop();
  // getPrizeLoop();
}