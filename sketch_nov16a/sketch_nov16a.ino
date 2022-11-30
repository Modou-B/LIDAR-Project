#include <Servo.h>

Servo myservo1;
Servo myservo2;
int val;
int val2;
int stop = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  stop = 1;
  myservo2.write(100);
  myservo1.write(0);
  delay((1000));
  if (!stop) {
    for(int j = 100; j >= 30; j--) {
      myservo2.write(j);
      val2 =  myservo2.read();
      Serial.println(val2);
      myservo1.write(0);
      delay((500));
      for(int i = 0; i <= 180; i++ ) {
        val = myservo1.read();
        Serial.println(val);
        myservo1.write(i);
        delay((4));
      }
    }
  }
  myservo2.write(100);
  myservo1.write(0);
  stop = 1;
  exit;

}





//Exit
// #include <Servo.h>

// Servo myservo;
// int val;
// void setup() {
//   // put your setup code here, to run once:

// Serial.begin(9600);
// }

// void loop() {
  
//   exit;
// }
