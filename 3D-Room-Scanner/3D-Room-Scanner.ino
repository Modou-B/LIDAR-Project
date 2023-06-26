#include <SoftwareSerial.h>
#include "TFMini.h"
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

Servo horizontalServo;
Servo verticalServo;

const double minHorizontalServoMicroDegree = 544;
const double maxHorizontalServoMicroDegree = 2400;
const double minVerticalServoMicroDegree = 853;
const double maxVerticalServoMicroDegree = 1575;
const double horizontalStep = (maxHorizontalServoMicroDegree - minHorizontalServoMicroDegree) / 180;
const double verticalStep = (maxVerticalServoMicroDegree - minVerticalServoMicroDegree) / 70;

const double horizontalInterval = horizontalStep / 2;
const double verticalInterval = verticalStep / 2;

double horizontalMicroDegreeT;
double verticalMicroDegree;
double horizontalDegree;
double verticalDegree;

// Max loop Servo rounds
double maxFastLoopServoRounds = 12780.0;

// Slow Loop Starting Values
double slowLoopStartingX = 544;
double slowLoopStartingY = 1575;

// Slow loop counters
int slowLoopX = 0;
int slowLoopY = 100;

int slowLoopXCounter = 0;
int slowLoopYCounter = 0;


// Fast Loop Starting Values
int fastLoopStartingX = 0;
int fastLoopStartingY = 100;

// Lidar port allocation
TFMini tfmini;
SoftwareSerial SerialTFMini(5,4);

const char* _SSID = "AndroidAP_7896";
const char* _Password = "passwordff";
const char* mqtt_server = "192.168.43.39";
//const char* mqtt_user = "roger";
// const char* mqtt_password = "password";

WiFiClient espClient;
PubSubClient client(espClient);

// Topic Values
int stopScript = 0;
int startScript = 0;
int restartScript = 0;
int speedMode = 1;

// SUBSCRIBE TOPICS
const char* START_TOPIC = "lidar/start";
const char* STOP_TOPIC = "lidar/stop";
const char* RESTART_TOPIC = "lidar/restart";
const char* SPEEDMODE_TOPIC = "lidar/speedMode";
const char* Y_POSITITON_TOPIC = "lidar/setYPosition";
const char* X_POSITITON_TOPIC = "lidar/setXPosition";

// PUBLISH TOPICS
const char* CORDS_TOPIC = "lidar/sendCords";
const char* PROCESS_TOPIC = "lidar/process";
const char* POSITION_TOPIC = "lidar/positions";

StaticJsonDocument<256> cords;
char out[128];

StaticJsonDocument<50> process;
char processMessage[25];

StaticJsonDocument<100> positions;
char positionMessage[50];

double calculateLidarProcess(double maxLoopRounds, double currentXValue, double currentYValue)
{
  return (currentYValue * 180) / maxLoopRounds;
}

void publishLidarProcess(double currentXValue, double currentYValue) 
{
    double currentYProcess = 100 - currentYValue + 1;
    double currentProcess = calculateLidarProcess(maxFastLoopServoRounds, currentXValue, currentYProcess);
    
    process["process"] = currentProcess * 100;

    serializeJson(process, processMessage);
    client.publish(PROCESS_TOPIC, processMessage);
}

void slowLoop()
{
  double startingValueX = slowLoopStartingX;
  double startingValueY = slowLoopStartingY;

  positions["x"] = slowLoopX;
  positions["y"] = slowLoopY;

  // --- Set starting positions ---
  verticalServo.writeMicroseconds(startingValueY);
  horizontalServo.writeMicroseconds(startingValueX);
  delay(800);

  for(double verticalMicroDegree = startingValueY; verticalMicroDegree >= minVerticalServoMicroDegree; verticalMicroDegree -= verticalInterval) {
    verticalServo.writeMicroseconds(verticalMicroDegree);
    slowLoopYCounter++;

    slowLoopStartingY = verticalMicroDegree;
    verticalDegree = ((verticalMicroDegree - minVerticalServoMicroDegree) / verticalStep) + 30;

    cords["y"] = verticalDegree;

    if (slowLoopYCounter == 2) {
      slowLoopYCounter = 0;
      slowLoopY--;
      positions["y"] = slowLoopY;
    }

    horizontalServo.writeMicroseconds(minHorizontalServoMicroDegree);
    delay(800);
    
    for(double horizontalMicroDegree = startingValueX; horizontalMicroDegree <= maxHorizontalServoMicroDegree; horizontalMicroDegree += horizontalInterval) {
      client.loop();
      horizontalServo.writeMicroseconds(horizontalMicroDegree);
      slowLoopXCounter++;

      slowLoopStartingX = horizontalMicroDegree;
      horizontalDegree = (horizontalMicroDegree - minHorizontalServoMicroDegree) / horizontalStep;
      cords["x"] = horizontalDegree;

      if (slowLoopXCounter == 2) {
        slowLoopXCounter = 0;
        slowLoopX++;
        positions["x"] = slowLoopX;
      }

      if (stopScript == 1) {
        break;
      }

      if (restartScript == 1) {
        break;
      }
      
      getRange();

      delay(20);
    }

    publishLidarProcess(slowLoopX, slowLoopY);
    if (stopScript == 1) {
        break;
    }

    if (restartScript == 1) {
      break;
    }

    slowLoopStartingX = 544;
    startingValueX = 544;
    slowLoopX = 0;
  }
  

}

