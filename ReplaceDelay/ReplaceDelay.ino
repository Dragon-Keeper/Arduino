/* 最好用的替代Delay延时方法 */

/* 完全正常使用，延时为2秒，只是不能动态更改延时，每次通过time1.Update();来使用2秒的延时
class DaleyTimes // Class Member Variables
{
    // These are initialized at startup
    long WaitTime;                // milliseconds of Wait-time
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
            Serial.println(F("Waiting......"));
        }
    }
};

DaleyTimes time1(2000);
void setup()
{
    Serial.begin(115200);
}
void loop()
{
    time1.Update();
}
*/
class DaleyTimes // Class Member Variables
{
    // These are initialized at startup
    long WaitTime;                // milliseconds of Wait-time
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
            Serial.print(F("Wait time is:"));
            Serial.println(WaitTime);
        }
    }
};

DaleyTimes time05(500);
DaleyTimes time1(1000);
DaleyTimes time2(2000);
void setup()
{
    Serial.begin(115200);
}
void loop()
{
    //各计时器间互不影响
    time05.Update();
    time1.Update();
    time2.Update();
}