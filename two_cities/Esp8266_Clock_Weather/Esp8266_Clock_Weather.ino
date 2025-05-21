#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <DNSServer.h>//密码直连将其三个库注释
#include <ESP8266WebServer.h>
#include <CustomWiFiManager.h>

#include <time.h>
#include <sys/time.h>                  
#include <coredecls.h>      

#include "SSD1306Wire.h"    //0.96寸用这个
#include "OLEDDisplayUi.h"
#include "HeFeng.h"
#include "WeatherStationFonts.h"
#include "WeatherStationImages.h"

/***************************
   Begin Settings
 **************************/
// const char* WIFI_SSID = "Huayun";  //填写你的WIFI名称及密码
// const char* WIFI_PWD = "huayun123";

const char* BILIBILIID = "97470772";  //填写你的B站账号
const char* HEFENG_KEY = "8188";//和风天气秘钥
const char* HEFENG_LOCATION =   "101010700";//城市ID,可到 https://github.com/qwd/LocationList/blob/master/China-City-List-latest.csv 查询
const char* HEFENG_LOCATION2 = "101090101";
// const char* HEFENG_LOCATION = "auto_ip";

#define TZ              8      // 中国时区为8
#define DST_MN          0      // 默认为0

const int UPDATE_INTERVAL_SECS = 30 * 60; // 30分钟更新一次天气
const int UPDATE_CURR_INTERVAL_SECS = 2 * 59; // 2分钟更新一次粉丝数

const int I2C_DISPLAY_ADDRESS = 0x3c;  //I2c地址默认
#if defined(ESP8266)
const int SDA_PIN = 0;  //引脚连接12  0
const int SDC_PIN = 2;  //14  2
#endif

const String WDAY_NAMES[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};  //星期
const String MONTH_NAMES[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};  //月份

