#include <Servo.h>

Servo myservo;
int val;
void setup() {
  // put your setup code here, to run once:
myservo.attach(9);
Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  // val = analogRead(0);
  // val = map(45,0,1023, 0, 100);

  // Serial.println(val);
  myservo.write(0);
  delay((1000));
  myservo.write(45);
  delay((1000));
  myservo.write(90);
  delay((1000));
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