void fastLoop()
{
  int horizontalStartingValue = fastLoopStartingX;
  int verticalStartingValue = fastLoopStartingY;

  horizontalServo.write(horizontalStartingValue);
  verticalServo.write(verticalStartingValue);
  
  while (horizontalServo.read() != horizontalStartingValue) {
    delay(400);
  }

  for(int j = verticalStartingValue; j >= 30; j--) {
    verticalServo.write(j);

    fastLoopStartingY = j;
    cords["y"] = j;
    positions["y"] = j;
    
    horizontalServo.write(horizontalStartingValue);
    while (horizontalServo.read() != horizontalStartingValue) {
      delay(400);
    }
    
    for(int i = horizontalStartingValue; i <= 180; i++ ) {
      client.loop();

      fastLoopStartingX = i;
      cords["x"] = i;
      positions["x"] = i;

      getRange();

      delay(20);

      horizontalServo.write(i);
      if (stopScript == 1) {
        break;
      }

      if (restartScript == 1) {
        break;
      }
    }

    publishLidarProcess(fastLoopStartingX, fastLoopStartingY);
    if (stopScript == 1) {
        break;
    }

    if (restartScript == 1) {
      break;
    }

    horizontalStartingValue = 0;
    fastLoopStartingX = 0;
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
 
void getRange()
{
  int distance = 0;
  getTFminiData(&distance);

  while (!distance) {
    getTFminiData(&distance);

    if (distance) {
      cords["z"] = distance;

      serializeJson(cords, out);
      client.publish(CORDS_TOPIC, out);
      delay(4);
      serializeJson(positions, positionMessage);
      client.publish(POSITION_TOPIC, positionMessage);

    }
  }  
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  if (strcmp(topic, START_TOPIC) == 0){    
    Serial.print("asdhasd, rc=");

    int startValue = (char)payload[0] - '0';
    startScript = startValue;
    
    stopScript = 0;
  }

  if (strcmp(topic, STOP_TOPIC) == 0) {
    int stopValue = (char)payload[0] - '0';
    stopScript = stopValue;

    startScript = 0;
  }

  if (strcmp(topic, RESTART_TOPIC) == 0) {
    int restartValue = (char)payload[0] - '0';
    restartScript = restartValue;

    fastLoopStartingX = 0;
    fastLoopStartingY = 100;
    slowLoopStartingX = 544;
    slowLoopStartingY = 1575;
    slowLoopX = 0;
    slowLoopY = 100;

    slowLoopXCounter = 0;
    slowLoopYCounter = 0;
  }  
   
  if (strcmp(topic, SPEEDMODE_TOPIC) == 0) {
    if (startScript == 0 && stopScript == 0) {
      int speedValue = (char)payload[0] - '0';
      speedMode = speedValue;
    }
  }  

  if (strcmp(topic, Y_POSITITON_TOPIC) == 0) {
    if (startScript == 0 && stopScript == 0) {
      payload[length] = '\0';
      int yPosition = atoi((char *)payload);

      slowLoopStartingY = maxVerticalServoMicroDegree - ((100 - yPosition) * verticalStep);
      slowLoopY = 100;
      fastLoopStartingY = yPosition;
    }
  }  

  if (strcmp(topic, X_POSITITON_TOPIC) == 0) {
    if (startScript == 0 && stopScript == 0) {
      payload[length] = '\0';
      int xPosition = atoi((char *)payload);

      slowLoopStartingX = minHorizontalServoMicroDegree + (xPosition * horizontalStep);
      slowLoopX = 0;
      fastLoopStartingX = xPosition;
    }
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
    // if (client.connect(clientId.c_str()), mqtt_user, mqtt_password) 
    if (client.connect(clientId.c_str())) 
    {
      Serial.println("connected");

      client.subscribe(START_TOPIC);
      client.subscribe(STOP_TOPIC);
      client.subscribe(RESTART_TOPIC);
      client.subscribe(SPEEDMODE_TOPIC);
      client.subscribe(Y_POSITITON_TOPIC);
      client.subscribe(X_POSITITON_TOPIC);      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  horizontalServo.attach(14);
  verticalServo.attach(12);
  Serial.begin(115200);

  while (!Serial);                        // wait for serial port to connect. Needed for native USB port only

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Serial.println("InitializingTasd...");
  SerialTFMini.begin(TFMINI_BAUDRATE);    //Initialize the data rate for the SoftwareSerial port
  tfmini.begin(&SerialTFMini);            //Initialize the TF Mini sensor
}
 
void loop() 
{ 
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  if (restartScript == 1) {
      positions["x"] = 0;
      positions["y"] = 100;

      serializeJson(positions, positionMessage);
      client.publish(POSITION_TOPIC, positionMessage);

      delay(8);
      process["process"] = 0;

      serializeJson(process, processMessage);
      client.publish(PROCESS_TOPIC, processMessage);

      horizontalServo.write(0);
      verticalServo.write(100);

      delay(5);
      stopScript = 0;
      restartScript = 0;
      startScript = 0;
  }

  if (startScript == 1) {
    delay(2000);
    if (speedMode == 1) {
      fastLoop();
    }

    if (speedMode == 0) {
      slowLoop();
    }

    startScript = 0;
  }

  if (startScript == 0 && stopScript == 0) {
    delay(5);
    if (verticalServo.read() != fastLoopStartingY) {
      verticalServo.write(fastLoopStartingY);
    }
    delay(5);
    if (horizontalServo.read() != fastLoopStartingX) {
      horizontalServo.write(fastLoopStartingX);    
    }
  }
}