// web配网页面自定义我的图标请随便使用一个图片转base64工具转换https://tool.css-js.com/base64.html, 64*64
const char Icon[] PROGMEM = "data:image/jpeg;base64,/9j/4AAQSkZJRgABAQEASABIAAD/4QAiRXhpZgAATU0AKgAAAAgAAQESAAMAAAABAAEAAAAAAAD/2wBDAAYEBQYFBAYGBQYHBwYIChAKCgkJChQODwwQFxQYGBcUFhYaHSUfGhsjHBYWICwgIyYnKSopGR8tMC0oMCUoKSj/2wBDAQcHBwoIChMKChMoGhYaKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCj/wAARCABAAEADASIAAhEBAxEB/8QAHwAAAQUBAQEBAQEAAAAAAAAAAAECAwQFBgcICQoL/8QAtRAAAgEDAwIEAwUFBAQAAAF9AQIDAAQRBRIhMUEGE1FhByJxFDKBkaEII0KxwRVS0fAkM2JyggkKFhcYGRolJicoKSo0NTY3ODk6Q0RFRkdISUpTVFVWV1hZWmNkZWZnaGlqc3R1dnd4eXqDhIWGh4iJipKTlJWWl5iZmqKjpKWmp6ipqrKztLW2t7i5usLDxMXGx8jJytLT1NXW19jZ2uHi4+Tl5ufo6erx8vP09fb3+Pn6/8QAHwEAAwEBAQEBAQEBAQAAAAAAAAECAwQFBgcICQoL/8QAtREAAgECBAQDBAcFBAQAAQJ3AAECAxEEBSExBhJBUQdhcRMiMoEIFEKRobHBCSMzUvAVYnLRChYkNOEl8RcYGRomJygpKjU2Nzg5OkNERUZHSElKU1RVVldYWVpjZGVmZ2hpanN0dXZ3eHl6goOEhYaHiImKkpOUlZaXmJmaoqOkpaanqKmqsrO0tba3uLm6wsPExcbHyMnK0tPU1dbX2Nna4uPk5ebn6Onq8vP09fb3+Pn6/9oADAMBAAIRAxEAPwCl8W7E2Oo2lnuZ2it1CvuL5XqDnH1rhrtRG6zp8jhs7M8Ed695T4aXF9IG1fXGuVRPKRAOUXGAM+nXtVbU/hVY2r/a1LXgTdmIjnBAx9cYrCdKUU5Gb3PLL281bWtH0q10m22T2MzNDcE4YxscshbPQdQeort7aGOCHzEjCvLGDvfkswzuOc85NLfOiLHBCqwwo4GNhJXkjpirOsaVNbaeoYzIjA4mdThsjsf6CvKqVZ4hWWiLOfuYruW5hCxiJpCfKZmJBGDkY9yPwq/Y3F/bW3m6oY1tFdZDGs21nXcQRjvn19q2oDLHocFxeXgEflqkMaA5A55ODnJ6VU03UNFgl0/+0LbzpYz8oHByG43AnBxn6V58qrmn7t7dilFLqZF8ki3YRLd7VXZJ4YpVGW5J4ByeuKkltoP7YZ/EE4IKB5vs7Esx4+UkAgdfyrSvtajj8S3F5pzQiWVVQtMckHcPXsenHpVO/mt7/V3shbQxxxw/u/scXzPIMHB9fmzTUqja5lbT5oVorZlqbXYtKhczX9xJIxzlH6H29qfoHxbsJLt7LVbryHQ4Eu0kfQ4rmvGmnrawSTzR7mPRz0A/E5H1rkfD2j6nqGmeTp8sDW0swkmidfmJ55z1/pX2FKLk7XFUaij6f8LeItN1tpP7Fu7a5mXBdlXkemTim+JtGvtVvLVJ7xVsVbfPGRlmA6BfTnv2rg/h/r2m+CdPXS7hYzI8hfejA7ASOPevS31m2nZCWTJwc56DvTlZ+6xK8dUeZeLdItdCuXMaz3NrNEWTIyUbPcd/WuOsrgQ2TXU8zRAfutroSzg5yR6V7X4itbK5hTzmLIDww5wPWvLvF2mi4uUtIWle3B+VmGNxwMH6ZrysRQhGbiuv3AldGPYQaZeaLeXUlxEtykq+SrPy6jr8o69am0WO4h1OM20qxy48wrNkAjv+nNbWqeFdHtZ4gl6sSxW6q5jRf3jqeowCee/0rldXupriV5ptv2l3YiKMjCp2xnpgA8fSvPpTVeMrXs+628hyXs9z0Hxp4fhfR2YjcxGQMHj65rxmx1W4s4pbWI+SFYgkDJNe0eK75rwlA/C8YAzn8K8l1a0jjkaQqIlORtPrX0BZm3M32sKJskKcj5cHPrXr3h29eXT7Nt+CECDcc88YJ/SvLrS3Rpo/lLbjkADOD6mu1hhmaxt0G6OPOeKTZLO9t783o8p2KICQj++Ocf59R6VgXF3Db3ssV3hbnny3RuGXs1dj4JFqbaeOdQXb96oK/d3AE4/GuN8WyR2yXaS2n2eO4kVBdtx5AyflGf4T7e9ceKU3qRoUNRurSd0SxVNs1vu3mQgs2Mn1wQfTrXNWQsZplkuUmeL5UdiDhNzdcg4ztz1qnqn2Y3EEa3JNvGmxJMYXPOTyec/1rFvdWeK0a1Nvli/7uRJNqntwO/t06muWCaju/mS3eR6bcEOAu/Yh5baeSKy7qwtzGkskeSOQnXn3Prinrdc7ivPU/wBBSSXREOYhmRj+VeublRNPkubgPxDHkDC9cCu4sIYHsEt2YNInTHWuQs1e6YeZJtIOODiuj01hDjY2e3PWhEyR0+lRCNYXXjAwfwP+GK1PEGhReItPEMhJUKQUzgNkfz6flWbYHdBg9jmt6xuCgABx2q+VSVmZM8G8VeG/+EegtlvYh56MZAmAVkAYYHHQ4rmZbIOm63jWa5kfzESElhCp6qQRnIr6Z8T6b/bOmtCgRZGBzIVBKjHb3zivA9Rtx4c/tS2s2kuAzhWRxjHGSc4Ofw9a4KtP2ctCHof/2Q==";
/***************************
   End Settings
 **************************/
 
SSD1306Wire     display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);   // 0.96寸用这个
OLEDDisplayUi   ui( &display );

HeFengCurrentData currentWeather; //实例化对象
HeFengForeData foreWeather[3];
HeFengCurrentData currentWeather2; // 第二城市实况天气
HeFengForeData foreWeather2[3];    // 第二城市天气预报
HeFeng HeFengClient;

#define TZ_MN           ((TZ)*60)   //时间换算
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)

time_t now; //实例化时间

String fans = "-1"; //粉丝数

bool readyForWeatherUpdate = false; // 天气更新标志
bool first = true;  //首次更新标志
long timeSinceLastWUpdate = 0;    //上次更新后的时间
long timeSinceLastCurrUpdate = 0;   //上次天气更新后的时间

void drawProgress(OLEDDisplay *display, int percentage, String label);   //提前声明函数
void updateData(OLEDDisplay *display);
void updateDatas(OLEDDisplay *display);
void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawCurrentWeather2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecast2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex);
void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state);
void setReadyForWeatherUpdate();

//添加框架
//此数组保留指向所有帧的函数指针
//框架是从右向左滑动的单个视图
FrameCallback frames[] = { drawDateTime, drawCurrentWeather, drawForecast, drawCurrentWeather2, drawForecast2};
//页面数量
int numberOfFrames = 5;

