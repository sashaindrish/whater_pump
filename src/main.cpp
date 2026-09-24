#include <Arduino.h>
#include "PressureSensor.h"
#include "ConfigManager.h"
#include "OzonationController.h"

const uint8_t SENSOR_PIN   = A0;
const uint8_t PUMP_PIN     = 2;
const float   RESISTOR_OHM = 220.0f;

PressureSensor      sensor(SENSOR_PIN, RESISTOR_OHM);
ConfigManager       cfgMgr;
OzonationController ozone(PUMP_PIN);

const unsigned long PRINT_PERIOD_MS = 50;
unsigned long lastPrint = 0;

void setup() {
  Serial.begin(9600);
  delay(1000);

  sensor.begin();
  cfgMgr.begin();
  ozone.begin();

  sensor.setKalmanQ(0.005f);
  sensor.setKalmanR(0.15f);

  Serial.println(F("Система озонирования запущена."));
  cfgMgr.print(Serial);
}

void loop() {
  // 1) UART-команды
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cfgMgr.handleLine(cmd, Serial);
  }

  // 2) Давление
  float pressure = sensor.readPressure();

  // 3) Логика
  ozone.update(pressure, cfgMgr.get());

  // 4) Плоттер: давление, вкл/выкл, состояние (0..2)
  unsigned long now = millis();
  if (now - lastPrint >= PRINT_PERIOD_MS) {
    lastPrint = now;
    Serial.print(pressure, 2);
    Serial.print(' ');
    Serial.print(ozone.isOn() ? 1 : 0);
    Serial.print(' ');
    Serial.println((int)ozone.getState());
  }
}