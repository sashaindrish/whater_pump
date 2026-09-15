#include <Arduino.h>
#include "PressureSensor.h"
#include "ConfigManager.h"
#include "PumpController.h"

// Пины и константы датчика
const int SENSOR_PIN = A0;     // Аналоговый вход
const float R = 220.0;         // Сопротивление, Ом

// Параметры фильтра Калмана (подбираются опытным путём)
const float Q = 0.01;          // Ковариация шума процесса (доверие к модели)
const float R_Kalman = 0.1;    // Ковариация шума измерения (доверие к датчику)
float x_est = 0.0;             // Оценка состояния (давление, бар)
float P_est = 1.0;             // Оценка ковариации ошибки

// Функция фильтра Калмана (возвращает отфильтрованное значение)
float kalmanFilter(float z) {
  // 1. Прогноз (экстраполяция)
  float x_pred = x_est;           // Состояние не меняется (модель: x_k = x_{k-1})
  float P_pred = P_est + Q;       // Увеличение неопределённости из-за процесса

  // 2. Коррекция (обновление по измерению)
  float K = P_pred / (P_pred + R_Kalman);  // Коэффициент Калмана
  x_est = x_pred + K * (z - x_pred);       // Обновление оценки
  P_est = (1.0 - K) * P_pred;              // Обновление ковариации

  return x_est;
}

void setup() {
  Serial.begin(9600);
  delay(1000);  // Стабилизация питания

  // Первое измерение для инициализации фильтра
  int adcFirst = analogRead(SENSOR_PIN);
  float voltFirst = adcFirst * (5.0 / 1023.0);
  float currFirst = (voltFirst / R) * 1000.0;
  x_est = (currFirst - 4.0) * (10.0 / 16.0);
  x_est = constrain(x_est, 0.0, 10.0);  // Начальная оценка
}

void loop() {
  // 1. Считывание и расчёт "сырого" давления
  int adcValue = analogRead(SENSOR_PIN);
  float voltage = adcValue * (5.0 / 1023.0);
  float current_mA = (voltage / R) * 1000.0;
  float rawPressure = (current_mA - 4.0) * (10.0 / 16.0);
  rawPressure = constrain(rawPressure, 0.0, 10.0);

  // 2. Применяем фильтр Калмана
  float filteredPressure = kalmanFilter(rawPressure);

  // 3. Вывод отфильтрованного значения в плоттер
  Serial.println(filteredPressure);

  // Задержка (можно уменьшить до 10-20 мс для большей частоты)
  delay(50);
}