OverlayCallback overlays[] = { drawHeaderOverlay }; //覆盖回调函数
int numberOfOverlays = 1;  //覆盖数

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // 屏幕初始化
  display.init();
  display.clear();
  display.display();

  // display.flipScreenVertically(); //屏幕翻转
  display.setContrast(120); //屏幕亮度

  // wifiConnect();
  webconnect();
  ui.setTargetFPS(30);  //刷新频率

  ui.setActiveSymbol(activeSymbole); //设置活动符号
  ui.setInactiveSymbol(inactiveSymbole); //设置非活动符号
  // 符号位置
  // 你可以把这个改成TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);
  // 定义第一帧在栏中的位置
  ui.setIndicatorDirection(LEFT_RIGHT);
  // 屏幕切换方向
  // 您可以更改使用的屏幕切换方向 SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);
  ui.setFrames(frames, numberOfFrames); // 设置框架
  ui.setTimePerFrame(5000); //设置切换时间
  ui.setOverlays(overlays, numberOfOverlays); //设置覆盖
  // UI负责初始化显示
  ui.init();
  // display.flipScreenVertically(); //屏幕反转

  configTime(TZ_SEC, DST_SEC, "ntp.ntsc.ac.cn", "ntp1.aliyun.com"); //ntp获取时间，你也可用其他"pool.ntp.org","0.cn.pool.ntp.org","1.cn.pool.ntp.org","ntp1.aliyun.com"
  delay(200);
}

void loop() {
  if (first) {  //首次加载
    updateDatas(&display);
    first = false;
  }
  if (millis() - timeSinceLastWUpdate > (1000L * UPDATE_INTERVAL_SECS)) { //屏幕刷新
    setReadyForWeatherUpdate();
    timeSinceLastWUpdate = millis();
  }
  if (millis() - timeSinceLastCurrUpdate > (1000L * UPDATE_CURR_INTERVAL_SECS)) { //粉丝数更新
    HeFengClient.fans(&currentWeather, BILIBILIID);
    fans = String(currentWeather.follower);
    timeSinceLastCurrUpdate = millis();
  }
  if (readyForWeatherUpdate && ui.getUiState()->frameState == FIXED) { //天气更新
    updateData();
  }

  int remainingTimeBudget = ui.update(); //剩余时间预算

  if (remainingTimeBudget > 0) {
    //你可以在这里工作如果你低于你的时间预算。
    delay(remainingTimeBudget);
  }
  
}



void webconnect() {
  // 播放几秒钟动画，3秒钟约显示一圈
  const unsigned char* frames[] = {
    bili_Logo_5, bili_Logo_6, bili_Logo_7, bili_Logo_8,
    bili_Logo_1, bili_Logo_2, bili_Logo_3, bili_Logo_4
  };
  const int frameCount = sizeof(frames) / sizeof(frames[0]);

  for (int i = 0; i < 2; i++) {  // 播放2轮动画（大约3秒）
    for (int j = 0; j < frameCount; j++) {
      display.clear();
      display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, frames[j]);
      display.display();
      delay(80);
    }
  }

  // 初始化 WiFiManager
  WiFiManager wifiManager;  //实例化WiFiManager
  wifiManager.setDebugOutput(false);  //关闭Debug
  wifiManager.setHeadImgBase64(FPSTR(Icon));  //设置图标
  wifiManager.setPageTitle("欢迎来到JOSHUA的WiFi配置页");//设置页标题

  if (!wifiManager.autoConnect("JOSHUA-IOT-Display")) {//AP模式
    Serial.println("连接失败并超时");
    ESP.restart();//重新设置并再试一次，或者让它进入深度睡眠状态
    delay(1000);
  }

  Serial.println("connected...^_^");
}



void drawProgress(OLEDDisplay *display, int percentage, String label) {    //绘制进度
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 10, label);
  display->drawProgressBar(2, 28, 124, 10, percentage);
  display->display();
}

void updateData(void) {  //天气更新
  HeFengClient.doUpdateCurr(&currentWeather, HEFENG_KEY, HEFENG_LOCATION);
  delay(1500);  // <== 关键：延迟1.5秒再发第二个请求
  HeFengClient.doUpdateFore(foreWeather, HEFENG_KEY, HEFENG_LOCATION);
  delay(1500);  // <== 关键：延迟1.5秒再发第二个请求
  HeFengClient.doUpdateCurr(&currentWeather2, HEFENG_KEY, HEFENG_LOCATION2);  // 第二城市
  delay(1500);  // <== 关键：延迟1.5秒再发第二个请求
  HeFengClient.doUpdateFore(foreWeather2, HEFENG_KEY, HEFENG_LOCATION2);      // 第二城市
  readyForWeatherUpdate = false;
}

