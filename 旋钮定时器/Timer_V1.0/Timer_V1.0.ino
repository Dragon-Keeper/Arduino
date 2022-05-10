#include <Wire.h>
#include <LiquidCrystal_I2C.h> //Default Arduino LCD Librarey is included
//设置LCD1602设备地址，这里的地址是0x3F，一般是0x20，或者0x27，具体看模块手册
LiquidCrystal_I2C lcd(0x3F, 16, 2);

#define encoderOutA 9 // CLK
#define encoderOutB 8 // DT
#define encoderSw 10  // SW

int counter = 60;
int State;
int old_State;
int Led = 7; // 测试按键用：定义板载LED

void setup()
{
    pinMode(encoderOutA, INPUT);
    pinMode(encoderOutB, INPUT);
    pinMode(encoderSw, INPUT_PULLUP);
    lcd.init();      // 初始化LCD
    lcd.backlight(); // 设置LCD背景等亮
    lcd.print("Default:60Second");
    lcd.setCursor(0, 1);
    lcd.print("Push To Work."); //工作结束倒计时
    Serial.begin(9600);
    // Read First Position of Channel B
    old_State = digitalRead(encoderOutB);
    pinMode(Led, OUTPUT); // 测试按键用：实际就是控制继电器的工作状态来控制进水电磁阀通断
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
        lcd.clear();
        lcd.print(counter); //输出当前值
    }

    if (digitalRead(encoderSw) == LOW) //如果按压低电平则开始抽水
    {
        // delay(1000); //延时1秒，让Sw键重新高电平从而进入抽水状态
        for (int W_counter = counter; W_counter > 0; W_counter = W_counter - 1)
        {
            digitalWrite(Led, HIGH); //继电器控制开始抽水

            delay(500);
            if (digitalRead(encoderSw) == LOW)
            {
                digitalWrite(Led, LOW); //继电器控制停止抽水
                delay(1000);            //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
                goto Quit_The_Work;     //转跳到标记
            }
            //将延时分成两份，可以增强控制
            delay(500);
            if (digitalRead(encoderSw) == LOW)
            {
                digitalWrite(Led, LOW); //继电器控制停止抽水
                delay(1000);            //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
                goto Quit_The_Work;     //转跳到标记
            }
            lcd.clear();
            lcd.print(counter); //工作时长
            lcd.setCursor(0, 1);
            lcd.print(W_counter); //工作结束倒计时
        }
        digitalWrite(Led, LOW); //继电器控制停止抽水
        delay(10);              //消抖
        lcd.clear();
        lcd.print(counter); //输出当前值
        goto Quit_The_Work; //转跳到标记
    }

    if (digitalRead(encoderSw) == LOW && counter == 0) //如果按压低电平且计时器为0则不停抽水直到再次按压关停
    {
        digitalWrite(Led, HIGH); //继电器控制开始抽水
        delay(500);
        if (digitalRead(encoderSw) == LOW)
        {
            digitalWrite(Led, LOW); //继电器控制停止抽水
            delay(1000);            //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
            goto Quit_The_Work;     //转跳到标记
        }
        //将延时分成两份，可以增强控制
        delay(500);
        if (digitalRead(encoderSw) == LOW)
        {
            digitalWrite(Led, LOW); //继电器控制停止抽水
            delay(1000);            //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
            goto Quit_The_Work;     //转跳到标记
        }
    }

    old_State = State; // the first position was changed
}
