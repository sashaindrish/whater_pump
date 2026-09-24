#include "OzonationController.h"

// Сколько давление должно непрерывно держаться выше startPress,
// чтобы выключить подачу озона
static const unsigned long STOP_HOLD_MS = 2000UL;

OzonationController::OzonationController(uint8_t pin)
  : _pin(pin), _output(false), _state(OZ_IDLE), _stateStart(0),
    _aboveActive(false), _aboveStart(0) {}

void OzonationController::begin() {
  pinMode(_pin, OUTPUT);
  _setOutput(false);
  _state       = OZ_IDLE;
  _stateStart  = millis();
  _aboveActive = false;
  _aboveStart  = 0;
}

void OzonationController::_setOutput(bool on) {
  _output = on;
  digitalWrite(_pin, on ? HIGH : LOW);
}

void OzonationController::_setState(OzoneState s, unsigned long now) {
  _state = s;
  _stateStart = now;
}

const char* OzonationController::getStateName() const {
  switch (_state) {
    case OZ_IDLE:     return "IDLE";
    case OZ_RUNNING:  return "RUN";
    case OZ_COOLDOWN: return "COOL";
  }
  return "?";
}

void OzonationController::update(float pressure, Config &cfg) {
  unsigned long now = millis();
  const bool startTrigger = (pressure <= cfg.startPress);
  const bool aboveStop    = (pressure >  cfg.startPress);   // ← без гистерезиса

  switch (_state) {

    // ---------- ожидание ----------
    case OZ_IDLE:
      if (startTrigger) {
        _setOutput(true);
        _setState(OZ_RUNNING, now);
        _aboveActive = false;
        _aboveStart  = 0;
        Serial.println(F("[OZ] Старт подачи"));
      }
      break;

    // ---------- подача озона ----------
    case OZ_RUNNING: {
      unsigned long elapsed = now - _stateStart;

      // --- таймер удержания выше порога ---
      if (aboveStop) {
        if (!_aboveActive) {
          _aboveActive = true;
          _aboveStart  = now;
        }
      } else {
        // давление упало обратно ниже порога — сбрасываем таймер
        _aboveActive = false;
        _aboveStart  = 0;
      }

      // 1) Давление держится выше порога >= 2 с → стоп
      if (_aboveActive && (now - _aboveStart >= STOP_HOLD_MS)) {
        _setOutput(false);
        _setState(OZ_COOLDOWN, now);
        _aboveActive = false;
        Serial.print(F("[OZ] Стоп (P>порога "));
        Serial.print(STOP_HOLD_MS / 1000.0f, 1);
        Serial.print(F(" с). Работа "));
        Serial.print(elapsed / 1000.0f, 1);
        Serial.println(F(" с"));
        break;
      }

      // 2) Таймаут работы — страховка
      if (elapsed >= cfg.onTime) {
        _setOutput(false);
        _setState(OZ_COOLDOWN, now);
        _aboveActive = false;
        Serial.print(F("[OZ] Стоп (таймаут). Работа "));
        Serial.print(cfg.onTime / 1000);
        Serial.println(F(" с"));
        break;
      }
      break;
    }

    // ---------- пауза ----------
    case OZ_COOLDOWN:
      if (now - _stateStart >= cfg.offTime) {
        _setState(OZ_IDLE, now);
        Serial.println(F("[OZ] Готов к новой подаче"));
      }
      break;
  }
}