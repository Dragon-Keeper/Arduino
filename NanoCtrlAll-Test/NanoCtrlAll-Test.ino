 #include <LCD5110_Basic.h>

//下面这行用于定义LCD用到的设置引脚
LCD5110 myGLCD(9,8,10,11,12); //myGLCD(CLK,DIN,DC,RST,CE)
//下面三行用于引入库的字体
extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];

 //下面两行用于定义电机控制输出端口
int input1 = 15; // 定义uno的pin 15 向 input1 输出 
int input2 = 16; // 定义uno的pin 16 向 input2 输出
//下面两行用于定义继电器控制输出端口
int sign1= 17; // 定义uno的pin 17 用于控制进水
int sign2= 18; // 定义uno的pin 18 用于控制排水
//下面这行用于定义控制“开始”输入端口
int startbutton = 19; //接中断信号的脚用于控制开始
//下面这行用于定义中断信号控制“暂停”输入端口
int stopbutton = 2; //接中断信号的脚用于控制暂停
//下面两行用于将“暂停”点触开关变自锁开关
volatile int stop_state = 1;
volatile int stop_ctrl = 1;
//下面这行用于暂停中持续放/排水
volatile int i= 1;
//下面这行用于定义进/排水时间每摁一次加5秒的输入端口pin 14
int addtimes = 14;
//下面两行用于定义进水、排水时间初始化，单位：秒
volatile long basicintimes = 20;
volatile long basicouttimes = 40;
//下面这行定义函数用于计算剩余时间
volatile long remain;
volatile long a;
//下面这行定义函数用于暂停时判断显示不同页面
volatile int choice = 0; 
volatile int page = 1; 
//下面这行定义函数用于重置系统
/*不使用任何硬件引脚，Arduino有一个名为resetFunc（）的内置函数，
 * 我们声明函数地址为0，当我们执行此功能时，Arduino将自动重置*/
void(* resetFunc) (void) = 0;

