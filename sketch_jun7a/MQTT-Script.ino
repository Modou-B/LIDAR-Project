#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* _SSID = "BBS-Public";
const char* _Password = "SchuelerWLan4BBSB";
const char* mqtt_server = "10.250.11.90"; // generiert aus 0.0.0.1 in der mosquitto.conf = 1883 0.0.0.1
//const char* mqtt_user = "roger";
// const char* mqtt_password = "password";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

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
    delay(10000);
    Serial.print("Attempting MQTT connection...");
    
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    // if (client.connect(clientId.c_str()), mqtt_user, mqtt_password) 
    if (client.connect(clientId.c_str())) 
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("lidar/sendCords", "hello world");
      // ... and resubscribe
      //client.subscribe("lidar/start");
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

void setup() 
{
  Serial.begin(115200);
  setup_wifi();

  pinMode(LED_BUILTIN, OUTPUT);    // Initialize the LED_BUILTIN pin as an output

  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}


void loop() 
{
  /*
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.println("Publish message: ");
    Serial.println(msg);
    client.publish("lidar/sendCords", msg);
  }
  */
}