unsigned char LED_0F[] =
    {
        // 共阳数码管编码
        // 0 1 2 3 4 5 6 7 8 9 a b c d e f
        0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0x88, 0x83, 0xc6, 0xa1, 0x86, 0x8e};
unsigned char LED[4]; //用于 LED 的 4 位显示缓存
int SCLK = 4;
int RCLK = 3;
int DIO = 8;

void LED4_Display();

void setup()
{
    pinMode(SCLK, OUTPUT);
    pinMode(RCLK, OUTPUT);
    pinMode(DIO, OUTPUT);
    Serial.begin(9600);
}
void loop()
{

    LED[0] = 0;
    LED[1] = 0;
    LED[2] = g;
    LED[3] = k;

    dely();
}
void dely()
{
    int z = 1000;
    while (z > 1)
    {
        z--;
        LED4_Display();
    }
}
void LED4_Display(void)
{
    unsigned char *led_table; // 查表指针
    unsigned char i;
    //显示第 1 位
    led_table = LED_0F + LED[0];
    i = *led_table;
    LED_OUT(i);
    LED_OUT(0x01);
    digitalWrite(RCLK, LOW);
    digitalWrite(RCLK, HIGH);
    //显示第 2 位
    led_table = LED_0F + LED[1];
    i = *led_table;
    LED_OUT(i);
    LED_OUT(0x02);
    digitalWrite(RCLK, LOW);
    digitalWrite(RCLK, HIGH);
    //显示第 3 位
    led_table = LED_0F + LED[2];
    i = *led_table;
    LED_OUT(i);
    LED_OUT(0x04);
    digitalWrite(RCLK, LOW);
    digitalWrite(RCLK, HIGH);
    //显示第 4 位
    led_table = LED_0F + LED[3];
    i = *led_table;
    LED_OUT(i);
    LED_OUT(0x08);
    digitalWrite(RCLK, LOW);
    digitalWrite(RCLK, HIGH);
}

void LED_OUT(unsigned char X)
{
    unsigned char i;
    for (i = 8; i >= 1; i--)
    {
        if (X & 0x80)
        {
            digitalWrite(DIO, HIGH);
        }
        else
        {
            digitalWrite(DIO, LOW);
        }
        X <<= 1;
        digitalWrite(SCLK, LOW);
        digitalWrite(SCLK, HIGH);
    }
}