void setup() {
Serial.begin (9600);
myGLCD.InitLCD(); //初始化 LCD
//初始化各IO,模式为OUTPUT 输出模式
pinMode(input1,OUTPUT);
pinMode(input2,OUTPUT);
pinMode(sign1, OUTPUT); 
pinMode(sign2, OUTPUT); 
//digitalWrite(sign1, HIGH);  //关闭进水阀
//digitalWrite(sign2, HIGH);  //关闭排水阀
//digitalWrite(input1,HIGH);  //停止马达
//digitalWrite(input2,HIGH); 
/* -----------下面是下一句代码里：INPUT_PULLUP的用法解释-------------
 * 1.由于Arduino上电后，数字I/O管脚处于悬空状态，此时通过digitalRead()读到的是一个不稳定的值(可能是高，也可能是低)。
 * 所以通过pinMode()函数设置按键引脚为上拉输入模式。即使用内部上拉电阻，按键未按下时，引脚将为高电平，按键按下为低电平。
 * 2.可以不使用内部上拉电阻，在电路上添加按键的上拉电阻或下拉电阻，可达到相同效果。
 */
pinMode(addtimes, INPUT_PULLUP); //设置管脚为进/排水时间信号输入上拉，令到它值为HIGH
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
  //！！！！！！！！！！下面这个时间如果变小的话，不能自锁进入暂停！！！！！！！！！！
  delay(100000);
  //-------新增加功能------暂停时停止进/排水、停止马达-------------//
  digitalWrite(sign1, HIGH);  //关闭进水阀
  digitalWrite(sign2, HIGH);  //关闭排水阀
  digitalWrite(input1,HIGH);  //停止马达
  digitalWrite(input2,HIGH); 
  //-------新增加功能------暂停时停止进/排水、停止马达-------------//
  /*
  myGLCD.clrScr();
  myGLCD.print("Stopping",CENTER,0);
  myGLCD.print("In/Out:",0,8);
  myGLCD.print(String(basicintimes),44,8);//由于时间是整数，所以要转换成字符串
  myGLCD.print("/",56,8);
  myGLCD.print(String(basicouttimes),62,8);
  myGLCD.print("Remain:Null",0,16);
  myGLCD.print("More...",0,24);
  delay(1000);
  */
  Serial.println("Stopping");

  if(choice == 0) //初始时choice=0，所以显示总界面
  {
  myGLCD.clrScr();
  myGLCD.print("Stopping",CENTER,0);
  myGLCD.print("Here You Can",CENTER,8);
  myGLCD.print("Water In/Out",CENTER,16);
  myGLCD.print("Reset System",CENTER,24);
  myGLCD.print("Click More...",0,32);
  delay(150);
  //-------下面这段通过addtime脚的电平变低然后计算判断显示哪个页面-------//
  if(digitalRead(addtimes) < 1) 
  {
  delay(50);
  /*
   * page=1，page=!page这句将page赋值为其反转值，1变为0，本来choice=0，所以
   * choice = choice + page结果为：choice = 0 + 0 =0 .所以依然在本语句内循环。
   * 但再次循环时由于page已经赋值为0，所以page=!page这句将page赋值为其反转值
   * page赋值为1，所以choice = choice + page结果为：choice = 0 + 1 =1. 由于
   * choice=1，所以跳出本循环到choice==1循环中。
   * 在下个循环中如果检测到按键，依然按照上面两次循环后+1到下一个循环。
  */
  page = !page; 
  Serial.println(page);
  choice = choice + page;
  Serial.println(choice);
  }
  Serial.println("000000000000000");
  }
  if(choice == 1) //显示进水选项页面，按住开始键时进水，放开则停止
  {
  myGLCD.clrScr();
  myGLCD.print("Stopping",CENTER,0);
  myGLCD.print("Water In:Yes?",0,16);
  myGLCD.print("Press To Start",CENTER,24);
  myGLCD.print("Click More...",0,32);
  delay(150);
  digitalWrite(sign1, HIGH); //先关闭进水阀
  Serial.println(digitalRead(sign1));
  Serial.println(digitalRead(sign2));
  if(digitalRead(addtimes) < 1) 
  {
  delay(50);
  page = !page;
  Serial.println(page);
  choice = choice + page;
  Serial.println(choice);
  }
  
  //-----下面这段通过startbutton脚的电平变低然后计算判断是否执行命令-----//
  if(digitalRead(startbutton) < 1)
  {
  digitalWrite(sign1, LOW); //打开进水阀
    /*-----官网已清楚说明delay在中断内不正常------//
  * Inside the attached function, delay() won't work and the value returned by millis() will not increment.
  * 就是说：在中断内，delay()不能正常工作，本来中断就是短频快的东西，
  * 一般这种单CPU单线程执行的设计中，都不希望在中断中做太多事，做太多事，
  * 其它事情都会被阻塞在哪里。。
  * 所以中断内的delay(1000)可能不到1秒，要实现通电效果的话，就要将它设置很大
  * 这里delay(200000)等于正常的delay(1000);所以约为200倍。
  *-----官网已清楚说明delay在中断内不正常------*/
  delay(400000); //添加2秒延时以防破坏逻辑影响下面的循环
   //-----下面是一个循环，等待按start键打破循环并且关闭进水阀------//
  while (digitalRead(startbutton) > 0)
  {
    i++;
    Serial.println("Water In Coming");
    myGLCD.clrScr();
    myGLCD.print("Stopping",CENTER,0);
    myGLCD.print("Water's Coming",0,16);
    myGLCD.print("Press To Stop",CENTER,24);
    myGLCD.print("Click More...",0,32);
    delay(150);
    Serial.println(digitalRead(sign1));
    Serial.println(digitalRead(sign2));
  }
  }
  else
  digitalWrite(sign1, HIGH); //关闭进水阀

  //-----上面这段通过startbutton脚的电平变低然后计算判断是否执行命令-----//
  Serial.println("111111111111111");
  }
  
  if(choice == 2) //显示排水选项页面，按住开始键时排水，放开则停止
  {
  myGLCD.clrScr();
  myGLCD.print("Stopping",CENTER,0);
  myGLCD.print("Water Out:Yes?",0,16);
  myGLCD.print("Press To Start",CENTER,24);
  myGLCD.print("Click More...",0,32);
  delay(150);
  digitalWrite(sign2, HIGH); //先关闭排水阀
  if(digitalRead(addtimes) < 1) 
  {
  delay(50);
  page = !page;
  Serial.println(page);
  choice = choice + page;
  Serial.println(choice);
  }
  if(digitalRead(startbutton) < 1) //下面这段用于打开排水阀
  {
  digitalWrite(sign2, LOW); //打开排水阀
  delay(400000);
 while (digitalRead(startbutton) > 0)
  {
    i++;
    Serial.println("Water Go Outing");
    myGLCD.clrScr();
    myGLCD.print("Stopping",CENTER,0);
    myGLCD.print("Water's Outing",0,16);
    myGLCD.print("Press To Stop",CENTER,24);
    myGLCD.print("Click More...",0,32);
    delay(150);
  }
  }
  else
  digitalWrite(sign2, HIGH); //关闭排水阀
  Serial.println("22222222222222");
  }
  if(choice == 3) //显示复位页面
  {
  myGLCD.clrScr();
  myGLCD.print("Stopping",CENTER,0);
  myGLCD.print("Reset System",CENTER,16);
  myGLCD.print("Press To Reset",CENTER,24);
  myGLCD.print("No More...",0,32);
  delay(150);
  if(digitalRead(addtimes) < 1) 
  {
  delay(50);
  page = !page;
  Serial.println(page);
  choice = choice + page;
  Serial.println(choice);
  }
  if(digitalRead(startbutton) < 1) //下面这段用于重置系统
  resetFunc(); //这句用于调用重置命令
  Serial.println("33333333333333333");
  }
  if(choice == 4) //用于返回第一个页面
  {
 //自动跳出循环
  Serial.println(choice);
  delay(150);
  choice = 0;
  Serial.println("444444444444444444");
  }
  //-------上面这段通过addtime脚的电平变低然后计算判断显示哪个页面-------//

}
}
//-------------------------结束判断是否暂停工作---------------//

