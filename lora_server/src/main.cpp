#include <SPI.h>
#include <RH_RF95.h>
#include "../../common_h/common.h"

RH_RF95 rf95;

#define BUTTON_PIN 3

int cmd = 0;
int button_state = LOW;
int cmd_state = CLIENT_STATE_RESET;

const uint8_t data[MSG_SIZE] = { '\0' }; // Initialize data with null characters

void parse_response(int a) {

  switch (a) {
    case 0:
      Serial.println("PING FROM CLIENT");
      break;
    case 1:
      Serial.println("RESET");
      break;
    case 2:
      Serial.println("START"); //unused on server
      break;
    case 3:
      Serial.println("STOP"); //unused on server
      break;
      case 4:
      Serial.println("CMD RECEIVED");
      break;
    default:
      Serial.println("UNKNOWN COMMAND");
      break;
  }

  return;
}


void setup() {
  Serial.begin(9600);

  pinMode(BUTTON_PIN, INPUT);

  if (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");
  if (!rf95.setFrequency(433.0)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  rf95.setTxPower(13, false);
}

void loop() {
  
  button_state = digitalRead(BUTTON_PIN);

  Serial.print("Server Status$ ");

  if(button_state == HIGH) {
    if (cmd_state == CLIENT_STATE_RESET) {
      cmd_state = CLIENT_STATE_START;
      strcpy((char*)data, START);
    } else if (cmd_state == CLIENT_STATE_START) {
      cmd_state = CLIENT_STATE_STOP;
      strcpy((char*)data, STOP); 
    } else if (cmd_state == CLIENT_STATE_STOP) {
      cmd_state = CLIENT_STATE_RESET; 
      strcpy((char*)data, RESET);
    }
    cmd = cmd_state;
  } else {
      strcpy((char*)data, PING);
  }
   //lets encrypt the data 
  XOR_CIPHER((uint8_t*)data, sizeof(data), (const uint8_t*)KEY, strlen(KEY));
  
  rf95.send(data, sizeof(data));
  
  rf95.waitPacketSent();

  uint8_t buf[MSG_SIZE];
  uint8_t len = MSG_SIZE;

  if (rf95.waitAvailableTimeout(3000)) {
    if (rf95.recv(buf, &len)) {

      // Decrypt the received data
      XOR_CIPHER(buf, len, (const uint8_t*)KEY, strlen(KEY));
      buf[len] = '\0'; // Null-terminate the string for printing

      cmd = atoi((char*)buf + PARSE_OFFSET);

      parse_response(cmd);

    } else {
      Serial.println("RECEIVE FAILED");
    }
  } else {
    Serial.println("CONNECTION DROP");
  }
  delay(TICK);
}