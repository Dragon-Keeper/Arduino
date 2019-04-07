 #include <LCD5110_Basic.h>

//下面这行用于定义LCD用到的设置引脚
LCD5110 myGLCD(9,8,10,11,12); //myGLCD(CLK,DIN,DC,RST,CE)
//下面三行用于引入库的字体
extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];

//下面这行用于定义控制“开始”输入端口
int startbutton = 19; //接中断信号的脚用于控制开始
//下面这行用于定义中断信号控制“暂停”输入端口
int stopbutton = 2; //接中断信号的脚用于控制暂停
volatile int stop_state = 1;
volatile int stop_ctrl = 1;

void setup() {
Serial.begin (9600);
myGLCD.InitLCD(); //初始化 LCD
/* -----------下面是下一句代码里：INPUT_PULLUP的用法解释-------------
 * 1.由于Arduino上电后，数字I/O管脚处于悬空状态，此时通过digitalRead()读到的是一个不稳定的值(可能是高，也可能是低)。
 * 所以通过pinMode()函数设置按键引脚为上拉输入模式。即使用内部上拉电阻，按键未按下时，引脚将为高电平，按键按下为低电平。
 * 2.可以不使用内部上拉电阻，在电路上添加按键的上拉电阻或下拉电阻，可达到相同效果。
 */
pinMode(startbutton, INPUT_PULLUP); //设置管脚为开始信号输入
pinMode(stopbutton, INPUT_PULLUP); //设置管脚为暂停信号输入
attachInterrupt(digitalPinToInterrupt(stopbutton), stop, LOW); //监视中断输入引脚3的电平是否低电平
}
//HIGH:断电:1
// LOW:通电:0
//-------------------------开始判断是否暂停工作---------------//

void stop(){
//HIGH:断电 LOW:通电OW;
for (stop_ctrl = !stop_ctrl;stop_state + stop_ctrl == 1;stop_state = digitalRead(stopbutton))
{
  //下面两行用于调试点触开关变自锁开关用
  Serial.println(stop_state);
  Serial.println(stop_ctrl);
  delay(100000);
  Serial.println("Stopping");
  myGLCD.clrScr();
  myGLCD.print("Sopping",CENTER,0);
  delay(50);
}
}
//-------------------------结束判断是否暂停工作---------------//

void loop(){
  myGLCD.setFont(SmallFont); //设置LCD显示的字体大小
  
//---------------------开始判断是否开始工作---------------//
for (int state = HIGH;state == HIGH;state = digitalRead(startbutton))
{
  delay(50);
  //Serial.println(state);
  Serial.println("Wait for pin19 Input LOW to start works..");
  //下面两行用于调试点触开关变自锁开关用
  Serial.println(stop_state);
  Serial.println(stop_ctrl);
  delay(50);
Serial.println("Now We Work.");
myGLCD.clrScr();
myGLCD.print("Working",CENTER,0);
delay(50);
//-------------------------结束判断是否开始工作---------------//
}
}
