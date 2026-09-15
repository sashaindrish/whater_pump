#ifndef PUMP_CONTROLLER_H
#define PUMP_CONTROLLER_H

#include <Arduino.h>
#include "ConfigManager.h"

enum PumpState {
  PUMP_OFF,
  PUMP_TURN_ON_DELAY,
  PUMP_ON,
  PUMP_EMERGENCY_STOP
};

class PumpController {
  public:
    explicit PumpController(uint8_t pin);

    void       begin();
    // Вызывать в loop(): обновляет автомат состояний по давлению и настройкам.
    void       update(float pressure, Config &cfg);

    bool       isOn() const        { return _output; }
    PumpState  getState() const    { return _state; }
    const char* getStateName() const;

  private:
    uint8_t       _pin;
    bool          _output;      // фактическое состояние выхода
    PumpState     _state;
    unsigned long _stateStart;  // millis() момента входа в состояние

    void _setState(PumpState s, unsigned long now);
    void _setOutput(bool on);
};

#endif