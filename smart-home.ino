/*********
  Author        Marceloams
  repository    github.com/marceloams/university_projects/38fetin
*********/
//-----------------------------------------------------------------------------------
//LIBRARIES--------------------------------------------------------------------------
#include <FS.h> //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>
#include <DNSServer.h> 
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          // https://github.com/bblanchon/ArduinoJson
#include <FirebaseESP8266.h>
#include <DHT.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
//-----------------------------------------------------------------------------------
//FIREBASE SETTINGS------------------------------------------------------------------
#define FIREBASE_HOST       "https://smarthome-be270-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH       "f5gUVzD4r5ZbxHxyRQBbmxHlUYmE3d5EM8FD4LxU"
FirebaseData firebaseData;
//-----------------------------------------------------------------------------------
//INPUT PINS------------------------------------------------------------------------
#define P0             0
#define P2             2
//-----------------------------------------------------------------------------------
//DHT11 SETTINGS---------------------------------------------------------------------
#define DHTTYPE DHT11
//-----------------------------------------------------------------------------------
//WIFI SETTINGS----------------------------------------------------------------------
WiFiManager wifiManager;
bool    shouldSaveConfig  =     false; //flag for saving data
//-----------------------------------------------------------------------------------
//SERVER SETTINGS--------------------------------------------------------------------
WiFiServer server(80);
 
String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
String html_1 = "<!DOCTYPE html><html><head><title>Smart Home</title><style> body {align-items: center;text-align: center;}.button {background-color: #008CBA;border-radius: 8px;border: 2px solid #FFF;color: white;padding: 15px 100px;text-align: center;text-decoration: none;display: inline-block;font-size: 20px;}</style></head>";
String html_2 = "<body><div id='main'><h1>Local Mode</h1><br>";
String html_3;
String html_4 = "<br><p>Energy is down, but you can see your sensors information here!</p><br></div></body></html>";

