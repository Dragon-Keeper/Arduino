/* This example shows how to get single-shot range
  measurements from the VL53L0X. The sensor can optionally be
  configured with different ranging profiles, as described in
  the VL53L0X API user manual, to get better performance for
  a certain application. This code is based on the four
  "SingleRanging" examples in the VL53L0X API.

  The range readings are in units of mm. */
/* Arduino-Nano

  充电宝5V口输入供电，Nano的3.3V输出口并联一个100Ω电阻接地，起到
  增大充电宝输出电流作用，充电宝就不会10s后关闭。

  手机Type-C转USB连接Nano的mini-B接口可持续供电。

  所有元件共地！

  OLED 1306
  GND-GND    VDD-3.3V  SCK-A5(19)    SDA-A4(18)

  TOF200C：GY-VL5310X
  GND-GND    VDD-3.3V  SCK-A5(19)    SDA-A4(18)

  屏幕旋转按钮
  Button-A1(15)        Button-GND
*/
#include <Wire.h>
#include <VL53L0X.h>
VL53L0X sensor;

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
//#if (SSD1306_LCDHEIGHT != 64)
//#error("Height incorrect, please fix Adafruit_SSD1306.h!");
//#endif

//控制屏幕方向-->0:正常显示；1：向右旋转90度；2：屏幕翻转180度；3：屏幕旋转270度。
int Rotation = 0; //初始化屏幕方向
//定义接受端口D8用于每摁一次屏幕旋转90度
int Rotation_Screen = 15;
int TextSize = 3;
int DrawWight = 124;
float distan;

// Uncomment this line to use long range mode. This
// increases the sensitivity of the sensor and extends its
// potential range, but increases the likelihood of getting
// an inaccurate reading because of reflections from objects
// other than the intended target. It works best in dark
// conditions.

//#define LONG_RANGE

// Uncomment ONE of these two lines to get
// - higher speed at the cost of lower accuracy OR
// - higher accuracy at the cost of lower speed

//#define HIGH_SPEED
#define HIGH_ACCURACY

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  sensor.setAddress(0x50);
  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println(F("Failed to detect and initialize sensor!"));
    while (1) {}
  }

#if defined LONG_RANGE
  // lower the return signal rate limit (default is 0.25 MCPS)
  sensor.setSignalRateLimit(0.1);
  // increase laser pulse periods (defaults are 14 and 10 PCLKs)
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
#endif

#if defined HIGH_SPEED
  // reduce timing budget to 20 ms (default is about 33 ms)
  sensor.setMeasurementTimingBudget(20000);

#elif defined HIGH_ACCURACY
  // increase timing budget to 200 ms
  sensor.setMeasurementTimingBudget(200000);
#endif

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64) ，0x3C为I2C协议通讯地址，需根据实际情况更改

  pinMode(Rotation_Screen, INPUT_PULLUP); //设置管脚为旋转屏幕信号输入上拉，令到它值为HIGH
}

void loop()
{
  Serial.println(sensor.readRangeSingleMillimeters()); //单次测量
  if (sensor.timeoutOccurred())
  {
    Serial.println(F(" TIMEOUT"));
  }

  //控制屏幕方向
  if (digitalRead(Rotation_Screen) < 1)
  {
    delay(1);
    Rotation = Rotation + 1;
    if (Rotation > 3)
    {
      Rotation = 0;
    }
  }
  //通过取余判断当前页面方向来改变字体大小
  if ((Rotation == 0) || (Rotation == 2))
  {
    TextSize = 3;  //横向时字体大小为3
    DrawWight = 124;  //横向时动态实心矩形宽124
  }
  else
  {
    TextSize = 2;  //正反向时字体大小为2
    DrawWight = 60;  //正反向时动态实心矩形宽60
  }

  //算术平均滤波法
  distan = Filter() - 4.3;  //结果减去4.3cm的偏移量得正确测量值
  /*------------------------显示距离---------------------*/
  display.clearDisplay();   //显示之前清屏
  display.setRotation(Rotation);  //旋转屏幕
  display.setTextSize(TextSize);  //选择字号
  display.setTextColor(WHITE);  //字体颜色
  display.setCursor(0, 10);  //字体位置
  display.print(distan, 1); //显示一位小数
  display.setTextSize(1);
  display.println(F("cm"));
  display.drawRect(0, 38, DrawWight, 12, WHITE);
  display.fillRect(2, 40, map(distan, 2, 200, 2, DrawWight), 8, WHITE);  //map(要映射的值，旧区间初值，旧区间终值，新区间初值，新区间终值)
  //进度条采用静态空心矩形套一个动态实心矩形实现//
  display.display();  //显示图形
  delay(510);
}

// 算术平均滤波法
#define FILTER_N 10
float Filter() {
  int i;
  float filter_sum = 0;
  for (i = 0; i < FILTER_N; i++) {
    filter_sum += sensor.readRangeSingleMillimeters();
    //delay(210);
  }
  return (float)(filter_sum / FILTER_N / 10);
}
