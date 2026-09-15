#include "ConfigManager.h"
#include <EEPROM.h>

ConfigManager::ConfigManager(int eepromAddr) : _addr(eepromAddr) {}

void ConfigManager::_setDefaults() {
  _cfg.lowPress          = 2.0f;
  _cfg.highPress         = 4.0f;
  _cfg.turnOnDelay       = 100;
  _cfg.maxRunTime        = 5000;
  _cfg.cooldownTime      = 3000;
  _cfg.emergencyLowPress = 0.8f;
}

void ConfigManager::begin() {
  load();
}

void ConfigManager::load() {
  EEPROM.get(_addr, _cfg);
  // Если EEPROM «чистая» — задаём значения по умолчанию
  if (_cfg.lowPress == 0.0f && _cfg.highPress == 0.0f) {
    _setDefaults();
    save();
  }
}

void ConfigManager::save() {
  EEPROM.put(_addr, _cfg);
}

void ConfigManager::print(Stream &out) {
  out.println(F("=== ТЕКУЩИЕ НАСТРОЙКИ ==="));
  out.print(F("L (нижний порог, бар): "));    out.println(_cfg.lowPress, 2);
  out.print(F("H (верхний порог, бар): "));   out.println(_cfg.highPress, 2);
  out.print(F("D (задержка, мс): "));         out.println(_cfg.turnOnDelay);
  out.print(F("M (макс. время, мс): "));      out.println(_cfg.maxRunTime);
  out.print(F("C (cooldown, мс): "));         out.println(_cfg.cooldownTime);
  out.print(F("E (аварийный порог, бар): ")); out.println(_cfg.emergencyLowPress, 2);
  out.println(F("========================="));
}

bool ConfigManager::handleCommand(Stream &serial) {
  if (!serial.available()) return false;

  String cmd = serial.readStringUntil('\n');
  cmd.trim();
  if (cmd.length() == 0) return false;

  if (cmd.equalsIgnoreCase("show")) { print(serial); return true; }
  if (cmd.equalsIgnoreCase("save")) {
    save();
    serial.println(F("Настройки сохранены в EEPROM."));
    return true;
  }

  int eq = cmd.indexOf('=');
  if (eq <= 0) {
    serial.println(F("Неверный формат. Пример: L=2.5, H=4.0, D=200, M=5000, C=3000, E=0.8"));
    return false;
  }

  String key = cmd.substring(0, eq);
  float  val = cmd.substring(eq + 1).toFloat();
  key.trim();
  key.toUpperCase();

  if      (key == "L") { _cfg.lowPress          = val; }
  else if (key == "H") { _cfg.highPress         = val; }
  else if (key == "D") { _cfg.turnOnDelay       = (unsigned long)val; }
  else if (key == "M") { _cfg.maxRunTime        = (unsigned long)val; }
  else if (key == "C") { _cfg.cooldownTime      = (unsigned long)val; }
  else if (key == "E") { _cfg.emergencyLowPress = val; }
  else {
    serial.print(F("Неизвестный параметр: "));
    serial.println(key);
    return false;
  }

  save();
  serial.print(key); serial.print(F(" = ")); serial.println(val, 3);
  return true;
}