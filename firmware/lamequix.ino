/**
 *  _                           ____        _      
 * | |                         / __ \      (_)     
 * | |     __ _ _ __ ___   ___| |  | |_   _ ___  __
 * | |    / _` | '_ ` _ \ / _ \ |  | | | | | \ \/ /
 * | |___| (_| | | | | | |  __/ |__| | |_| | |>  < 
 * |______\__,_|_| |_| |_|\___|\___\_\\__,_|_/_/\_\
 * 
 * configuration via sd card
 * 
 * wifi.txt
 *  line 1: SSID
 *  line 2: password
 *  
 * mqtt.txt
 *  line 1: servername
 *  line 2: topic to subscribe to
 */

#include <ESP8266WiFi.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Wire.h>
#include <PubSubClient.h>

uint32_t bat = 4200;
Adafruit_PCD8544 display = Adafruit_PCD8544(2, 15, 0);

#define BUTTON_UP    B10000000
#define BUTTON_DOWN  B00100000
#define BUTTON_LEFT  B01000000
#define BUTTON_RIGHT B00010000
#define BUTTON_B     B00000100
#define BUTTON_A     B00001000
#define BUTTON_SD    B00000010

#define MESSAGE_LENGTH    100
#define MAX_MESSAGES        4

boolean bottonU_pressed = false;
boolean bottonD_pressed = false;
boolean bottonL_pressed = false;
boolean bottonR_pressed = false;
boolean bottonA_pressed = false;
boolean bottonB_pressed = false;
boolean bottonSD_pressed = false;

boolean bottonU_last_pressed = false;
boolean bottonD_last_pressed = false;
boolean bottonL_last_pressed = false;
boolean bottonR_last_pressed = false;
boolean bottonA_last_pressed = false;
boolean bottonB_last_pressed = false;
boolean bottonSD_last_pressed = false;

boolean bottonU_changed = false;
boolean bottonD_changed = false;
boolean bottonL_changed = false;
boolean bottonR_changed = false;
boolean bottonA_changed = false;
boolean bottonB_changed = false;
boolean bottonSD_changed = false;

String mqtt_server;

String wifi_ssid;
String wifi_password;

String readChannel;
const char* writeChannel = "outTopic";

WiFiClient espClient;
PubSubClient client(espClient);

char msg[100];
char mqtt_server_char[100];

long lastMsg = 0;
int showMessage = 0;
int messageCount = 0;
char lastmsg[MAX_MESSAGES][MESSAGE_LENGTH];

bool hasMessage = false;
bool hasRead = true;

int value = 0;

const int chipSelect = 16;

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  Wire.beginTransmission(0x38);
  Wire.write(B11111110);
  Wire.endTransmission();

  setBackgroundColor('c', 200,200,200);

  read_config_from_sd();

  display.begin();
  display.clearDisplay();
  display.setRotation(2);
  display.setContrast(58);
  display.display();

  setBackgroundColor('n', 255, 100, 100);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.println("OH HI!");
  display.display();
  delay(2000);

  display.setTextColor(BLACK, WHITE);
  display.setTextSize(1);

  setup_wifi();

  if (mqtt_server.length()>0) {
    mqtt_server.trim();
    mqtt_server.toCharArray(mqtt_server_char, mqtt_server.length()+1);

    Serial.print("MQTT Server: ");
    Serial.print(mqtt_server_char);
  
    client.setServer(mqtt_server_char, 1883);
    client.setCallback(callback);
  }
}

void read_config_from_sd() {
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File mqttFile = SD.open("mqtt.txt", FILE_READ);

  int linecounter = 0;

  // if the file is available, write to it:
  if (mqttFile) {
    linecounter = 0;
    while (mqttFile.available()) {
      switch(linecounter) {
        case 0:
          mqtt_server = mqttFile.readStringUntil('\n');
          mqtt_server.trim();
          break;
        case 1:
          readChannel = mqttFile.readStringUntil('\n');
          readChannel.trim();
          break;
      }

      linecounter++;
      if (linecounter > 1) {
        break;
      }
    }
    mqttFile.close();
    Serial.println(readChannel);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening mqtt.txt");
  }
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File wifiFile = SD.open("wifi.txt", FILE_READ);

  // if the file is available, write to it:
  if (wifiFile) {
    linecounter = 0;
    while (wifiFile.available()) {
      switch(linecounter) {
        case 0:
          wifi_ssid = wifiFile.readStringUntil('\n');
          wifi_ssid.trim();
          break;
        case 1:
          wifi_password = wifiFile.readStringUntil('\n');
          wifi_password.trim();
          break;
      }

      linecounter++;
      if (linecounter > 1) {
        break;
      }
    }
    wifiFile.close();
    
    Serial.print("SSID: ");
    Serial.println(wifi_ssid);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening wifi.txt");
  }  
}

