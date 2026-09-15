#include "PumpController.h"

PumpController::PumpController(uint8_t pin)
  : _pin(pin), _output(false), _state(PUMP_OFF), _stateStart(0) {}

void PumpController::begin() {
  pinMode(_pin, OUTPUT);
  _setOutput(false);
  _state      = PUMP_OFF;
  _stateStart = millis();
}

void PumpController::_setOutput(bool on) {
  _output = on;
  digitalWrite(_pin, on ? HIGH : LOW);
}

void PumpController::_setState(PumpState s, unsigned long now) {
  _state      = s;
  _stateStart = now;
}

const char* PumpController::getStateName() const {
  switch (_state) {
    case PUMP_OFF:            return "OFF";
    case PUMP_TURN_ON_DELAY:  return "DELAY";
    case PUMP_ON:             return "ON";
    case PUMP_EMERGENCY_STOP: return "EMERGENCY";
  }
  return "?";
}

void PumpController::update(float pressure, Config &cfg) {
  unsigned long now = millis();

  const bool low         = (pressure <= cfg.lowPress);
  const bool high        = (pressure >= cfg.highPress);
  const bool emergencyLow = (pressure <  cfg.emergencyLowPress);

  switch (_state) {

    // ---------- Насос выключен ----------
    case PUMP_OFF:
      if (low) _setState(PUMP_TURN_ON_DELAY, now);
      break;

    // ---------- Ожидание перед включением ----------
    case PUMP_TURN_ON_DELAY:
      if (!low) {
        _setState(PUMP_OFF, now);                 // условие снято – отмена
      } else if (now - _stateStart >= cfg.turnOnDelay) {
        _setOutput(true);
        _setState(PUMP_ON, now);
      }
      break;

    // ---------- Насос работает ----------
    case PUMP_ON:
      if (now - _stateStart >= cfg.maxRunTime) {
        _setOutput(false);
        _setState(PUMP_EMERGENCY_STOP, now);
        Serial.println(F("[PUMP] Авария: превышено время работы"));
      } else if (emergencyLow) {
        _setOutput(false);
        _setState(PUMP_EMERGENCY_STOP, now);
        Serial.println(F("[PUMP] Авария: давление ниже аварийного порога"));
      } else if (high) {
        _setOutput(false);
        _setState(PUMP_OFF, now);                 // нормальное выключение
      }
      break;

    // ---------- Аварийное ожидание ----------
    case PUMP_EMERGENCY_STOP:
      if (now - _stateStart >= cfg.cooldownTime) {
        _setState(PUMP_OFF, now);
        Serial.println(F("[PUMP] Cooldown завершён, система готова"));
      }
      break;
  }
}