# 1 "c:\\Users\\25\\Desktop\\旋钮定时器\\Timer_V1.0\\Timer_V1.0.ino"
# 2 "c:\\Users\\25\\Desktop\\旋钮定时器\\Timer_V1.0\\Timer_V1.0.ino" 2
# 3 "c:\\Users\\25\\Desktop\\旋钮定时器\\Timer_V1.0\\Timer_V1.0.ino" 2
//设置LCD1602设备地址，这里的地址是0x3F，一般是0x20，或者0x27，具体看模块手册
LiquidCrystal_I2C lcd(0x3F, 16, 2);





int counter = 60;
int State;
int old_State;
int Led = 7; // 测试按键用：定义板载LED

void setup()
{
    pinMode(9 /* CLK*/, 0x0);
    pinMode(8 /* DT*/, 0x0);
    pinMode(10 /* SW*/, 0x2);
    lcd.init(); // 初始化LCD
    lcd.backlight(); // 设置LCD背景等亮
    lcd.print("Default:60Second");
    lcd.setCursor(0, 1);
    lcd.print("Push To Work."); //工作结束倒计时
    Serial.begin(9600);
    // Read First Position of Channel B
    old_State = digitalRead(8 /* DT*/);
    pinMode(Led, 0x1); // 测试按键用：实际就是控制继电器的工作状态来控制进水电磁阀通断
}

void loop()
{
Quit_The_Work:
    State = digitalRead(8 /* DT*/);
    if (State != old_State)
    {
        if (digitalRead(9 /* CLK*/) != State)
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

    if (digitalRead(10 /* SW*/) == 0x0) //如果按压低电平则开始抽水
    {
        // delay(1000); //延时1秒，让Sw键重新高电平从而进入抽水状态
        for (int W_counter = counter; W_counter > 0; W_counter = W_counter - 1)
        {
            digitalWrite(Led, 0x1); //继电器控制开始抽水

            delay(500);
            if (digitalRead(10 /* SW*/) == 0x0)
            {
                digitalWrite(Led, 0x0); //继电器控制停止抽水
                delay(1000); //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
                goto Quit_The_Work; //转跳到标记
            }
            //将延时分成两份，可以增强控制
            delay(500);
            if (digitalRead(10 /* SW*/) == 0x0)
            {
                digitalWrite(Led, 0x0); //继电器控制停止抽水
                delay(1000); //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
                goto Quit_The_Work; //转跳到标记
            }
            lcd.clear();
            lcd.print(counter); //工作时长
            lcd.setCursor(0, 1);
            lcd.print(W_counter); //工作结束倒计时
        }
        digitalWrite(Led, 0x0); //继电器控制停止抽水
        delay(10); //消抖
        lcd.clear();
        lcd.print(counter); //输出当前值
        goto Quit_The_Work; //转跳到标记
    }

    if (digitalRead(10 /* SW*/) == 0x0 && counter == 0) //如果按压低电平且计时器为0则不停抽水直到再次按压关停
    {
        digitalWrite(Led, 0x1); //继电器控制开始抽水
        delay(500);
        if (digitalRead(10 /* SW*/) == 0x0)
        {
            digitalWrite(Led, 0x0); //继电器控制停止抽水
            delay(1000); //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
            goto Quit_The_Work; //转跳到标记
        }
        //将延时分成两份，可以增强控制
        delay(500);
        if (digitalRead(10 /* SW*/) == 0x0)
        {
            digitalWrite(Led, 0x0); //继电器控制停止抽水
            delay(1000); //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
            goto Quit_The_Work; //转跳到标记
        }
    }

    old_State = State; // the first position was changed
}
