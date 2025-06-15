#include <SPI.h>
#include <RH_RF95.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "../../common_h/common.h"

RH_RF95 rf95;
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define BUTTON_PIN 3

int client_response = 0;
volatile int button_pressed = 0;
volatile int cmd_state = CLIENT_STATE_RESET;
volatile int last_cmd_state = CLIENT_STATE_RESET;

uint8_t buf[MSG_SIZE];
uint8_t len = MSG_SIZE;
const uint8_t data[MSG_SIZE] = { '\0' }; // Initialize data with null characters

void parse_response(int a) {

  switch (a) {
    case 0:
      Serial.println("PING FROM CLIENT");
      lcd.setCursor(0, 1);
      lcd.print("CONNECTION OKAY");
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
      lcd.setCursor(0, 0);
      lcd.print("CMD RECEIVED");
      break;
    default:
      Serial.println("UNKNOWN COMMAND");
      break;
  }

  return;
}

//interrupt function when button is pressed
void button_ISR() {
  
  button_pressed = 1;

  switch(cmd_state) {
    case CLIENT_STATE_STOP:
      cmd_state = CLIENT_STATE_RESET;
      break;
    case CLIENT_STATE_RESET:
      cmd_state = CLIENT_STATE_START;
      break;
    case CLIENT_STATE_START:
      cmd_state = CLIENT_STATE_STOP;
      break;
    default:
      break;
  }

  return;
}


void setup() {
  Serial.begin(9600);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), button_ISR, FALLING);

  lcd.init();
  lcd.backlight();

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
  
  memset(buf, 0, sizeof(buf)); // Clear the buffer

  Serial.print("Server Status$ ");

  if(button_pressed) {
    //process user command
    if (cmd_state == CLIENT_STATE_START) {
      strcpy((char*)data, START);
    } else if (cmd_state == CLIENT_STATE_STOP) {
      strcpy((char*)data, STOP); 
    } else if (cmd_state == CLIENT_STATE_RESET) {
      strcpy((char*)data, RESET);
    }
    button_pressed = 0;
  } else {
    //no user command, send ping
    strcpy((char*)data, PING);
  }

  //lets encrypt the data 
  XOR_CIPHER((uint8_t*)data, sizeof(data), (const uint8_t*)KEY, strlen(KEY));

  rf95.send(data, sizeof(data));

  rf95.waitPacketSent();

  if (rf95.waitAvailableTimeout(3000)) {
    if (rf95.recv(buf, &len)) {

      // Decrypt the received data
      XOR_CIPHER(buf, len, (const uint8_t*)KEY, strlen(KEY));
      buf[len] = '\0'; // Null-terminate the string for printing

      client_response = atoi((char*)buf + PARSE_OFFSET);

      parse_response(client_response);

    } else {
      Serial.println("DRIVER RECEIVE FAILED");
    }
  } else {
    lcd.setCursor(0, 1);
    lcd.print("CONNECTION DROP");
    Serial.println("CONNECTION DROP");
  }


  delay(TICK);
}