void updateDatas(OLEDDisplay *display) {  //首次天气更新
  drawProgress(display, 0, "Updating fansnumb...");
  HeFengClient.fans(&currentWeather, BILIBILIID);
  fans = String(currentWeather.follower);

  drawProgress(display, 20, "Updating weather...");
  HeFengClient.doUpdateCurr(&currentWeather, HEFENG_KEY, HEFENG_LOCATION);
  delay(1500);  // <== 关键：延迟1.5秒再发第二个请求
  drawProgress(display, 40, "Updating forecasts...");
  HeFengClient.doUpdateFore(foreWeather, HEFENG_KEY, HEFENG_LOCATION);
  delay(1500);  // <== 关键：延迟1.5秒再发第二个请求
  drawProgress(display, 60, "Updating weather city2...");
  HeFengClient.doUpdateCurr(&currentWeather2, HEFENG_KEY, HEFENG_LOCATION2);
  delay(1500);  // <== 关键：延迟1.5秒再发第二个请求
  drawProgress(display, 80, "Updating forecast city2...");
  HeFengClient.doUpdateFore(foreWeather2, HEFENG_KEY, HEFENG_LOCATION2);
  
  readyForWeatherUpdate = false;
  drawProgress(display, 100, "Done...");
  delay(200);
  
}

void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {  //显示时间
  now = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&now);
  char buff[16];

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  String date = WDAY_NAMES[timeInfo->tm_wday];

  sprintf_P(buff, PSTR("%04d-%02d-%02d  %s"), timeInfo->tm_year + 1900, timeInfo->tm_mon + 1, timeInfo->tm_mday, WDAY_NAMES[timeInfo->tm_wday].c_str());
  display->drawString(64 + x, 5 + y -3, String(buff));
  display->setFont(ArialMT_Plain_24);

  sprintf_P(buff, PSTR("%02d:%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
  display->drawString(64 + x, 22 + y -3, String(buff));
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {  //显示天气
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 38 + y -1, currentWeather.cond_txt + "    |   Wind: " + currentWeather.wind_sc + "  ");

  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  String temp = currentWeather.tmp + "°C" ;
  display->drawString(60 + x, 3 + y -1, temp);
  display->setFont(ArialMT_Plain_10);
  display->drawString(62 + x, 26 + y -1, currentWeather.fl + "°C | " + currentWeather.hum + "%");
  display->setFont(Meteocons_Plain_36);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(32 + x, 0 + y, currentWeather.iconMeteCon);
}

void drawForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {  //天气预报
  drawForecastDetails(display, x, y, 0);
  drawForecastDetails(display, x + 44, y, 1);
  drawForecastDetails(display, x + 88, y, 2);
}

void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex) {  //天气预报
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y, foreWeather[dayIndex].dateStr);
  display->setFont(Meteocons_Plain_21);
  display->drawString(x + 20, y + 12, foreWeather[dayIndex].iconMeteCon);

  String temp = foreWeather[dayIndex].tmp_min + " | " + foreWeather[dayIndex].tmp_max;
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y + 34, temp);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawCurrentWeather2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) { //显示天气城市2
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 38 + y -1, currentWeather2.cond_txt + "    |   Wind: " + currentWeather2.wind_sc + "  ");

  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  String temp = currentWeather2.tmp + "°C";
  display->drawString(60 + x, 3 + y -1, temp);
  display->setFont(ArialMT_Plain_10);
  display->drawString(62 + x, 26 + y -1, currentWeather2.fl + "°C | " + currentWeather2.hum + "%");
  display->setFont(Meteocons_Plain_36);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(32 + x, 0 + y, currentWeather2.iconMeteCon);
}

void drawForecast2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) { //预报天气城市2
  drawForecastDetails2(display, x, y, 0);
  drawForecastDetails2(display, x + 44, y, 1);
  drawForecastDetails2(display, x + 88, y, 2);
}

void drawForecastDetails2(OLEDDisplay *display, int x, int y, int dayIndex) {
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y, foreWeather2[dayIndex].dateStr);
  display->setFont(Meteocons_Plain_21);
  display->drawString(x + 20, y + 12, foreWeather2[dayIndex].iconMeteCon);

  String temp = foreWeather2[dayIndex].tmp_min + " | " + foreWeather2[dayIndex].tmp_max;
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y + 34, temp);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}



void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {   //绘图页眉覆盖
  now = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&now);
  char buff[14];
  sprintf_P(buff, PSTR("%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min);

  display->setColor(WHITE);
  display->setFont(ArialMT_Plain_10);//10
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(6, 50, String(buff));
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  String temp = fans;
  display->drawString(122, 50, temp);
  display->drawHorizontalLine(0, 48, 128);
}

void setReadyForWeatherUpdate() {  //为天气更新做好准备
  Serial.println("Setting readyForUpdate to true");
  readyForWeatherUpdate = true;
}
