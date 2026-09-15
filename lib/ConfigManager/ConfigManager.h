#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>

struct Config {
  float         lowPress;         // бар, порог включения
  float         highPress;        // бар, порог выключения
  unsigned long turnOnDelay;      // мс, задержка перед включением
  unsigned long maxRunTime;       // мс, макс. время работы
  unsigned long cooldownTime;     // мс, ожидание после аварии
  float         emergencyLowPress;// бар, аварийный нижний порог
};

class ConfigManager {
  public:
    explicit ConfigManager(int eepromAddr = 0);

    void      begin();                                  // читает EEPROM
    void      load();                                   // загрузка
    void      save();                                   // запись
    void      print(Stream &out);                       // вывод настроек

    // Обрабатывает одну строку команды из Serial (если она есть).
    // Возвращает true, если команда была распознана.
    bool      handleCommand(Stream &serial);

    Config&   get() { return _cfg; }

  private:
    int      _addr;
    Config   _cfg;

    void     _setDefaults();
};

#endif