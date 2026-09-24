#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>

struct Config {
  float         startPress;   // бар, порог запуска (3.0)
  unsigned long onTime;       // мс, время подачи озона (15000)
  unsigned long offTime;      // мс, пауза между подачами (45000)
};

class ConfigManager {
  public:
    explicit ConfigManager(int eepromAddr = 0);

    void      begin();
    void      load();
    void      save();
    void      reset();          // сброс к значениям по умолчанию
    void      print(Stream &out);

    bool      handleLine(const String &cmd, Stream &serial);
    Config&   get() { return _cfg; }

  private:
    int      _addr;
    Config   _cfg;

    void     _setDefaults();
    bool     _isValid() const;
};

#endif