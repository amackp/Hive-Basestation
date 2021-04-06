/* Comment this out to disable prints and save space */
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266_WM.h>
#include "Credentials.h"
#include "Hive.h"

#define BLYNK_PRINT Serial

#define USE_SSL         false
#define USE_LITTLEFS    true
#define USE_SPIFFS      false
#define TIMEOUT_RECONNECT_WIFI                    10000L
#define RESET_IF_CONFIG_TIMEOUT                   true
#define CONFIG_TIMEOUT_RETRYTIMES_BEFORE_RESET    5

#define USE_DYNAMIC_PARAMETERS      false

bool use_fahrenheit_f = 0;
bool set_temp_f = 0;
volatile int fetching = 0;
 
BlynkTimer push_timer;
BlynkTimer fetch_timer;

HiveData hive;

void hive_fetch_data(){
  if(fetching > 5){
    fetching = 0;
    Blynk.setProperty(V12, "offLabel", "Refresh timout - Retry?"); 
    Blynk.virtualWrite(V12, 0);
  }
  else if(fetching){
    fetching++;
    lora_send_cmd("AT+SEND=500,6,FETCH=", NULL, 5);
  }
  else{
    //ignore
  }
  
}

void hive_start_fetch(){
  fetching = 1;
}

bool hive_parse_rx_data(String input){
  bool ret = false;
  int start_index = 0;
  int stop_index = 0;
  String cmd;
  String header;
  String payload;
  for( ; stop_index < input.length() ; ){
    if(input.charAt(stop_index) == ','){
      
       cmd = input.substring(start_index, stop_index);
       start_index = stop_index+1;
       header = cmd.substring(0, HEADER_LENGTH);
       payload = cmd.substring(HEADER_LENGTH);
       Serial.print("Parsing data: ");Serial.println(header+payload);
       if(header.equals(HIVE_INSIDE_TEMP)){
          hive.inside_temperature_c = payload.toFloat();
          hive.inside_temperature_f = (hive.inside_temperature_c * (9/5)) + 32;
          ret = true;
        }
        else if(header.equals(HIVE_INSIDE_HUMID)){
          hive.inside_humidity = payload.toFloat();
          ret = true;
        }
        else if(header.equals(HIVE_OUTSIDE_TEMP)){
          hive.outside_temperature_c = payload.toFloat();
          ret = true;
        }
        else if(header.equals(HIVE_OUTSIDE_HUMID)){
          hive.outside_humidity = payload.toFloat();
          ret = true;
        }
        else if(header.equals(HIVE_SOUND_LEVEL)){
          hive.sound_db = payload.toInt();
          ret = true;
        }
        else if(header.equals(HIVE_BAT_LEVEL)){
          hive.bat_charge = payload.toInt();
          ret = true;
        }
        else if(header.equals(HIVE_BAT_STATUS)){
          hive.bat_status = payload.toInt();
          ret = true;
        }
        else if(header.equals(HIVE_WEIGHT)){
          hive.raw_weight = payload.toFloat();
          //Blynk.setProperty(V12, "offLabel", "Refresh"); 
          Blynk.virtualWrite(V8, 0);
          ret = true;
        }
        else if(header.equals(HIVE_FAN_STATUS)){
          hive.fan_status = payload.toFloat();
          ret = true;
        }
        else if(header.equals(HIVE_CMD_SETTEMP)){
          if(hive.set_temp_c != payload.toInt()){
            set_temp_f = true;  //flag to sync the set temp
          }
          ret = true;
        }
        else{
          Serial.print("Error, unrecognized payload header: ");Serial.println(header);
        }
    }
    stop_index++;
  }

  if(ret){
    Serial.println("Successfully parsed a message!");
    fetching = 0;
  }
  return ret;
}


