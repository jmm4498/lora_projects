#include <SPI.h>
#include <RH_RF95.h>
#include "../../common_h/common.h"

#define RX_LED_PIN 7 //why not
#define TX_LED_PIN 6 //dont care its pwm

RH_RF95 rf95;


void setup() {
  Serial.begin(9600);

  pinMode(RX_LED_PIN, OUTPUT);
  pinMode(TX_LED_PIN, OUTPUT);
  digitalWrite(RX_LED_PIN, LOW);
  digitalWrite(TX_LED_PIN, LOW);

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
  
  const uint8_t data[] = "CLIENT RECEIVED SIGNAL";  
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  digitalWrite(RX_LED_PIN, LOW); //turn it off
  digitalWrite(TX_LED_PIN, LOW);

  //lets encrypt the data 
  XOR_CIPHER((uint8_t*)data, sizeof(data), (const uint8_t*)KEY, strlen(KEY));

  if (rf95.waitAvailableTimeout(3000)) {
    
    if (rf95.recv(buf, &len)) {

      // Decrypt the received data
      XOR_CIPHER(buf, len, (const uint8_t*)KEY, strlen(KEY));
      buf[len] = '\0'; // Null-terminate the string for printing

      if(strcmp((char*)buf, "START SIGNAL") != 0) {
        Serial.println("UNEXPECTED MESSAGE RECEIVED"); //noise?
        Serial.println((char*)buf);
      }  else {
        Serial.print("MESSAGE RECEIVED FROM SERVER: ");
        Serial.println((char*)buf);

        digitalWrite(RX_LED_PIN, HIGH); //turn it on

        rf95.send(data, sizeof(data));
        rf95.waitPacketSent();

        digitalWrite(TX_LED_PIN, HIGH); //turn it on

        Serial.println("REPLY SENT TO SERVER");
      }
    } else {
      Serial.println("RECEIVE FAILED");
    }
  } else {
    Serial.println("NO MESSAGE FROM SERVER");
  }
  delay(1000);
}