void setup_wifi() {
  if (wifi_ssid.length()==0) {
    return;
  }
  
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  char ssid[wifi_ssid.length()+1];
  wifi_ssid.toCharArray(ssid, wifi_ssid.length()+1);

  char password[wifi_password.length()+1];
  wifi_password.toCharArray(password, wifi_password.length()+1);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  if (messageCount < MAX_MESSAGES) {
    messageCount++;
  }

  ///// lastmsg buffer change
  for (int k = MAX_MESSAGES - 1; k > 0; k--) {
    for(int l = 0; l < MESSAGE_LENGTH; l++) {
      lastmsg[k][l] = lastmsg[k-1][l];
    }
  }

  for(int j = 0; j < MESSAGE_LENGTH; j++) {
    lastmsg[0][j] = NULL;
  }
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    if (i < MESSAGE_LENGTH) {
      lastmsg[0][i] = (char)payload[i];
    }
  }
  hasMessage = true;
  hasRead = false;
  showMessage = 0;
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(writeChannel, "hello world");
      // ... and resubscribe

      char subscriptionChannel[readChannel.length()+1];
      readChannel.toCharArray(subscriptionChannel, readChannel.length()+1);
      
      Serial.print("Subscribed to: ");
      Serial.println(subscriptionChannel);
      client.subscribe(subscriptionChannel);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() 
{
  read_buttons();
  
  display.clearDisplay();

  // show selected message
  display.print("Msg #");
  display.print((showMessage + 1));
  display.print("/");
  display.print(messageCount);
  
  // show battery voltage
  uint32_t aIn = analogRead(A0);
  aIn = map(aIn, 0, 1024, 0, 4400); 
  bat = (aIn+bat)/2;
  display.print("  ");
  display.println(bat);

  // mqtt loop / reconnect
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // print selected message
  display.print(lastmsg[showMessage]);
  display.println("");

  // backlight on in case of unread message
  if (hasRead) {
    setBackgroundColor('n', 0, 0, 0);
  } else {
    setBackgroundColor('n', 255, 100, 100);
  }

  // notify for new message
  if (hasMessage) {
    vibrate(200);
    hasMessage = false;
  }

  // scroll messages down
  if (bottonD_pressed && bottonD_changed) {
    if (showMessage < ( MAX_MESSAGES - 1) ) {
      showMessage++;
    }
  }

  // scroll messages up
  if (bottonU_pressed && bottonU_changed) {
    if (showMessage > 0) {
      showMessage--;
    }
  }
  
  // mark message read
  if (bottonB_pressed) {
    if (showMessage == 0) {
      hasRead = true;
    }
  }

  // send message
  if (bottonA_pressed && bottonA_changed) {
    ++value;
    snprintf (msg, 75, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(writeChannel, msg);
    vibrate(100);
  }
  
  display.display();
  delay(100);
}

void vibrate(int milli) {
    Wire.beginTransmission(0x38);
    Wire.write(B11111111);
    Wire.endTransmission();

    delay (milli);
    
    Wire.beginTransmission(0x38);
    Wire.write(B11111110);
    Wire.endTransmission();
}

void setBackgroundColor(char c, int r, int g, int b)
{
  // c is fade to color, n is now color
  Wire.beginTransmission(0x80);
  Wire.write(c);
  Wire.write(r);
  Wire.write(g);
  Wire.write(b);
  Wire.endTransmission();
}

void read_buttons()
{
  Wire.requestFrom(0x38, 1);
  byte data = Wire.read();
  bottonU_pressed = (~data & BUTTON_UP) > 0;
  bottonD_pressed = (~data & BUTTON_DOWN) > 0;
  bottonL_pressed = (~data & BUTTON_LEFT) > 0;
  bottonR_pressed = (~data & BUTTON_RIGHT) > 0;
  bottonA_pressed = (~data & BUTTON_A) > 0;
  bottonB_pressed = (~data & BUTTON_B) > 0;
  bottonSD_pressed = (~data & BUTTON_SD) > 0;

  bottonU_changed = bottonU_last_pressed != bottonU_pressed;
  bottonD_changed = bottonD_last_pressed != bottonD_pressed;
  bottonL_changed = bottonL_last_pressed != bottonL_pressed;
  bottonR_changed = bottonR_last_pressed != bottonR_pressed;
  bottonA_changed = bottonA_last_pressed != bottonA_pressed;
  bottonB_changed = bottonB_last_pressed != bottonB_pressed;
  bottonSD_changed = bottonSD_last_pressed != bottonSD_pressed;
  
  bottonU_last_pressed = bottonU_pressed;
  bottonD_last_pressed = bottonD_pressed;
  bottonL_last_pressed = bottonL_pressed;
  bottonR_last_pressed = bottonR_pressed;
  bottonA_last_pressed = bottonA_pressed;
  bottonB_last_pressed = bottonB_pressed;
  bottonSD_last_pressed = bottonSD_pressed;
}