void pushValues()
{
  Serial.println("Pushing data to Blynk server.");
  Blynk.setProperty(V12, "offLabel", "Refresh"); 
  Blynk.virtualWrite(V12, 0);
  if(hive.temp_select){
    Blynk.virtualWrite(V0, hive.inside_temperature_f);
  }
  else{
    Blynk.virtualWrite(V0, hive.inside_temperature_c);
  }
  Blynk.virtualWrite(V1, hive.inside_humidity);
  Blynk.virtualWrite(V2, hive.outside_temperature_c);
  Blynk.virtualWrite(V3, hive.outside_humidity);
  Blynk.virtualWrite(V4, hive.sound_db);
  Blynk.virtualWrite(V5, hive.bat_charge);
  Blynk.virtualWrite(V6, hive.bat_status);
  if(hive.weight_select){
     Blynk.virtualWrite(V7, hive.raw_weight * 0.453592);
  }
  else{
    Blynk.virtualWrite(V7, hive.raw_weight);
  }
  
  Blynk.virtualWrite(V11,hive.bat_status==1?"Charging":"Not Charging");
  Blynk.virtualWrite(V13,hive.fan_status==1?"Active":"Inactive");
}

BLYNK_WRITE(V8)
{
  if(param.asInt() == 1){
    lora_send_cmd("AT+SEND=500,6,STARE=", NULL, 5);
  }
}
BLYNK_WRITE(V9)
{
  hive.set_temp_c = param.asInt();
  set_temp_f = true;
}
BLYNK_WRITE(V10)
{
  if(param.asInt() == 1){
    hive.temp_select = true;
    Blynk.virtualWrite(V0, hive.inside_temperature_f);
    Blynk.setProperty(V0, "min", 0);
    Blynk.setProperty(V0, "max", 175);
  }
  else{
    hive.temp_select = false;
    Blynk.virtualWrite(V0, hive.inside_temperature_c);
    Blynk.setProperty(V0, "min", -20);
    Blynk.setProperty(V0, "max", 80);
  }
}
BLYNK_WRITE(V12)
{
  if(param.asInt() == 1){
    fetching = 1;
  }
}
BLYNK_WRITE(V14)
{
  if(param.asInt() == 1){
    hive.weight_select = true;
    Blynk.virtualWrite(V7, hive.raw_weight * 0.453592);
  }
  else{
    hive.weight_select = false;
    Blynk.virtualWrite(V7, hive.raw_weight);
  }
}

BLYNK_CONNECTED() {
    Blynk.syncAll();
}

uint32_t send_timestamp = 0;
void setup()
{
  // Debug console
  Serial.begin(19200);
  lora_configure();
    // Set config portal SSID and Password
  Blynk.setConfigPortal("SmartHive-Home", "12345678");
  // Set config portal IP address
  Blynk.setConfigPortalIP(IPAddress(192, 168, 200, 1));
  // Set config portal channel, default = 1. Use 0 => random channel from 1-13 to avoid conflict
  Blynk.setConfigPortalChannel(0);

  Blynk.setSTAStaticIPConfig(IPAddress(192, 168, 2, 220), IPAddress(192, 168, 2, 1), IPAddress(255, 255, 255, 0));

  Blynk.begin("SmartHive-Home");
  
  //push_timer.setInterval(30000L, pushValues);
  fetch_timer.setInterval(3000L, hive_fetch_data);
  fetch_timer.setInterval(60000L, hive_start_fetch);
}
int expecting_rx = 0;
void loop()
{
  Blynk.run();
  //push_timer.run();
  fetch_timer.run();
  
  String rx = lora_parse_rx(lora_recieve());
  if(!rx.equals("ERR")){
    if(hive_parse_rx_data(rx)){
      pushValues();
    }
  }

  if(set_temp_f){
    set_temp_f = false;
    String subcmd = HIVE_CMD_SETTEMP+String(hive.set_temp_c);
    String cmd = "AT+SEND=500,"+String(subcmd.length())+","+subcmd;
    lora_send_cmd(cmd, NULL, 5);
  }
  
}
