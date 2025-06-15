#include <SPI.h>
#include <RH_RF95.h>
#include "../../common_h/common.h"

#define RX_LED_PIN 7 //why not
#define TX_LED_PIN 4 //dont care its pwm

#define STATE_STOP_PIN 3
#define STATE_START_PIN 5
#define STATE_RESET_PIN 6

RH_RF95 rf95;

int cmd = 0;

void parse_response(int a) {

  switch (a) {
    case 0:
      Serial.println("PING FROM SERVER");
      break;
    case 1:
      Serial.println("RESET");
      digitalWrite(STATE_RESET_PIN, HIGH);
      digitalWrite(STATE_STOP_PIN, LOW);
      break;
    case 2:
      Serial.println("START");
      digitalWrite(STATE_RESET_PIN, LOW);
      digitalWrite(STATE_START_PIN, HIGH);
      break;
    case 3:
      Serial.println("STOP"); 
      digitalWrite(STATE_STOP_PIN, HIGH);
      digitalWrite(STATE_START_PIN, LOW);
      break;
    default:
      Serial.println("UNKNOWN COMMAND");
      break;
  }

  return;
}

void setup() {
  Serial.begin(9600);

  pinMode(RX_LED_PIN, OUTPUT);
  pinMode(TX_LED_PIN, OUTPUT);
  pinMode(STATE_RESET_PIN, OUTPUT);
  pinMode(STATE_START_PIN, OUTPUT);
  pinMode(STATE_STOP_PIN, OUTPUT);
  
  digitalWrite(RX_LED_PIN, LOW);
  digitalWrite(TX_LED_PIN, LOW);

  digitalWrite(STATE_STOP_PIN, LOW);
  digitalWrite(STATE_START_PIN, LOW);
  digitalWrite(STATE_RESET_PIN, HIGH);

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
  
  const uint8_t data[MSG_SIZE] = { '\0' }; // Initialize data with null characters
  strcpy((char*)data, RECEIVED);

  Serial.print("Client Status$ ");

  if(cmd == 0) {
    strcpy((char*)data, PING);
  } else if(cmd > 0 && cmd < 5) {
    strcpy((char*)data, RECEIVED);
  } 

  uint8_t buf[MSG_SIZE];
  uint8_t len = MSG_SIZE;

  digitalWrite(RX_LED_PIN, LOW); //turn it off
  digitalWrite(TX_LED_PIN, LOW);

  //lets encrypt the data 
  XOR_CIPHER((uint8_t*)data, sizeof(data), (const uint8_t*)KEY, strlen(KEY));

  if (rf95.waitAvailableTimeout(3000)) {
    
    digitalWrite(RX_LED_PIN, HIGH); //turn it on

    if (rf95.recv(buf, &len)) {

      // Decrypt the received data
      XOR_CIPHER(buf, len, (const uint8_t*)KEY, strlen(KEY));
      buf[len] = '\0'; // Null-terminate the string for printing

      cmd = atoi((char*)buf + PARSE_OFFSET); // Extract command from the received message

      parse_response(cmd);

      rf95.send(data, sizeof(data));
  
      rf95.waitPacketSent();
      digitalWrite(TX_LED_PIN, HIGH); //turn it on

    } else {

      Serial.println("RECEIVE FAILED");

    }
  } else {

    Serial.println("NO SERVER REQUEST");

  }
  delay(TICK);
}