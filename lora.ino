#include <SoftwareSerial.h>
#include "Hive.h"

SoftwareSerial lora(D7,D8);

bool lora_configure(){
  bool low_baud = false;
  lora.begin(19200);
  while(!lora_send_and_verify("AT+IPR=19200", 1)){
    lora.end();
    delay(100);
    if(low_baud){
      lora.begin(9600);
    }
    else{
      lora.begin(19200);
    }
    low_baud = !low_baud;
  }
  lora_send_and_verify("AT+PARAMETER=9,7,1,4", 5);
  lora_send_and_verify("AT+BAND=915000000", 5);
  lora_send_and_verify("AT+NETWORKID=7", 5);
  lora_send_and_verify("AT+ADDRESS=501", 5);
  //lora_send_and_verify("AT+CPIN=FABC0002EEDCAA90FABC0002EEDCAA90", 5);  
  lora.setTimeout(1000);
}

String lora_recieve(){
  String rx_string;
  
  if(lora.available()){
   
    rx_string = lora.readStringUntil('\n');
    
    if(rx_string.length() == 0){
      Serial.println("No repsonse!");
    }
    else{
      Serial.print("New Rx, len: ");Serial.println(rx_string.length());
      Serial.print("Rx: ");Serial.println(rx_string);
      return rx_string;
    }
  }
  return String("ERR");
}

//rx fromat +RCV=<ADDR>,<LENGTH>,<MSG>,<RSSI>,<SNR>
//ex +RCV=50,5,HELLO,-99,40
String lora_parse_rx(String message_in){
  if(message_in.equals("ERR")){
    //empty string input, ignore
    return String("ERR");
  }
  String header = message_in.substring(0,5);
  if(!header.equals("+RCV=")){
    Serial.println("Error parsing message, invalid header!");
    Serial.println(message_in);
    return String("ERR");
  }
  
  int index_start = 5;
  int index_end = 5;
  for( ; index_end < message_in.length(); index_end++){
    if(message_in.charAt(index_end) == ','){
      break;
    }
  }
  if(index_end == message_in.length()){
    Serial.println("Error tx addr, no comma found!");
    Serial.println(message_in);
    return String("ERR");
  }
  String tx_addr = message_in.substring(index_start, index_end);
  index_end++;
  index_start = index_end;
  Serial.print("TX Addr: ");Serial.println(tx_addr);

  for( ; index_end < message_in.length(); index_end++){
    if(message_in.charAt(index_end) == ','){
      break;
    }
  }
  if(index_end == message_in.length()){
    Serial.println("Error parsing msg length, no comma found!");
    Serial.println(message_in);
    return String("ERR");
  }
  String msg_length = message_in.substring(index_start, index_end);
  index_end++;
  index_start = index_end;
  Serial.print("Msg Length: ");Serial.println(msg_length);

  for( ; index_end < message_in.length(); index_end++){
    if(message_in.charAt(index_end) == ','){
      if(index_end-index_start >= msg_length.toInt()){
        break;
      }
    }
  }
  if(index_end == message_in.length()){
    Serial.println("Error parsing msg data, no comma found!");
    Serial.println(message_in);
    return String("ERR");
  }
  String msg_data = message_in.substring(index_start, index_end);
  index_end++;
  index_start = index_end;
  Serial.print("Msg Data: ");Serial.println(msg_data);

  for( ; index_end < message_in.length(); index_end++){
    if(message_in.charAt(index_end) == ','){
      break;
    }
  }
  if(index_end == message_in.length()){
    Serial.println("Error parsing rssi, no comma found!");
    Serial.println(message_in);
    return String("ERR");
  }
  String rssi = message_in.substring(index_start, index_end);
  index_end++;
  index_start = index_end;
  Serial.print("RSSI: ");Serial.println(rssi);

  String snr = message_in.substring(index_start);
  Serial.print("SNR: ");Serial.println(rssi);

  return msg_data;  
}

bool lora_send_cmd(String cmd_str, String * output, int retries){
  String ret_str;
  char ret_char;
  
  for(uint8_t i = 0; i < retries; i++){

    lora.println(cmd_str);
    Serial.print("Sent cmd: ");Serial.println(cmd_str);
    delay(100);
    int ctr = 0;
    while(!lora.available()){
      ctr++;// wait for device to respond
      delay(1);
      if(ctr > 1000){
        Serial.println("Timeout while waiting for device!");
        break;
      }
    }
    if(lora.available()){
      ret_str = lora.readStringUntil('\n');
    }

    if(ret_str.length() == 0){
      Serial.println("No repsonse!");
    }
    else{
      Serial.print("Ret length: ");Serial.println(ret_str.length());
      Serial.print("Return: ");Serial.println(ret_str);
      return true;
    }
    ret_str.remove(0, ret_str.length());  //clean input string
    delay(500);
  }
  return false;
}

bool lora_send_and_verify(String cmd_str, uint8_t retries){

  String exp_ret = "+OK\r";
  String ret_str;
  char ret_char;
  
  for(uint8_t i = 0; i < retries; i++){
    lora.println(cmd_str);
    Serial.print("Sent cmd: ");Serial.println(cmd_str);
    delay(100);
    int ctr = 0;
    while(!lora.available()){
      ctr++;// wait for device to respond
      delay(1);
      if(ctr > 1000){
        Serial.println("Timeout while waiting for device!");
        break;
      }
    }
    if(lora.available()){
      ret_str = lora.readStringUntil('\n');
    }

    if(ret_str.length() == 0){
      Serial.println("No repsonse!");
    }
    else{
      Serial.print("Ret length: ");Serial.println(ret_str.length());
      Serial.print("Return: ");Serial.println(ret_str);
      Serial.print("Match = ");Serial.println(ret_str.equals(exp_ret));
      if(ret_str.equals(exp_ret)){
        return true;
      }
    }
    ret_str.remove(0, ret_str.length());  //clean input string
    delay(500);
  }
  return false;
}