String request = "";
//-----------------------------------------------------------------------------------
//VARIABLES--------------------------------------------------------------------------
String  deviceId;
int     deviceMode        =     0;
bool    resetValue        =     false;
bool    firstConnection   =     true;
bool    configured;
bool    ap_mode;
//-----------------------------------------------------------------------------------
//CUSTOM VARIABLES-------------------------------------------------------------------
char device_id[20]      =       "Put device id here";
char ssid[20]           =       "Smart Home";
char password[20]       =       "12345678";
//-----------------------------------------------------------------------------------
//DHT11 VARIABLES--------------------------------------------------------------------
float h = 0;
float t = 0;
DHT dht(D3, DHTTYPE);
//-----------------------------------------------------------------------------------
//TIMESTAMP VARIABLES----------------------------------------------------------------
String timeStamp;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -10800);
//-----------------------------------------------------------------------------------
//WIFI FUNCTIONS---------------------------------------------------------------------
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
//-----------------------------------------------------------------------------------
//PIN SETUP FUNCTION-----------------------------------------------------------------
void setupPins(){
  pinMode(P0,INPUT);
  pinMode(P2,INPUT);
  pinMode(D4,INPUT);
}
//-----------------------------------------------------------------------------------
//FIREBASE SETUP FUNCTION------------------------------------------------------------
void setupFirebase(){
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial.println("Firebase ok!");
}
//-----------------------------------------------------------------------------------
//DHT11 SETUP FUNCTION---------------------------------------------------------------
void setupDht11(){
  dht.begin();
  Serial.println("DHT11 ok!");
}
//-----------------------------------------------------------------------------------
//TIMESTAMP SETUP FUNCTION-----------------------------------------------------------
void setupTimestamp(){
  timeClient.begin();
  Serial.println("Timestamp ok!");
}
//-----------------------------------------------------------------------------------
//DHT11 MEASUREMENT FUNCTION---------------------------------------------------------
void readDht11(){
  delay(2000);
  h = dht.readHumidity();
  t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  FirebaseJson json;
  
  json.set("timestamp",getTimestamp());
  json.set("humidity",h);
  json.set("temperature",t);

  if (!Firebase.pushJSON(firebaseData, deviceId + "/measures/mode1", json)) {
    Serial.println("Firebase: Error to push JSON!");
  }
}
//-----------------------------------------------------------------------------------
//PRESENCE MEASUREMENT FUNCTION------------------------------------------------------
void readPresence(){

  int presence = digitalRead(D4); 

  if(presence == HIGH){
    if (!Firebase.pushString(firebaseData, deviceId + "/measures/mode2", getTimestamp())) {
      Serial.println("Firebase: Error to push JSON!");
    } 
  }
}
//-----------------------------------------------------------------------------------
//TIMESTAMP GETTIME FUNCTION---------------------------------------------------------
String getTimestamp(){
  timeClient.update();

  //get epoch time
  unsigned long epochTime = timeClient.getEpochTime();

  //get formatted time
  String formattedTime = timeClient.getFormattedTime();

  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  int monthDay = ptm->tm_mday;

  int currentMonth = ptm->tm_mon+1;

  int currentYear = ptm->tm_year+1900;

  //get current Date
  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);

  //get current date + formatted time = timestamp
  timeStamp = currentDate + " " + formattedTime;

  return timeStamp;
}
//-----------------------------------------------------------------------------------
//VOID SETUP-------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  //-----------------------------------------------------------------------------------
  //LOAD FS INFO-----------------------------------------------------------------------
  //SPIFFS.format();//clean FS, for testing

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(device_id, json["device_id"]);
          strcpy(ssid, json["ssid"]);
          strcpy(password, json["password"]);
          configured = json["configured"];
          ap_mode = json["ap_mode"];
          deviceMode = json["device_mode"];
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
  //-----------------------------------------------------------------------------------
  //VERIFY IF HAS BEEN ALREADY CONFIGURED----------------------------------------------
  if(!configured || !ap_mode){
    //-----------------------------------------------------------------------------------
    //CUSTOM PARAMETER SETTINGS----------------------------------------------------------
  
    //custom inputs
    // id/name, placeholder/prompt, default, length
    WiFiManagerParameter custom_device_id("device", "Device", device_id, 20);
    WiFiManagerParameter custom_ssid("ssid", "SSID", ssid, 20);
    WiFiManagerParameter custom_password("password", "Password", password, 20);
  
    //custom html
    WiFiManagerParameter custom_device("<h4>Device Settings</h4>");
    WiFiManagerParameter custom_wifi("<h4>Local Wi-Fi Settings</h4>");
  
    //add all your parameters here
    wifiManager.addParameter(&custom_device);
    wifiManager.addParameter(&custom_device_id);
    wifiManager.addParameter(&custom_wifi);
    wifiManager.addParameter(&custom_ssid);
    wifiManager.addParameter(&custom_password);
  
    //-----------------------------------------------------------------------------------
    //WIFI SETTINGS----------------------------------------------------------------------
    wifiManager.setSaveConfigCallback(saveConfigCallback); //set config save notify callback
  
    //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); //set static ip
    //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); // set custom ip for portal
    //wifiManager.resetSettings();// Uncomment and run it once, if you want to erase all the stored information
    //wifiManager.setMinimumSignalQuality(); //set minimu quality of signal so it ignores AP's under that quality (defaults to 8%)
    //wifiManager.setTimeout(120); //sets timeout until configuration portal gets turned off (in seconds)
  
    
    wifiManager.autoConnect(ssid,password); //wifi configuration
    //wifiManager.autoConnect(); // or use this for auto generated name ESP + ChipID
    Serial.println("Connected."); // if you get here you have connected to the WiFi
    //-----------------------------------------------------------------------------------
    //GET VALUES FORM INTERFACE----------------------------------------------------------
    strcpy(device_id, custom_device_id.getValue());
    strcpy(ssid, custom_ssid.getValue());
    strcpy(password, custom_password.getValue());
  
    //-----------------------------------------------------------------------------------
    //SAVE THE CUSTOM PARAMETERS TO FS-------------------------------------------------
    if (shouldSaveConfig) {
      configured = true;
      Serial.println("saving config");
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.createObject();
      json["device_id"] = device_id;
      json["ssid"] = ssid;
      json["device_mode"] = deviceMode;
      json["password"] = password;
      json["configured"] = configured;
      json["ap_mode"] = ap_mode;
  
      File configFile = SPIFFS.open("/config.json", "w");
      if (!configFile) {
        Serial.println("failed to open config file for writing");
      }
  
      json.printTo(Serial);
      json.printTo(configFile);
      configFile.close();
      //end save
    }
    //-----------------------------------------------------------------------------------
    //PRINT TO SEE IF EVERITHYNG HAS BEEN CONFIGURED-------------------------------------  
    delay(3000);
    Serial.println("Wifi is ready!");
    Serial.print("Device SSID: ");
    Serial.println(ssid);
    Serial.print("Device Password");
    Serial.println(password);
  
    Serial.println("");
    Serial.print("Device has been already Configured? ");
    Serial.println(configured);
    //-----------------------------------------------------------------------------------
    //SETUP FIREBASE---------------------------------------------------------------------
    setupFirebase();
    //-----------------------------------------------------------------------------------
    //SETUP DHT11------------------------------------------------------------------------
    setupDht11();
    //-----------------------------------------------------------------------------------
    //SETUP Timestamp--------------------------------------------------------------------
    setupTimestamp();
    //-----------------------------------------------------------------------------------
    ////SET DEVICE ID--------------------------------------------------------------------
    deviceId = device_id;
  } else {
    //-----------------------------------------------------------------------------------
    //SERVER SETUP-----------------------------------------------------------------------
    Serial.println("Ap mode begining...");
    boolean conn = WiFi.softAP(ssid, password);
    server.begin();
  }
  //-----------------------------------------------------------------------------------
  //SETUP PINS-------------------------------------------------------------------------
  setupPins();
}

