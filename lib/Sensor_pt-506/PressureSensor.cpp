#include "PressureSensor.h"

PressureSensor::PressureSensor(uint8_t pin, float resistorOhm)
  : _pin(pin), _R(resistorOhm),
    _Q(0.01), _Rk(0.1), _x_est(0.0), _P_est(1.0) {}

void PressureSensor::begin() {
  pinMode(_pin, INPUT);
  int adc = analogRead(_pin);
  _x_est = _adcToPressure(adc);
  _P_est = 1.0;
}

void PressureSensor::setKalmanQ(float q) { _Q = q; }
void PressureSensor::setKalmanR(float r) { _Rk = r; }

float PressureSensor::_adcToPressure(int adc) {
  float voltage   = adc * (5.0f / 1023.0f);
  float currentMA = (voltage / _R) * 1000.0f;
  float pressure  = (currentMA - 4.0f) * (10.0f / 16.0f);
  return constrain(pressure, 0.0f, 10.0f);
}

float PressureSensor::_kalman(float z) {
  float xPred = _x_est;
  float pPred = _P_est + _Q;
  float k     = pPred / (pPred + _Rk);
  _x_est = xPred + k * (z - xPred);
  _P_est = (1.0f - k) * pPred;
  return _x_est;
}

float PressureSensor::readRawPressure() {
  return _adcToPressure(analogRead(_pin));
}

float PressureSensor::readPressure() {
  return _kalman(readRawPressure());
}