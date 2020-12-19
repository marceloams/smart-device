/*********
  Author        marceloams
  repository    github.com/marceloams/smart-device
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
//DHT11 SETTINGS---------------------------------------------------------------------
#define DHTTYPE DHT11
//-----------------------------------------------------------------------------------
//WIFI SETTINGS----------------------------------------------------------------------
WiFiManager wifiManager;
bool    shouldSaveConfig  =     false; //flag for saving data
//-----------------------------------------------------------------------------------
//SERVER SETTINGS--------------------------------------------------------------------
WiFiServer server(80);
//-----------------------------------------------------------------------------------
//VARIABLES--------------------------------------------------------------------------
String  deviceId;
int     deviceMode        =     0;
bool    resetValue        =     false;
bool    firstConnection   =     true;
bool    configured = false;
bool    ap_mode = false;
//-----------------------------------------------------------------------------------
//CUSTOM VARIABLES-------------------------------------------------------------------
char device_id[20]      =       "Put device id here";
char ssid[20]           =       "Smart Device";
char password[20]       =       "12345678";
//-----------------------------------------------------------------------------------
//DHT11 VARIABLES--------------------------------------------------------------------
float h = 0;
float t = 0;
DHT dht(2, DHTTYPE);
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
  pinMode(0,INPUT);
  pinMode(2,INPUT);
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
  delay(30000);
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

  if (!Firebase.pushJSON(firebaseData, deviceId + "/measures", json)) {
    Serial.println("Firebase: Error to push JSON!");
  }
}
//-----------------------------------------------------------------------------------
//PRESENCE MEASUREMENT FUNCTION------------------------------------------------------
void readPresence(){
  delay(500);
  int presence = digitalRead(0); 

  FirebaseJson json;

  if(presence == HIGH){
    
    json.set("timestamp",getTimestamp());
    json.set("presence",true);
    
    if (!Firebase.pushJSON(firebaseData, deviceId + "/measures", json)) {
      Serial.println("Firebase: Error to push JSON!");
    } 
  }else {
    json.set("timestamp",getTimestamp());
    json.set("presence",false);
    if (!Firebase.updateNode(firebaseData, deviceId + "/measures/nopresence", json)) {
      Serial.println("Firebase: Error to update JSON!");
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

      if(Firebase.deleteNode(firebaseData, deviceId)){
        Serial.println("Fail to delete device from firebase!");
      }
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
      json["ssid"] = "Smart Device";
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
    }else{
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
        case 0: readDht11();
        break;
        case 1: readPresence();
        break;
      }
    }
   
    //-----------------------------------------------------------------------------------
    //1 SEC DELAY------------------------------------------------------------------------
    delay(1000);
  }
}
