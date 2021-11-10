/*
  【Arduino】168种传感器模块系列实验（资料 +代码 +图形 +仿真）
  实验二十七：GY-BMP280-3.3 高精度大气压强传感器模块（高度与温度计）
  程序之二：IIC通信 Forced模式读取BMP280
  实验接线
  Arduino  BMP280
  3.3V --- VCC
  GND ---  GND
  A5  ---  SCL
  A4  ---  SDA
*/

#include <BMP280_DEV.h>

float temperature, pressure, altitude;//创建温度、压力和高度变量
BMP280_DEV bmp280;  //实例化（创建）BMP280_DEV对象并设置I2C操作

void setup()
{
  Serial.begin(115200);// 初始化串行端口
  bmp280.begin(BMP280_I2C_ALT_ADDR);  // 使用可选I2C地址（0x76）进行默认初始化，将BMP280置于休眠模式
  bmp280.setPresOversampling(OVERSAMPLING_X16);    // 将压力过采样设置为X4
  bmp280.setTempOversampling(OVERSAMPLING_X16);    // 将温度过采样设置为X1
  bmp280.setIIRFilter(IIR_FILTER_16);             // 将IIR滤波器设置为设置4
  bmp280.setTimeStandby(TIME_STANDBY_2000MS);    // 将待机时间设置为1秒
  bmp280.startNormalConversion();                // 在正常模式下启动BMP280连续转换
}

void loop()
{
  //唤醒BMP280，提出测量要求
  bmp280.startForcedConversion();
  //获取结果并输出
  if (bmp280.getMeasurements(temperature, pressure, altitude))
  {
    Serial.print(temperature);  
    Serial.print(F("*C   "));
    Serial.print(pressure);
    Serial.print(F("hPa   "));
    Serial.print(altitude);
    Serial.println(F("m"));
    delay(2000);
  }
}
