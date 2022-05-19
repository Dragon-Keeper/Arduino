/*  
    使用LM2596为Arduino Nano供电时，LM2596输出端需要调节到10-11V，才能使Nano的5V输出端电压达到5v，
    如果LM2596输出端需要调节到7-9V左右时，Nano的5V输出端电压达不到5v，外设工作异常。
*/

#include <Wire.h>
#include <TM1637Display.h>

// Module connection pins (Digital Pins)
#define CLK 8
#define DIO 9

// 定义显示刷新时间间隔
#define TEST_DELAY 50

TM1637Display display(CLK, DIO);

// 初始化旋钮编码器各引脚
#define EC11_DATA_key 4
#define EC11_DATA_A 2
#define EC11_DATA_B 3

int counter = 60;
int State;
int old_State;
int valve = 7; // 测试按键用：定义板载valve，0和1接线口之间是通的（常闭），0和2接线口之间是不通的（常开）

void setup()
{
    pinMode(EC11_DATA_key, INPUT_PULLUP); //输入上拉
    pinMode(EC11_DATA_A, INPUT_PULLUP);   //输入上拉
    pinMode(EC11_DATA_B, INPUT_PULLUP);   //输入上拉
    Serial.begin(9600);
    // Read First Position of Channel B
    old_State = digitalRead(EC11_DATA_B);
    pinMode(valve, OUTPUT);   // 测试按键用：实际就是控制继电器的工作状态来控制进水电磁阀通断
    display.setBrightness(3); //设置亮度用，从0-7级，运行默认为3级亮度
    display.clear();
    display.showNumberDec(counter, false);
}

void loop()
{
Quit_The_Work:
    State = digitalRead(EC11_DATA_B);
    if (State != old_State)
    {
        if (digitalRead(EC11_DATA_A) != State)
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
        // delay(10); //此处不能加延时，否则影响数据加减
    }

    if (digitalRead(EC11_DATA_key) == LOW) //如果按压低电平则开始抽水
    {
        delay(500); //延时0.5秒，让Sw键重新高电平从而进入抽水状态
        for (int W_counter = counter; W_counter > 0; W_counter = W_counter - 1)
        {
            digitalWrite(valve, HIGH); //继电器控制开始抽水
            delay(500);
            if (digitalRead(EC11_DATA_key) == LOW)
            {
                digitalWrite(valve, LOW); //继电器控制停止抽水
                delay(500);              //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
                display.showNumberDec(counter, false);
                goto Quit_The_Work; //转跳到标记
            }
            //将延时分成两份，可以增强控制
            delay(500);
            if (digitalRead(EC11_DATA_key) == LOW)
            {
                digitalWrite(valve, LOW); //继电器控制停止抽水
                delay(500);              //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
                display.showNumberDec(counter, false);
                goto Quit_The_Work; //转跳到标记
            }
            display.showNumberDec(W_counter, false);
            delay(TEST_DELAY);
        }
        digitalWrite(valve, LOW); //继电器控制停止抽水
        delay(10);                //消抖
        display.showNumberDec(counter, false);
        delay(TEST_DELAY);
        goto Quit_The_Work; //转跳到标记
    }

    if (digitalRead(EC11_DATA_key) == LOW && counter == 0) //如果按压低电平且计时器为0则不停抽水直到再次按压关停
    {
        delay(500);                //延时0.5秒，让Sw键重新高电平从而进入抽水状态
        digitalWrite(valve, HIGH); //继电器控制开始抽水
        if (digitalRead(EC11_DATA_key) == LOW)
        {
            digitalWrite(valve, LOW); //继电器控制停止抽水
            delay(500);              //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
            goto Quit_The_Work;       //转跳到标记
        }
        //将延时分成两份，可以增强控制
        delay(500);
        if (digitalRead(EC11_DATA_key) == LOW)
        {
            digitalWrite(valve, LOW); //继电器控制停止抽水
            delay(500);              //必须要有这个延时，才能让语句不那么快又执行到检测到按压低电平
            goto Quit_The_Work;       //转跳到标记
        }
    }

    old_State = State; // the first position was changed
}
