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
// #if (SSD1306_LCDHEIGHT != 64)
// #error("Height incorrect, please fix Adafruit_SSD1306.h!");
// #endif

// 控制屏幕方向-->0:正常显示；1：向右旋转90度；2：屏幕翻转180度；3：屏幕旋转270度。
// int Rotation = 0; //初始化屏幕方向
// 定义接受端口D8用于每摁一次屏幕旋转90度
// int Rotation_Screen = 15;
int TextSize = 3;
int DrawWight = 124;
float distan = 0;
float distan1 = 0;

/*卡尔曼参数*/
double R = 0.5;                                         // 系统测量噪声
double Q = 0.5;                                         // 系统过程噪声
double X_mid;                                           // 利用上一状态预测的结果
double X_last = 0;                                      // 上一刻的最优状态值 ，初始值为0
double X_now;                                           // 系统当前预测值和测量值估计的最优状态值
double Kg;                                              // 卡尔曼增益
double P_mid;                                           // 利用上一状态结果计算的协方差
double P_last = 1;                                      // 上一刻最优预测值的协方差
double P_now;                                           // 这一刻对应的最优协方差
double Z_mearure; // 超声波的测量值

// Uncomment this line to use long range mode. This
// increases the sensitivity of the sensor and extends its
// potential range, but increases the likelihood of getting
// an inaccurate reading because of reflections from objects
// other than the intended target. It works best in dark
// conditions.

// #define LONG_RANGE

// Uncomment ONE of these two lines to get
// - higher speed at the cost of lower accuracy OR
// - higher accuracy at the cost of lower speed

// #define HIGH_SPEED
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
        while (1)
        {
        }
    }

#if defined LONG_RANGE
    // lower the return signal rate limit (default is 0.25 MCPS)
    /*
    設定傳回訊號速率上限，單位是MCPS（每秒百萬次計數）。這是為了有效解讀目標物反射訊號所需的最低速率設定。
    較低的設定值可增加感測範圍，但也可能因增加讀取到目標以外的物體的反射訊號，而導致准確率下滑。
    訊號速率限制的初始預設值為0.25 MCPS。此函式將傳回一個bool值，代表設定值是否有效。
    */
    // sensor.setSignalRateLimit(0.1);
    sensor.setSignalRateLimit(0.1);
    // increase laser pulse periods (defaults are 14 and 10 PCLKs)
    /*
      設定成指定的週期時脈值。較長的週期時間可加長感測距離。有效值為（僅偶數）：
      Pre：12到18（預設為14）
      Final：8到14（預設為10）
      傳回值是bool類型，指出設定的週期時間是否有效。
    */
    sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
    sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);

#endif

#if defined HIGH_SPEED
    // reduce timing budget to 20 ms (default is about 33 ms)
    /*
    設定測量資料更新時間（單位是微秒）。這是一次測距所允許的時間；較長的資料更新時間可獲得更準確的測量。
    資料更新時間約為33000微秒，也就是33毫秒；最低為20毫秒。傳回值是bool類型，代表設定的資料更新時間是否有效。
    */
    sensor.setMeasurementTimingBudget(20000);

#elif defined HIGH_ACCURACY
    // increase timing budget to 200 ms
    sensor.setMeasurementTimingBudget(200000);
#endif
    // initialize with the I2C addr 0x3C (for the 128x64) ，0x3C为I2C协议通讯地址，需根据实际情况更改
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

    // pinMode(Rotation_Screen, INPUT_PULLUP); //设置管脚为旋转屏幕信号输入上拉，令到它值为HIGH
}

void loop()
{
    // Serial.println(sensor.readRangeSingleMillimeters()); //单次测量并传回单位（mm）的测量值
    if (sensor.timeoutOccurred())
    {
        Serial.println(F(" TIMEOUT"));
    }

    /*
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
   */

    /*
    //只卡尔曼滤波
    Z_mearure = sensor.readRangeSingleMillimeters(); // 超声波的测量值
    distan1 = Kalman_Filter(Z_mearure) / 10 - 4 ;
    */
   
    // 先算术平均滤波再卡尔曼滤波
    distan = Filter();
    distan1 = Kalman_Filter(distan) / 10 - 4 ;
    /*------------------------显示距离---------------------*/
    display.clearDisplay(); // 显示之前清屏
    // display.setRotation(Rotation);  //旋转屏幕
    display.setTextSize(TextSize); // 选择字号
    display.setTextColor(WHITE);   // 字体颜色
    display.setCursor(0, 10);      // 字体位置
    display.print(distan1, 1);     // 显示一位小数
    display.setTextSize(1);
    display.println(F("cm"));
    display.drawRect(0, 38, DrawWight, 12, WHITE);
    display.fillRect(2, 40, map(distan1, 2, 200, 2, DrawWight), 8, WHITE); // map(要映射的值，旧区间初值，旧区间终值，新区间初值，新区间终值)
    // 进度条采用静态空心矩形套一个动态实心矩形实现//
    display.display(); // 显示图形
}

// 算术平均滤波法
#define FILTER_N 4
float Filter()
{
    int i;
    float filter_sum = 0;
    for (i = 0; i < FILTER_N; i++)
    {
        filter_sum += sensor.readRangeSingleMillimeters();
    }
    return (float)(filter_sum / FILTER_N);
}

// 卡尔曼滤波

/*卡尔曼滤波，mearure为超声波测量值入口参数*/
double Kalman_Filter(double mearure)
{
    Z_mearure = mearure;

    X_mid = X_last;     // 1.预测先验估计值
    P_mid = P_last + Q; // 2.预测当前先验估计值所对应的协方差

    Kg = P_mid / (P_mid + R);                 // 3.更新卡尔曼增益
    X_now = X_mid + Kg * (Z_mearure - X_mid); // 修正先验估计值的最优值
    P_now = (1 - Kg) * P_mid;                 // 更新当前状态所对应的协方差

    // 更新下一次的协方差和预测值
    X_last = X_now; //
    P_last = P_now; //

    // 返回最优解
    return X_now;
}