void loop(){
  myGLCD.setFont(SmallFont); //设置LCD显示的字体大小
  //-------程序启动时先默认停止进/排水、停止马达-------------//
  digitalWrite(sign1, HIGH);  //关闭进水阀
  digitalWrite(sign2, HIGH);  //关闭排水阀
  digitalWrite(input1,HIGH);  //停止马达
  digitalWrite(input2,HIGH); 
  //-------程序启动时先默认停止进/排水、停止马达-------------//
  
//---------------------开始判断是否开始工作---------------//
for (int state = HIGH;state == HIGH;state = digitalRead(startbutton))
{
  delay(50);
  //Serial.println(state);
  Serial.println("Wait for pin19 Input LOW to start works..");
  Serial.println(digitalRead(sign1));
  Serial.println(digitalRead(sign2));
  //下面两行用于调试点触开关变自锁开关用
  //Serial.println(stop_state);
  //Serial.println(stop_ctrl);
  delay(50);
  //---------------------开始进/排水时间信号输入------------//
  int timestate = digitalRead(addtimes);
  /*
  Serial.println(" The timestate is:");
  Serial.println(timestate);
  */
  if(timestate == LOW)
  {
/*
   Serial.println(" The basicintimes is:");
   Serial.println(basicintimes);
   delay(100);
*/
   basicintimes = basicintimes + 5;
   basicouttimes = basicouttimes + 5;
   Serial.println(" The basicintimes is change to:");
   Serial.println(basicintimes);
   Serial.println(" The basicouttimes is change to:");
   Serial.println(basicouttimes);
   delay(100);
   }
  myGLCD.clrScr();
  myGLCD.print("Wait For Start",CENTER,0);
  myGLCD.print("In/Out:",0,16);
  myGLCD.print(String(basicintimes),44,16);//由于时间是整数，所以要转换成字符串
  myGLCD.print("/",56,16);
  myGLCD.print(String(basicouttimes),62,16);
  myGLCD.print("Remain:Null",0,24);
  delay(50);
  //----------------------结束进/排水时间信号输入------------//
}
Serial.println("Now We Work.");
myGLCD.clrScr();
myGLCD.print("Working",CENTER,0);
myGLCD.print("In/Out:",0,16);
myGLCD.print(String(basicintimes),44,16);//由于时间是整数，所以要转换成字符串
myGLCD.print("/",56,16);
myGLCD.print(String(basicouttimes),62,16);
myGLCD.print("Remain:",0,24);
remain = 30 + (basicintimes * 3 +  basicouttimes * 3)/60;//计算剩余时间
myGLCD.print(String(remain),44,24);
delay(50);
//-------------------------结束判断是否开始工作---------------//

//-------------------------开始工作---------------------------//
 for(int b = 0; b < 5; b++) //工作5个大循环共30分钟
 {
 Serial.println(b);
 Serial.println("------The Big Loop------");
 delay(3000);  //延时3秒启动

 //----------------------控制进水阀进水
 digitalWrite(sign1, LOW); //打开进水阀
 delay(basicintimes*1000); //进水的时间默认20秒
 digitalWrite(sign1, HIGH);  //关闭进水阀
 delay(4000);

 //----------------------控制马达左右转
 for(int c = 0; c < 36; c++) //一个小循环10秒，36个共6分钟
 {
  Serial.println(c);
  Serial.println("------The Small Loop------");
  
 //---------下面用于计算显示倒计时，已去计算倒计时代码，纯显示用--------//
 myGLCD.clrScr();
 myGLCD.print("Working",CENTER,0);
 myGLCD.print("In/Out:",0,16);
 myGLCD.print(String(basicintimes),44,16);//由于时间是整数，所以要转换成字符串
 myGLCD.print("/",56,16);
 myGLCD.print(String(basicouttimes),62,16);
 myGLCD.print("Remain:",0,24);
 myGLCD.print(String(a),44,24);
 delay(50);
  //---------上面用于计算显示倒计时，已去计算倒计时代码，纯显示用--------//
  
 //forward 向前转
 digitalWrite(input1,HIGH); //给高电平-顺时针转
 digitalWrite(input2,LOW);  //给低电平
 delay(3000);   //转动3秒

 //stop 停止
 digitalWrite(input1,HIGH);
 digitalWrite(input2,HIGH);  
 delay(2000);  //停止2秒
 
 //---------下面用于计算显示倒计时，已去计算倒计时代码，纯显示用--------//
 myGLCD.clrScr();
 myGLCD.print("Working",CENTER,0);
 myGLCD.print("In/Out:",0,16);
 myGLCD.print(String(basicintimes),44,16);//由于时间是整数，所以要转换成字符串
 myGLCD.print("/",56,16);
 myGLCD.print(String(basicouttimes),62,16);
 myGLCD.print("Remain:",0,24);
 myGLCD.print(String(a),44,24);
 delay(50);
  //---------上面用于计算显示倒计时，已去计算倒计时代码，纯显示用--------//
 
 //back 向后转
 digitalWrite(input1,LOW);  //给低电平-逆时针转
 digitalWrite(input2,HIGH); //给高电平   
 delay(3000);  //转动3秒

 //stop 停止
 digitalWrite(input1,HIGH);
 digitalWrite(input2,HIGH);  
 delay(2000);  //停止2秒

 //------------------下面用于计算显示倒计时---------------------//
 myGLCD.clrScr();
 myGLCD.print("Working",CENTER,0);
 myGLCD.print("In/Out:",0,16);
 myGLCD.print(String(basicintimes),44,16);//由于时间是整数，所以要转换成字符串
 myGLCD.print("/",56,16);
 myGLCD.print(String(basicouttimes),62,16);
 myGLCD.print("Remain:",0,24);
 a = remain - long(c * 10 / 60);
 myGLCD.print(String(a),44,24);
 delay(50);
 }
 remain = a - long(b * (basicintimes + basicouttimes) / 60); //将前面a的值赋予remain，一会再进循环继续减小
 //------------------上面用于计算显示倒计时---------------------//

 //----------------------控制排水阀排水
 digitalWrite(sign2, LOW); //打开排水阀
 delay(basicouttimes*1000); //排水的时间默认40秒
 digitalWrite(sign2, HIGH);  //关闭排水阀

 delay(50);
 //-------------------------结束工作---------------------------//
 }
}