//-----------------------------------------------------------------------------------
//VOID LOOP--------------------------------------------------------------------------
void loop(){

  if(!ap_mode){
    //-----------------------------------------------------------------------------------
    //GET MODE INFO FROM FIREBASE ------------------------------------------------------- 
    Firebase.getInt(firebaseData, deviceId + "/mode");
    deviceMode = firebaseData.intData();
    
    //-----------------------------------------------------------------------------------
    // Device Modes: --------------------------------------------------------------------
    // Mode 0: Default ------------------------------------------------------------------
    // Mode 1: DHT11 Sensor -------------------------------------------------------------
    // Mode 2: Presence Sensor ----------------------------------------------------------
    switch(deviceMode){
      case 0: break;
      case 1: readDht11();
      break;
      case 2: readPresence();
      break;
    }
   
    //-----------------------------------------------------------------------------------
    //GET RESET INFO FROM FIREBASE-------------------------------------------------------
    // Verifica o valor da porta no firebase 
    Firebase.getBool(firebaseData, deviceId + "/reset");
    resetValue = firebaseData.boolData();
    
    //-----------------------------------------------------------------------------------
    //RESET DEVICE-----------------------------------------------------------------------
    if(resetValue){
      //-----------------------------------------------------------------------------------
      //PRINT TO SAY THAT IS RESETING------------------------------------------------------
      Serial.println("Reseting device...");
      //-----------------------------------------------------------------------------------
      //RESET VARIABLES--------------------------------------------------------------------
      resetValue = false;
      configured = false;
      strcpy(device_id, "Put device id here");
      //-----------------------------------------------------------------------------------
      //PRINT UPDATED VARIABLES------------------------------------------------------------
      Serial.print("Reset value: ");
      Serial.println(resetValue);
      Serial.print("Configured: ");
      Serial.println(configured);
      Serial.print("Device Id: ");
      Serial.println(device_id);
      //-----------------------------------------------------------------------------------
      //UPDATE FS--------------------------------------------------------------------------
      Serial.println("saving config");
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.createObject();
      json["device_id"] = device_id;
      json["ssid"] = "Smart Home";
      json["password"] = "12345678";
      json["configured"] = configured;
      File configFile = SPIFFS.open("/config.json", "w");
      if (!configFile) {
        Serial.println("failed to open config file for writing");
      }
      json.printTo(Serial);
      json.printTo(configFile);
      configFile.close();
      //end save
      //-----------------------------------------------------------------------------------
      //RESET WIFI SETTINGS----------------------------------------------------------------
      wifiManager.resetSettings();
      //-----------------------------------------------------------------------------------
      //RESTART NODEMCU--------------------------------------------------------------------
      ESP.restart();
    }
    //-----------------------------------------------------------------------------------
    //1 SEC DELAY------------------------------------------------------------------------
    delay(1000);
  } else{
    // Check if a client has connected
    WiFiClient client = server.available();
    if (!client)  {  return;  }
 
    // Read the first line of the request
    request = client.readStringUntil('\r');
 
    if (request.indexOf("DOOR") > 0){
    }
 
    client.flush();
 
    client.print( header );
    client.print( html_1 );
    client.print( html_2 );
    client.print( html_3 );
    client.print( html_4);
 
    delay(5);
  }
}
