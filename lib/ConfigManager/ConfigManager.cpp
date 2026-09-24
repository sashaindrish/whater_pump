#include "ConfigManager.h"
#include <EEPROM.h>

// Магический маркер — ставим в самом начале EEPROM.
// Если его там нет, значит EEPROM «чистая» или от другой прошивки.
static const uint16_t MAGIC = 0xA5C3;
static const int      MAGIC_ADDR = 0;
static const int      DATA_ADDR  = 2;   // после маркера

ConfigManager::ConfigManager(int eepromAddr)
  : _addr(eepromAddr ? eepromAddr : DATA_ADDR) {}

void ConfigManager::_setDefaults() {
  _cfg.startPress = 3.0f;
  _cfg.onTime     = 15000UL;   // 15 с
  _cfg.offTime    = 45000UL;   // 45 с
}

bool ConfigManager::_isValid() const {
  if (isnan(_cfg.startPress) || isinf(_cfg.startPress)) return false;
  if (_cfg.startPress <= 0.0f || _cfg.startPress > 10.0f) return false;
  if (_cfg.onTime  == 0 || _cfg.onTime  > 3600000UL)  return false; // ≤ 1 ч
  if (_cfg.offTime == 0 || _cfg.offTime > 3600000UL)  return false;
  return true;
}

void ConfigManager::begin() { load(); }

void ConfigManager::load() {
  uint16_t magic = 0;
  EEPROM.get(MAGIC_ADDR, magic);

  if (magic != MAGIC) {
    // EEPROM чистая или от другой прошивки
    _setDefaults();
    save();
    return;
  }

  EEPROM.get(_addr, _cfg);

  if (!_isValid()) {
    // Данные повреждены или несовместимы — сбрасываем
    _setDefaults();
    save();
  }
}

void ConfigManager::save() {
  EEPROM.put(MAGIC_ADDR, MAGIC);
  EEPROM.put(_addr, _cfg);
}

void ConfigManager::reset() {
  _setDefaults();
  save();
}

void ConfigManager::print(Stream &out) {
  out.println(F("=== НАСТРОЙКИ ОЗОНИРОВАНИЯ ==="));
  out.print(F("P startPress, бар: ")); out.println(_cfg.startPress, 2);
  out.print(F("O onTime, мс: "));      out.println(_cfg.onTime);
  out.print(F("F offTime, мс: "));     out.println(_cfg.offTime);
  out.println(F("=============================="));
}

bool ConfigManager::handleLine(const String &cmd, Stream &serial) {
  if (cmd.length() == 0) return false;

  if (cmd.equalsIgnoreCase("show")) { print(serial); return true; }
  if (cmd.equalsIgnoreCase("save")) {
    save();
    serial.println(F("Настройки сохранены в EEPROM."));
    return true;
  }
  if (cmd.equalsIgnoreCase("R")) {
    reset();
    serial.println(F("Настройки сброшены к значениям по умолчанию."));
    print(serial);
    return true;
  }

  int eq = cmd.indexOf('=');
  if (eq <= 0) {
    serial.println(F("Формат: P=3.0 O=15000 F=45000, show, save, R"));
    return false;
  }

  String key = cmd.substring(0, eq);
  float  val = cmd.substring(eq + 1).toFloat();
  key.trim();
  key.toUpperCase();

  if      (key == "P") { _cfg.startPress = val; }
  else if (key == "O") { _cfg.onTime     = (unsigned long)val; }
  else if (key == "F") { _cfg.offTime    = (unsigned long)val; }
  else {
    serial.print(F("Неизвестный параметр: "));
    serial.println(key);
    return false;
  }

  if (!_isValid()) {
    serial.println(F("Значения вне допустимого диапазона, отменено."));
    load();
    return false;
  }

  save();
  serial.print(key); serial.print(F(" = ")); serial.println(val, 3);
  return true;
}