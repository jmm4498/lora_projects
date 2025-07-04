#include <SPI.h>
#include <RH_RF95.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "../../common_h/common.h"

RH_RF95 rf95;
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define BUTTON_PIN 3

const int max_status_ticks = 10;

int status_ticks = 0;
int client_response = 0;
volatile int button_pressed = 0;
volatile int client_state = CLIENT_STATE_RESET;
volatile int server_state = CLIENT_STATE_RESET;

uint8_t buf[MSG_SIZE];
uint8_t len = MSG_SIZE;
const uint8_t cmd_buffer[MSG_SIZE] = { '\0' }; // Initialize data with null characters
char stat_buf[18] = { '\0' }; // Buffer for status messages

const int status_cyles = 15;
int status_cycle_count = 0;
volatile int last_status = 0;

void parse_response(int response, int state) {

  switch (response) {
    case 0:
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
      status_cycle_count = 0; //reset the counter if a new status is received
      Serial.println("CMD RECEIVED");
      break;
    default:
      Serial.println("UNKNOWN COMMAND");
      break;
  }

  if(status_cycle_count == 0) { //'first' time we saw this status, lets print it for a few cycles before resetting
    last_status = response;
  }

  if(status_cycle_count++ >= status_cyles) {
    status_cycle_count = 0;
  }

  sprintf(stat_buf, "STATUS:%d,STATE:%d", last_status, state);

  lcd.setCursor(0, 0);
  lcd.print(stat_buf);


  return;
}

//interrupt function when button is pressed
void button_ISR() {
  
  button_pressed = 1;

  switch(client_state) {
    case CLIENT_STATE_STOP:
      server_state = CLIENT_STATE_RESET;
      break;
    case CLIENT_STATE_RESET:
      server_state = CLIENT_STATE_START;
      break;
    case CLIENT_STATE_START:
      server_state = CLIENT_STATE_STOP;
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
    if (client_state == CLIENT_STATE_START) {
      strcpy((char*)cmd_buffer, STOP);
    } else if (client_state == CLIENT_STATE_STOP) {
      strcpy((char*)cmd_buffer, RESET); 
    } else if (client_state == CLIENT_STATE_RESET) {
      strcpy((char*)cmd_buffer, START);
    }
    button_pressed = 0;
  } else {
    //no user command, send ping
    strcpy((char*)cmd_buffer, PING);
  }

  //lets encrypt the data 
  XOR_CIPHER((uint8_t*)cmd_buffer, sizeof(cmd_buffer), (const uint8_t*)KEY, strlen(KEY));

  rf95.send(cmd_buffer, sizeof(cmd_buffer));

  rf95.waitPacketSent();

  if (rf95.waitAvailableTimeout(3000)) {
    if (rf95.recv(buf, &len)) {

      // Decrypt the received data
      XOR_CIPHER(buf, len, (const uint8_t*)KEY, strlen(KEY));
      buf[len] = '\0'; // Null-terminate the string for printing

      //client_response = buf[PARSE_OFFSET_ID]; //last digit is command ID
      client_response = atoi((char*)buf + PARSE_OFFSET_ID); //last digit is command ID
      client_state = atoi((char*)buf + PARSE_OFFSET_STATE); //first digit is client LED state (START, STOP, RESET)

      parse_response(client_response, client_state);

    } else {
      Serial.println("DRIVER RECEIVE FAILED");
    }
  } else {
    lcd.setCursor(0, 1);
    lcd.print("CONNECTION DROP");
    //Serial.println("CONNECTION DROP");
  }


  delay(TICK);
}