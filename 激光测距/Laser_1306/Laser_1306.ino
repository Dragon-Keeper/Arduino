/*!
   @file DFRobot_VL53L0X.ino
   @brief DFRobot's Laser rangefinder library
   @n The example shows the usage of VL53L0X in a simple way.

   @copyright  [DFRobot](http://www.dfrobot.com), 2016
   @copyright GNU Lesser General Public License

   @author [LiXin]
   @version  V1.0
   @date  2017-8-21
   @https://github.com/DFRobot/DFRobot_VL53L0X
*/

/* #include DFRobotVL53L0X sensor  //创建一个VL53L0X对象

  函数功能：设置测距模式。
  参数1 mode: 测距模式。
  Single: 单次测距        Continuous: 连续测距
  参数2 precision: 测量精度
  High: 高精度(0.25mm)    Low: 标准精度(1mm)
  void setMode(uint8_t mode, uint8_t precision);

  函数功能：开始测量距离。
  void start();

  函数功能：停止测量。
  void stop();

  函数功能：获取距离。
  uint16_t getDistance();

  函数功能：获取环境量。
  uint16_t getAmbientCount();

  函数功能：获取信号数。
  uint16_t getSignalCount();
*/
/* Arduino-Nano

   充电宝5V口输入供电，Nano的3.3V输出口并联一个100Ω电阻接地，起到
   增大充电宝输出电流作用，充电宝就不会10s后关闭。

   所有元件共地！

   OLED 1306
   GND-GND    VDD-3.3V  SCK-A5(19)    SDA-A4(18)

   TOF200C：GY-VL5310X
   GND-GND    VDD-3.3V  SCK-A5(19)    SDA-A4(18)

   屏幕旋转按钮
   Button-A1(15)        Button-GND
*/
#include "Wire.h"
#include "DFRobot_VL53L0X.h"
DFRobotVL53L0X sensor;

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

//控制屏幕方向-->0:正常显示；1：向右旋转90度；2：屏幕翻转180度；3：屏幕旋转270度。
int Rotation = 0; //初始化屏幕方向
//定义接受端口D8用于每摁一次屏幕旋转90度
int Rotation_Screen = 15;
int TextSize = 3;

void setup() {
  //initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
  //join i2c bus (address optional for master)
  Wire.begin();
  //Set I2C sub-device address
  sensor.begin(0x50);
  //Set to Back-to-back mode and high precision mode
  sensor.setMode(Continuous, High);
  //Laser rangefinder begins to work
  sensor.start();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64) ，0x3C为I2C协议通讯地址，需根据实际情况更改

  pinMode(Rotation_Screen, INPUT_PULLUP); //设置管脚为旋转屏幕信号输入上拉，令到它值为HIGH
}

void loop()
{
  //Get the distance
  Serial.print(F("Distance: "));
  Serial.println(sensor.getDistance());
  //The delay is added to demonstrate the effect, and if you do not add the delay,
  //it will not affect the measurement accuracy
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
    TextSize = 3;  //横向时字体大小为2
  }
  else
  {
    TextSize = 2;  //正反向时字体大小为3
  }
  
  //算术平均滤波法
  float distan;
  distan = Filter();
  distan = distan / 10 - 5; //减5cm的偏移量得准确值
  /*------------------------显示距离---------------------*/
  display.clearDisplay();   //显示之前清屏
  display.setRotation(Rotation);  //旋转屏幕
  display.setTextSize(TextSize);  //选择字号
  display.setTextColor(WHITE);  //字体颜色
  display.setCursor(0, 20);  //字体位置
  display.print(distan, 1); //显示一位小数
  display.setTextSize(1);
  display.println(F("cm"));
  //display.drawRect(0, 38, 128, 12, WHITE);
  //display.fillRect(2, 40, map(distan, 2, 200, 2, 124), 8, WHITE);  //map(要映射的值，旧区间初值，旧区间终值，新区间初值，新区间终值)
  //进度条采用静态空心矩形套一个动态实心矩形实现//
  display.display();  //显示图形
  delay(100);
}

// 算术平均滤波法
#define FILTER_N 39
float Filter() {
  int i;
  float filter_sum = 0;
  for (i = 0; i < FILTER_N; i++) {
    filter_sum += sensor.getDistance();
    delay(25);
  }
  return (float)(filter_sum / FILTER_N);
}
