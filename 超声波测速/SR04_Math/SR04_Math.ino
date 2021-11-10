/* Arduino-Nano

   充电宝5V口输入供电，Nano的3.3V输出口并联一个100Ω电阻接地，起到
   增大充电宝输出电流作用，充电宝就不会10s后关闭。

   所有元件共地！

   OLED 1306
   GND-GND    VDD-3.3V      SCK-A5(19)    SDA-A4(18)

   HC-SR04  CS100A
   VCC-3.3V   Trig-A2(16)   Echo-A3(17)   GND-GND

   DHT11
   S-D8(8)    VCC-3.3V      GND-GND
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET -1
#define TrigPin 16
#define EchoPin 17

Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);//声明OLED

#include <DHT.h>
#define DHTTYPE DHT11
DHT dht(8, DHTTYPE);

float dis;

void setup() {
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(WHITE);//开像素点发光
  display.clearDisplay();//清屏

  display.setTextSize(2); //设置字体大小
  pinMode(TrigPin, OUTPUT);
  pinMode(EchoPin, INPUT);

  dht.begin();
}

void loop() {

  //----------------------DHT11----------------//
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  //----------------------声速修正公式----------------//
  float sound_spd = 331.4 + (0.606 * t) + (0.0124 * h);
  //Serial.println(sound_spd);

  //----------------------测距----------------//
  // Write a pulse to the HC-SR04 Trigger Pin//做一个10uS的TTL，激发测距模块。
  digitalWrite(TrigPin, LOW);  delayMicroseconds(2);
  digitalWrite(TrigPin, HIGH); delayMicroseconds(10);
  digitalWrite(TrigPin, LOW);
  dis = (pulseIn(EchoPin, HIGH) / 2) * sound_spd / 10000; //测距公式
  //Serial.print(F("Distance:"));
  //Serial.println(dis);
  
  //算术平均滤波法
  float distan;
  distan = Filter();

  display.clearDisplay();//清屏
  display.setCursor(20, 5);
  display.print(distan, 2); //显示一位小数
  display.drawRect(0, 28, 128, 12, WHITE);
  display.fillRect(2, 30, map(distan, 2, 200, 2, 124), 8, WHITE);//map(要映射的值，旧区间初值，旧区间终值，新区间初值，新区间终值)
  /*
    进度条采用静态空心矩形套一个动态实心矩形实现
  */
  display.setCursor(0, 50);
  display.print(F("T"));
  display.print(int(t));
  display.setCursor(40, 50);
  display.print(F("H"));
  display.print(int(h));
  display.setCursor(80, 50);
  display.print(F("F"));
  display.print(int(hic));
  display.display();//开显示
  //delay(500);
}

// 算术平均滤波法
#define FILTER_N 27 //最坏的情况下距离测量需要66ms，取整按70ms算，14次刚好980ms
float Filter() {
  int i;
  float filter_sum = 0;
  for (i = 0; i < FILTER_N; i++) {
    filter_sum += dis;
    delay(70);
  }
  return (float)(filter_sum / FILTER_N);
}
