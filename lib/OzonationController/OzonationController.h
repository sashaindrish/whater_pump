#ifndef OZONATION_CONTROLLER_H
#define OZONATION_CONTROLLER_H

#include <Arduino.h>
#include "ConfigManager.h"

enum OzoneState {
  OZ_IDLE,
  OZ_RUNNING,
  OZ_COOLDOWN
};

class OzonationController {
  public:
    explicit OzonationController(uint8_t pin);

    void        begin();
    void        update(float pressure, Config &cfg);

    bool        isOn() const        { return _output; }
    OzoneState  getState() const    { return _state; }
    const char* getStateName() const;

  private:
    uint8_t       _pin;
    bool          _output;
    OzoneState    _state;
    unsigned long _stateStart;

    // Таймер удержания давления выше порога
    bool          _aboveActive;       // true, если давление сейчас выше порога
    unsigned long _aboveStart;        // момент, когда оно впервые превысило порог

    void _setState(OzoneState s, unsigned long now);
    void _setOutput(bool on);
};

#endif