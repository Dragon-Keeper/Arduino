#include "SevSeg.h"

SevSeg sevseg;

byte numDigits = 4;                                  // 数码管位数
byte digitPins[] = {2, 3, 4, 5};                     //数码管公共极连接Arduino的引脚
byte segmentPins[] = {6, 7, A4, A5, A3, 11, 12, 13}; //数码管a, b, c, d, e, f, g, dp对应引脚
bool resistorsOnSegments = false;                    // 'false' 表示电阻器在数字引脚上
byte hardwareConfig = COMMON_CATHODE;                // 使用的是共阴极数码管
bool updateWithDelays = false;                       // Default 'false' is Recommended
bool leadingZeros = false;                           // 如果你想保持前导零，就用true
bool disableDecPoint = false;                        //如果你的小数点不存在或没有关联，请使用true

#define encoderOutA 9 // CLK 9
#define encoderOutB 8 // DT 8
#define encoderSw 10  // SW 10

int counter = 60;
int State;
int old_State;
int valve = A2; // 测试按键用：定义板载valve，0和1接线口之间是通的（常闭），0和2接线口之间是不通的（常开）

// 声明并定义可变的变量 :
int ledState = LOW; // LED的状态值
// 一般来说，用 "unsigned long"类型的变量来存储时间值比较好。因为如果用int类型“装不下”这么大的数字。
unsigned long previousMillis = 0; // 存储上次LED状态被改变的时间
unsigned long currentMillis;

void setup()
{
    Serial.begin(9600);
    //初始化数码管
    sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
                 updateWithDelays, leadingZeros, disableDecPoint); //添加参数
    sevseg.setBrightness(6);

    pinMode(encoderOutA, INPUT);
    pinMode(encoderOutB, INPUT);
    pinMode(encoderSw, INPUT_PULLUP);
    old_State = digitalRead(encoderOutB);
    pinMode(valve, OUTPUT); // 测试按键用：实际就是控制继电器的工作状态来控制进水电磁阀通断
}

void loop()
{
Quit_The_Work:
    State = digitalRead(encoderOutB);
    if (State != old_State)
    {
        if (digitalRead(encoderOutA) != State)
        {
            counter++;
            counter = counter + 2; //步长为6
        }
        else
        {
            counter--;
            counter = counter - 2;
            if (counter == 0 || counter < 0) //当小于或等于0时，计时器止步于0
            {
                counter = 0;
            }
        }

        currentMillis = millis(); // millis()的单位是毫秒
        if (currentMillis - previousMillis >= 100)
        {
            // 更新时间标记
            previousMillis = currentMillis;
        }
        
        Serial.println(counter);
        sevseg.setNumber(counter); // 设置要显示的数据
        for (int j = 0; j < 2000; j++)
        {
            sevseg.refreshDisplay(); // 必须重新运行刷新数码管显示
        }
    }

    if (digitalRead(encoderSw) == LOW) //如果按压低电平则开始抽水
    {
        for (int W_counter = counter; W_counter > 0; W_counter = W_counter - 1)
        {
            digitalWrite(valve, HIGH); //继电器控制开始抽水

            sevseg.setNumber(W_counter); //工作结束倒计时
            for (int j = 0; j < 32000; j++)
            {
                sevseg.refreshDisplay(); // 必须重新运行刷新数码管显示
            }

            // 检查看看LED是否到了应该打开或关闭的时间; 就是说，检查下现在时间离开记录的时间是否超过了要求LED状态改变的间隔时间。
            currentMillis = millis(); // millis()的单位是毫秒

            if (currentMillis - previousMillis >= counter * 1000)
            {
                // 更新时间标记
                previousMillis = currentMillis;

                digitalWrite(valve, LOW);
            }

            currentMillis = millis(); // millis()的单位是毫秒
            if (currentMillis - previousMillis >= 500)
            {
                // 更新时间标记
                previousMillis = currentMillis;
            }
            if (digitalRead(encoderSw) == LOW)
            {
                digitalWrite(valve, LOW); //继电器控制停止抽水
                currentMillis = millis(); // millis()的单位是毫秒
                //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
                if (currentMillis - previousMillis >= 1000)
                {
                    // 更新时间标记
                    previousMillis = currentMillis;
                }
                goto Quit_The_Work; //转跳到标记
            }

            //将延时分成两份，可以增强控制
            currentMillis = millis(); // millis()的单位是毫秒
            if (currentMillis - previousMillis >= 500)
            {
                // 更新时间标记
                previousMillis = currentMillis;
            }
            if (digitalRead(encoderSw) == LOW)
            {
                digitalWrite(valve, LOW); //继电器控制停止抽水
                currentMillis = millis(); // millis()的单位是毫秒
                //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
                if (currentMillis - previousMillis >= 1000)
                {
                    // 更新时间标记
                    previousMillis = currentMillis;
                }
                goto Quit_The_Work; //转跳到标记
            }
            sevseg.setNumber(W_counter); //工作结束倒计时
            for (int j = 0; j < 32000; j++)
            {
                sevseg.refreshDisplay(); // 必须重新运行刷新数码管显示
            }
        }
        digitalWrite(valve, LOW); //继电器控制停止抽水

        //消抖
        currentMillis = millis(); // millis()的单位是毫秒
        //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
        if (currentMillis - previousMillis >= 10)
        {
            // 更新时间标记
            previousMillis = currentMillis;
        }

        sevseg.setNumber(counter); //工作结束倒计时
        for (int j = 0; j < 32000; j++)
        {
            sevseg.refreshDisplay(); // 必须重新运行刷新数码管显示
        }
        goto Quit_The_Work; //转跳到标记
    }

    /*
    if (digitalRead(encoderSw) == LOW && counter == 0) //如果按压低电平且计时器为0则不停抽水直到再次按压关停
    {
        digitalWrite(valve, HIGH); //继电器控制开始抽水
        delay(500);
        if (digitalRead(encoderSw) == LOW)
        {
            digitalWrite(valve, LOW); //继电器控制停止抽水
            delay(1000);              //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
            goto Quit_The_Work;       //转跳到标记
        }
        //将延时分成两份，可以增强控制
        delay(500);
        if (digitalRead(encoderSw) == LOW)
        {
            digitalWrite(valve, LOW); //继电器控制停止抽水
            delay(1000);              //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
            goto Quit_The_Work;       //转跳到标记
        }
    }
    */

    old_State = State; // the first position was changed
}
