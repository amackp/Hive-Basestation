#pragma once
#define MAX_NUM_HIVES 1

typedef struct Sensor_Data{
  //Temp/Humidity sensor data
  float inside_temperature_c;
  float inside_temperature_f;
  float inside_humidity;
  
  float outside_temperature_c;
  float outside_temperature_f;
  float outside_humidity;

  float set_temp_c;
  float set_temp_f;

  int fan_status; //0 off, 1 running

  char temp_select; //0 for C, 1 for f
  
  //load cell data
  float raw_weight;
  char weight_select; //0 for Kgs, 1 for Lbs

  //battery data
  int bat_charge;
  int bat_status;

  //sound data
  int sound_db;
}HiveData;

//length for headers of all messages, inlcuding '='
#define HEADER_LENGTH 6

//base to hive command
#define HIVE_CMD_FETCH       "FETCH="
#define HIVE_CMD_TARE        "STARE="
#define HIVE_CMD_SETTEMP     "STEMP=" //Input temp value after '='

//hive to base messages

#define HIVE_INSIDE_TEMP     "ITEMP="
#define HIVE_INSIDE_HUMID    "IHUMD="
#define HIVE_OUTSIDE_TEMP    "OTEMP="
#define HIVE_OUTSIDE_HUMID   "OHUMD="
#define HIVE_SOUND_LEVEL     "SOUND="
#define HIVE_BAT_LEVEL       "BATLV="
#define HIVE_BAT_STATUS      "BATST="
#define HIVE_WEIGHT          "SCALE="
#define HIVE_FAN_STATUS      "FANST="


//typedef union{
//  struct{
//      float inside_temperature_c;
//      float inside_humidity;
//
//      float outside_temperature_c;
//      float outside_humidity;
//
//      float raw_weight;
//      
//      int bat_charge;
//      int bat_status;
//      
//      int sound_db;
//  }str;
//  char bytes[sizeof(str)];
//}LoRa_MISO_Stream;
//
//typedef union{
//  struct{
//    float set_temp_c;
//    char tare_scale;
//  }str;
//  char bytes[sizeof(str)];
//}LoRa_MOSI_Stream;
