/*
    MPU6050--NodeMCU 1.0

    VCC 3.3-5V（内部有稳压芯片）
    GND 地线
    SCL -GIO5(D1)   MPU6050作为从机时IIC时钟线
    SDA -GIO4(D2)   MPU6050作为从机时IIC数据线
    XCL             MPU6050作为主机时IIC时钟线
    XDA             MPU6050作为主机时IIC数据线
    AD0             地址管脚，该管脚决定了IIC地址的最低一位
    INT             中断引脚
*/

#include "I2Cdev.h"
#include "MPU6050.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 oled(128, 64, &Wire, -1);

/*舵机
#include <Servo.h>
Servo myservo;  //创建一个舵机控制对象
Servo myservo2; //创建一个舵机控制对象
// 使用Servo类最多可以控制8个舵机
*/

MPU6050 mpu; //实例化一个 MPU6050 对象，对象名称为 mpu
int16_t ax, ay, az, gx, gy, gz;
 
//********************angle data*********************//
float Gyro_y; //Y轴陀螺仪数据暂存
float Gyro_x;
float Gyro_z;
float AngleXY;
float AngleY;
float K1 = 0.05; // 对加速度计取值的权重
float AngleX; //一阶互补滤波计算出的X轴最终倾斜角度
float accelz = 0;
 
//********************angle data*********************//
 
//***************Kalman_Filter*********************//
float P[2][2] = {{ 1, 0 },
  { 0, 1 }
};
float Pdot[4] = { 0, 0, 0, 0};
float Q_angle = 0.001, Q_gyro = 0.005; //角度数据置信度,角速度数据置信度
float R_angle = 0.5 , C_0 = 1;
float q_bias, angle_err, PCt_0, PCt_1, E, K_0, K_1, t_0, t_1;
float timeChange = 5; //滤波法采样时间间隔毫秒
float dt = timeChange * 0.001; //注意：dt的取值为滤波器采样时间
//***************Kalman_Filter*********************//
 
void Angletest()
{
  //平衡参数
  AngleX = atan2(ay , az) * 180 / PI;           //角度计算公式
  Gyro_x = (gx - 128.1) / 131;             //角度转换
  Kalman_Filter(AngleX, Gyro_x);            //卡曼滤波
  //旋转角度Z轴参数
  if (gz > 32768) gz -= 65536;             //强制转换2g  1g
  Gyro_z = -gz / 131;                      //Z轴参数转换
  accelz = az / 16.4;
  //下面三行计算Y轴转动角度
  AngleY = atan2(ax, az) * 180 / PI; //计算与x轴夹角
  Gyro_y = (gy - 128.1) / 131;       //计算角速度
  Kalman_Filter(AngleY, Gyro_y);
  //一阶互补滤波
  //AngleXY = atan2(ax, az) * 180 / PI;  //计算与x轴夹角
  //Gyro_y = -gy / 131.00;              //计算角速度
  //AngleY = K1 * AngleXY + (1 - K1) * (AngleY + Gyro_y * dt);
}
 
//kalman
float angle, angle_dot;                                //平衡角度值
void Kalman_Filter(double angle_m, double gyro_m)
{
  angle += (gyro_m - q_bias) * dt;
  angle_err = angle_m - angle;
  Pdot[0] = Q_angle - P[0][1] - P[1][0];
  Pdot[1] = - P[1][1];
  Pdot[2] = - P[1][1];
  Pdot[3] = Q_gyro;
  P[0][0] += Pdot[0] * dt;
  P[0][1] += Pdot[1] * dt;
  P[1][0] += Pdot[2] * dt;
  P[1][1] += Pdot[3] * dt;
  PCt_0 = C_0 * P[0][0];
  PCt_1 = C_0 * P[1][0];
  E = R_angle + C_0 * PCt_0;
  K_0 = PCt_0 / E;
  K_1 = PCt_1 / E;
  t_0 = PCt_0;
  t_1 = C_0 * P[0][1];
  P[0][0] -= K_0 * t_0;
  P[0][1] -= K_0 * t_1;
  P[1][0] -= K_1 * t_0;
  P[1][1] -= K_1 * t_1;
  angle += K_0 * angle_err; //角度
  q_bias += K_1 * angle_err;
  angle_dot = gyro_m - q_bias; //角速度
}
 
void setup() {
  Wire.begin();                            //加入 I2C 总线序列
  Serial.begin(115200);                    //开启串口，设置波特率
  delay(1000);
  mpu.initialize();                        //初始化MPU6050

  //myservo.attach(9);  // 该舵机由arduino第九脚控制，通过PWM控制
  //myservo2.attach(8); // 该舵机由arduino第八脚控制，通过PWM控制

  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.setTextColor(WHITE); //开像素点发光
  oled.clearDisplay();      //清屏
  oled.print("MPU6050");
  oled.display(); // 开显示
}
 
void loop() {
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);     //IIC获取MPU6050六轴数据 ax ay az gx gy gz  
  Angletest();                                      //获取Angle角度和卡曼滤波
 
  Serial.print("X:");
  Serial.print(AngleX, 0); //X轴角度
  Serial.print(",");
  Serial.print("Y:");
  Serial.print(AngleY, 0); //Y轴角度
  Serial.print(",");
  Serial.print("Gyro_z:");
  Serial.println(Gyro_z, 0); //Z轴加速度
  delay(5);

  oled.clearDisplay();  //清屏
  oled.setTextSize(2);  //设置字体大小
  oled.setCursor(0, 5); //设置显示位置
  oled.print("X:");
  oled.setCursor(30, 5); //设置显示位置
  oled.println(AngleX, 0);
  oled.setCursor(0, 25); //设置显示位置
  oled.print("Y:");
  oled.setCursor(30, 25); //设置显示位置
  oled.println(AngleY, 0);
  oled.setCursor(0, 45); //设置显示位置
  oled.print("G_z:");
  oled.setCursor(50, 45); //设置显示位置
  oled.println(Gyro_z, 0);
  oled.setTextSize(1);  //设置字体大小
  oled.setCursor(80, 5); //设置显示位置
  oled.print("T7_OLED");
  oled.display(); // 开显示

  //myservo.write(agx);  // 指定舵机转向的角度
  //delay(15);           // 等待15ms让舵机到达指定位置
  //myservo2.write(agy); // 指定舵机转向的角度
  //delay(15);           // 等待15ms让舵机到达指定位置
}