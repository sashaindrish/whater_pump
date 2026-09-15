#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

#include <Arduino.h>

/*
 * Датчик давления 4–20 мА, подключён через резистор R на GND.
 * Напряжение снимается с резистора и подаётся на аналоговый вход Arduino.
 * Внутри — линейное преобразование и одномерный фильтр Калмана.
 */
class PressureSensor {
  public:
    PressureSensor(uint8_t pin, float resistorOhm);

    void  begin();                       // инициализация + первый отсчёт
    float readPressure();                // отфильтрованное значение, бар
    float readRawPressure();             // «сырое» значение, бар

    void  setKalmanQ(float q);           // шум процесса
    void  setKalmanR(float r);           // шум измерения

  private:
    uint8_t _pin;
    float   _R;        // сопротивление, Ом
    float   _Q;        // шум процесса
    float   _Rk;       // шум измерения
    float   _x_est;    // оценка состояния
    float   _P_est;    // оценка ковариации

    float _adcToPressure(int adc);
    float _kalman(float z);
};

#endif