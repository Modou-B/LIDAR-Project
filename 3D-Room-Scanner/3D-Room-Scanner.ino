#include <SoftwareSerial.h>
#include "TFMini.h"
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>



const double minHorizontalServoMicroDegree = 544; // get data tomorrow
const double maxHorizontalServoMicroDegree = 2400; // get data tomorrow
const double minVerticalServoMicroDegree = 853; // get data tomorrow
const double maxVerticalServoMicroDegree = 1575; //get data tomorrow

const char* _SSID = "FRITZ!Box 6660 Cable EF";
const char* _Password = "*****************";
const char* mqtt_server = "127.0.0.1";
const char* mqtt_user = "user";
const char* mqtt_password = "test";

double horizontalMicroDegreeT;
double verticalMicroDegree;
double horizontalDegree;
double verticalDegree;

Servo horizontalServo;
Servo verticalServo;

// int startScript = 0; // 1 = Start script, 0 = Stop script

// Lidar port allocation
TFMini tfmini;
SoftwareSerial SerialTFMini(2, 0);

WiFiClient espClient;
PubSubClient client(espClient);





void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  for (int i = 0; i < length; i++) 
  {
    Serial.print((char)payload[i]);
  }
  
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') 
  {
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else 
  {
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}



void setup_wifi() 
{

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(_SSID, _Password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  
  randomSeed(micros());
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}



void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str()), mqtt_user, mqtt_password) 
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("lidar/sendCords", "hello world");
      // ... and resubscribe
      client.subscribe("lidar/start");
    } else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



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
    }
  }  
}






void setup()
{
  Serial.begin(115200);
  setup_wifi();
  pinMode(LED_BUILTIN, OUTPUT);    // Initialize the LED_BUILTIN pin as an output

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  if (startScript == 1) {
    horizontalServo.attach(9);
    verticalServo.attach(10);
    Serial.begin(9600);
    Serial.begin(14400);                   //Initialize hardware serial port (serial debug port)

    while (!Serial);                        // wait for serial port to connect. Needed for native USB port only
    // Serial.println("Initializing...");
    SerialTFMini.begin(TFMINI_BAUDRATE);    //Initialize the data rate for the SoftwareSerial port
    tfmini.begin(&SerialTFMini);            //Initialize the TF Mini sensor
  } 
}
 


void loop() 
{
  //getFastLoop();
  // getPrizeLoop();

  // if (!client.connected()) 
  // {
  //   reconnect();
  // }
  // 
  // client.loop();
}