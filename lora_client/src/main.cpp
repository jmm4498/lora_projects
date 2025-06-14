#include <SPI.h>
#include <RH_RF95.h>


RH_RF95 rf95;
void setup() {
  Serial.begin(9600);
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

  if (rf95.waitAvailableTimeout(3000)) {
    
    if (rf95.recv(buf, &len)) {
      Serial.print("MESSAGE RECEIVED FROM SERVER: ");
      Serial.println((char*)buf);

      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      Serial.println("REPLY SENT TO SERVER");
    } else {
      Serial.println("RECEIVE FAILED");
    }
  } else {
    Serial.println("NO REPLY, IS THERE A SERVER AROUND?");
  }
  delay(1000);
}