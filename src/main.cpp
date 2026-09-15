#include <Arduino.h>
#include "PressureSensor.h"
#include "ConfigManager.h"
#include "PumpController.h"

// ---- Аппаратные пины ----
const uint8_t SENSOR_PIN   = A0;
const uint8_t PUMP_PIN     = 2;

// ---- Номинал резистора ----
const float   RESISTOR_OHM = 220.0f;

// ---- Объекты ----
PressureSensor  sensor(SENSOR_PIN, RESISTOR_OHM);
ConfigManager   configManager;
PumpController  pump(PUMP_PIN);

// ---- Период вывода в плоттер ----
const unsigned long PRINT_PERIOD_MS = 50;
unsigned long lastPrint = 0;

void setup() {
  Serial.begin(9600);
  delay(1000);                 // стабилизация питания датчика

  sensor.begin();
  configManager.begin();
  pump.begin();

  Serial.println(F("Система хлорирования запущена."));
  configManager.print(Serial);
}

void loop() {
  // 1. Настройки по UART
  configManager.handleCommand(Serial);

  // 2. Давление (отфильтрованное)
  float pressure = sensor.readPressure();

  // 3. Управление насосом
  pump.update(pressure, configManager.get());

  // 4. Вывод в плоттер Arduino IDE 2.x: "<давление> <насос 0/1>"
  unsigned long now = millis();
  if (now - lastPrint >= PRINT_PERIOD_MS) {
    lastPrint = now;
    Serial.print(pressure, 2);
    Serial.print(' ');
    Serial.println(pump.isOn() ? 1 : 0);
  }
}