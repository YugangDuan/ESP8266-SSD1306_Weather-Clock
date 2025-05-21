
#pragma once
#include <ArduinoJson.h>

typedef struct HeFengCurrentData {

  String cond_txt;
  String fl;
  String tmp;
  String hum;
  String wind_sc;
  String iconMeteCon;
  String follower;
}
HeFengCurrentData;
typedef struct HeFengForeData {
  String dateStr;
  String tmp_min;
  String tmp_max;
  String iconMeteCon;

}
HeFengForeData;
class HeFeng {
  private:
    String getMeteConIcon(String cond_code);
    bool fetchBuffer(const char* url);
    static uint8_t _buffer[1024 * 3]; //gzip流最大缓冲区
    static size_t _bufferSize;
  public:
    HeFeng();
    void doUpdateCurr(HeFengCurrentData *data, String key, String location);
    void doUpdateFore(HeFengForeData *data, String key, String location);
    void fans(HeFengCurrentData *data, String id);
};

