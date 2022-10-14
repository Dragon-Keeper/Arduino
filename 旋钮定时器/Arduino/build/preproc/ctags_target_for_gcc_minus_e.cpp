# 1 "c:\\Users\\25\\Documents\\GitHub\\Arduino\\旋钮定时器\\Timer_Sevseg_Mos_V2.0\\Timer_Sevseg_Mos_V2.0.ino"
/*  

    EC11外壳固定引脚接地防静电感应触发按压功能，应该接到电源的地线，不要接到arduino的地线，否则会有干扰了；

    使用LM2596为Arduino Nano供电时，LM2596输出端需要调节到10-11V，才能使Nano的5V输出端电压达到5v，

    如果LM2596输出端需要调节到7-9V左右时，Nano的5V输出端电压达不到5v，外设工作异常。



    这个是用数码管显示并用MOS管控制电磁阀版本

*/
# 9 "c:\\Users\\25\\Documents\\GitHub\\Arduino\\旋钮定时器\\Timer_Sevseg_Mos_V2.0\\Timer_Sevseg_Mos_V2.0.ino"
# 10 "c:\\Users\\25\\Documents\\GitHub\\Arduino\\旋钮定时器\\Timer_Sevseg_Mos_V2.0\\Timer_Sevseg_Mos_V2.0.ino" 2
# 11 "c:\\Users\\25\\Documents\\GitHub\\Arduino\\旋钮定时器\\Timer_Sevseg_Mos_V2.0\\Timer_Sevseg_Mos_V2.0.ino" 2

// Module connection pins (Digital Pins)



// 定义显示刷新时间间隔


TM1637Display display(A4, A1);

// 初始化旋钮编码器各引脚




int counter = 60;
int State;
int old_State;
int valve = 12; // 接MosFET隔离模块，高电平时接通

class DaleyTimes // Class Member Variables
{
    // These are initialized at startup
    long WaitTime; // milliseconds of Wait-time
    unsigned long previousMillis; // will store last time things was updated
public:
    DaleyTimes(long on)
    {
        WaitTime = on;
        previousMillis = 0;
    }
    void Update()
    {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= WaitTime)
        {
            previousMillis = currentMillis; // Remember the time
            Serial.print((reinterpret_cast<const __FlashStringHelper *>(
# 48 "c:\\Users\\25\\Documents\\GitHub\\Arduino\\旋钮定时器\\Timer_Sevseg_Mos_V2.0\\Timer_Sevseg_Mos_V2.0.ino" 3
                        (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 48 "c:\\Users\\25\\Documents\\GitHub\\Arduino\\旋钮定时器\\Timer_Sevseg_Mos_V2.0\\Timer_Sevseg_Mos_V2.0.ino"
                        "Wait time is:"
# 48 "c:\\Users\\25\\Documents\\GitHub\\Arduino\\旋钮定时器\\Timer_Sevseg_Mos_V2.0\\Timer_Sevseg_Mos_V2.0.ino" 3
                        ); &__c[0];}))
# 48 "c:\\Users\\25\\Documents\\GitHub\\Arduino\\旋钮定时器\\Timer_Sevseg_Mos_V2.0\\Timer_Sevseg_Mos_V2.0.ino"
                        )));
            Serial.println(WaitTime);
        }
    }
};

DaleyTimes time05(500);
DaleyTimes time001(10);
DaleyTimes time005(50);

/*

//在loop中调用以下语句来达到延时目的，各计时器间互不影响

    time05.Update();

    time001.Update();

    time005.Update();

*/
# 65 "c:\\Users\\25\\Documents\\GitHub\\Arduino\\旋钮定时器\\Timer_Sevseg_Mos_V2.0\\Timer_Sevseg_Mos_V2.0.ino"
void setup()
{
    pinMode(9, 0x2); //输入上拉
    pinMode(6, 0x2); //输入上拉
    pinMode(3, 0x2); //输入上拉
    Serial.begin(9600);
    // Read First Position of Channel B
    old_State = digitalRead(3);
    pinMode(valve, 0x1); // 测试按键用：实际就是控制继电器的工作状态来控制进水电磁阀通断
    display.setBrightness(3); //设置亮度用，从0-7级，运行默认为3级亮度
    display.clear();
    display.showNumberDec(counter, false);
}

void loop()
{
Quit_The_Work:
    State = digitalRead(3);
    if (State != old_State)
    {
        if (digitalRead(6) != State)
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
        display.showNumberDec(counter, false);
        // time1.Update(); //此处不能加延时，否则影响数据加减
    }

    if (digitalRead(9) == 0x0) //如果按压低电平则开始抽水
    {
        time05.Update(); //延时0.5秒，让Sw键重新高电平从而进入抽水状态
        for (int W_counter = counter; W_counter > 0; W_counter = W_counter - 1)
        {
            digitalWrite(valve, 0x1); //继电器控制开始抽水
            time05.Update();
            if (digitalRead(9) == 0x0)
            {
                digitalWrite(valve, 0x0); //继电器控制停止抽水
                time05.Update(); //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
                display.showNumberDec(counter, false);
                goto Quit_The_Work; //转跳到标记
            }
            //将延时分成两份，可以增强控制
            time05.Update();
            if (digitalRead(9) == 0x0)
            {
                digitalWrite(valve, 0x0); //继电器控制停止抽水
                time05.Update(); //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
                display.showNumberDec(counter, false);
                goto Quit_The_Work; //转跳到标记
            }
            display.showNumberDec(W_counter, false);
            time005.Update(); //delay(TEST_DELAY); 显示刷新时间间隔
        }
        digitalWrite(valve, 0x0); //继电器控制停止抽水
        time001.Update(); //消抖
        display.showNumberDec(counter, false);
        time005.Update(); //delay(TEST_DELAY); 显示刷新时间间隔
        goto Quit_The_Work; //转跳到标记
    }

    if (digitalRead(9) == 0x0 && counter == 0) //如果按压低电平且计时器为0则不停抽水直到再次按压关停
    {
        time05.Update(); //延时0.5秒，让Sw键重新高电平从而进入抽水状态
        digitalWrite(valve, 0x1); //继电器控制开始抽水
        if (digitalRead(9) == 0x0)
        {
            digitalWrite(valve, 0x0); //继电器控制停止抽水
            time05.Update(); //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
            goto Quit_The_Work; //转跳到标记
        }
        //将延时分成两份，可以增强控制
        time05.Update();
        if (digitalRead(9) == 0x0)
        {
            digitalWrite(valve, 0x0); //继电器控制停止抽水
            time05.Update(); //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
            goto Quit_The_Work; //转跳到标记
        }
    }

    old_State = State; // the first position was changed
}
