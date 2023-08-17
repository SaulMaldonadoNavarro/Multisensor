#include <SPI.h>
#include <LoRa.h>

#define ss 5
#define rst 14
#define dio0 2


void setup() {
  Serial.begin(9600);
  Serial.println("Nodo Puerta de enlace");

  LoRa.setPins(ss,rst,dio0);
  if (!LoRa.begin(915E6)) {
    Serial.println("Error al iniciar!");
    while (1);
  }

   // contraseña
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa OK!");
}

void loop() {
  String LoRaData;
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    while (LoRa.available()) {
      LoRaData.concat(LoRa.readString()); 
    }
    Serial.println(LoRaData);
  }
}
