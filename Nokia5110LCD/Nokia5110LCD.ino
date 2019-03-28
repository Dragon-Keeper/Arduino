#include <LCD5110_Basic.h>

LCD5110 myGLCD(3,4,5,6,7);
extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];
void setup()
{
  myGLCD.InitLCD(); //初始化 LCD
}
 
void loop()
{
myGLCD.setFont(SmallFont);
/*--------------以下为显示满屏Y轴测试--------------
myGLCD.print("01234567890124",CENTER,0);
myGLCD.print("12345678901245",CENTER,8);
myGLCD.print("23456789012456",CENTER,16);
myGLCD.print("34567890124567",CENTER,24);
myGLCD.print("45678901245678",CENTER,32);
myGLCD.print("56789012456789",CENTER,40);
----------------以上为显示满屏Y轴测试------------*/
/*--------------以下为显示满屏X轴测试--------------
myGLCD.print("0",0,0);
myGLCD.print("1",6,0);
myGLCD.print("2",12,0);
myGLCD.print("3",18,0);
myGLCD.print("4",24,0);
myGLCD.print("5",30,0);
myGLCD.print("6",36,0);
myGLCD.print("7",42,0);
myGLCD.print("8",48,0);
myGLCD.print("9",54,0);
myGLCD.print("1",60,0);
myGLCD.print("2",66,0);
myGLCD.print("3",72,0);
myGLCD.print("4",78,0);
----------------以上为显示满屏X轴测试------------*/
myGLCD.print("Start:",0,0);
myGLCD.print("Stop:",0,8);
myGLCD.print("In/Out:",0,16);
myGLCD.print("Remain:",0,24);
delay(1000);
myGLCD.clrScr();
 
}
