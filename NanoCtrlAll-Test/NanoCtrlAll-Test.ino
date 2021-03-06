#include <LCD5110_Basic.h>

//下面这行用于定义LCD用到的设置引脚
LCD5110 myGLCD(9,8,10,11,12); //myGLCD(CLK,DIN,DC,RST,CE)
//下面三行用于引入库的字体
extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t Hanzi12x16[];
extern uint8_t Hanzi12x17[];
extern uint8_t Hanzi16x16[];
//---------------------------------------将暂停键改为自锁式------------------------------
 //下面两行用于定义电机控制输出端口
int input1 = 15; // 定义uno的pin 15 向 input1 输出 
int input2 = 16; // 定义uno的pin 16 向 input2 输出
//下面两行用于定义继电器控制输出端口
int sign1= 17; // 定义uno的pin 17 用于控制电磁阀进水
int sign2= 18; // 定义uno的pin 18 用于控制水泵排水
//下面这行用于定义控制“开始”输入端口
int startbutton = 4; //接中断信号的脚用于控制开始
//下面这行用于定义中断信号控制“暂停”输入端口
int stopbutton = 3; //接中断信号的脚用于控制暂停
//int Signal = 13; //用于指示进入暂停已否的信号指示灯
//下面两行用于将“暂停”点触开关变自锁开关
volatile int stop_state = 1;
volatile int stop_ctrl = 1;
//下面这行用于暂停中持续放/排水
volatile int i= 1;
//下面这行用于定义进/排水时间每摁一次加5秒的输入端口pin 14
int addtimes = 6;
//下面两行用于定义进水、排水时间初始化，单位：秒
volatile long basicintimes = 15;
volatile long basicouttimes = 55;
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
//pinMode(Signal, OUTPUT); 
//digitalWrite(Signal, HIGH); //初始化信号灯为关闭状态
digitalWrite(sign1, HIGH);  //关闭进水阀
digitalWrite(sign2, HIGH);  //关闭排水泵
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
//HIGH:断电 LOW:通电;
for (stop_ctrl = !stop_ctrl;stop_state + stop_ctrl == 1;stop_state = digitalRead(stopbutton))
{
  //下面两行用于调试点触开关变自锁开关用
  Serial.println(stop_state);
  Serial.println(stop_ctrl);
  //！！！！！！！！！！下面这个时间如果变小的话，不能自锁进入暂停！！！！！！！！！！
  delay(100000);
  //-------新增加功能------暂停时停止进/排水、停止马达-------------//
  digitalWrite(sign1, HIGH);  //关闭进水阀
  digitalWrite(sign2, HIGH);  //关闭排水泵
  digitalWrite(input1,HIGH);  //停止马达
  digitalWrite(input2,HIGH); 
  //-------新增加功能------暂停时停止进/排水、停止马达-------------//
  Serial.println("Stopping");

  if(choice == 0) //初始时choice=0，所以显示总界面
  {

  myGLCD.clrScr();
  myGLCD.setFont(Hanzi16x16); //每行5个字
  myGLCD.print("456", 0, 0);
  myGLCD.setFont(Hanzi12x16); //每行7个字（字宽12像素），最多一屏3行
  myGLCD.print("34", 0, 16); //进水
  myGLCD.print("54", 28, 16); //排水
  myGLCD.print("67", 56, 16); //重启
  myGLCD.setFont(Hanzi12x17); //字模量大于9时无法解决选字问题，所以这样
  myGLCD.print("0", 0, 32); //按
  myGLCD.print("3", 16, 32); //3 数字的X轴对应数字在12的基础上加4
  myGLCD.print("4567", 24, 32); //选择功能
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
  if(choice == 1) //显示进水选项页面，按开始键进水，再次按开始键则停止
  {

  myGLCD.clrScr();
  myGLCD.setFont(Hanzi16x16); //每行5个字
  myGLCD.print("456", 0, 0);
  myGLCD.setFont(Hanzi12x17); 
  myGLCD.print("0", 0, 16); //按
  myGLCD.print("1", 16, 16); //1 数字的X轴对应数字在12的基础上加4
  myGLCD.setFont(Hanzi12x16); //每行7个字（字宽12像素），最多一屏3行
  myGLCD.print("34", 24, 16); //进水
  myGLCD.setFont(Hanzi12x17); //字模量大于9时无法解决选字问题，所以这样
  myGLCD.print("0", 0, 32); //按
  myGLCD.print("3", 16, 32); //3 数字的X轴对应数字在12的基础上加4
  myGLCD.print("4567", 24, 32); //选择功能
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
  myGLCD.setFont(Hanzi16x16); //每行5个字
  myGLCD.print("796", 0, 0); //进水中
  myGLCD.setFont(Hanzi12x17); 
  myGLCD.print("0", 0, 16); //按
  myGLCD.print("1", 16, 16); //1 数字的X轴对应数字在12的基础上加4
  myGLCD.setFont(Hanzi12x17); //字模量大于9时无法解决选字问题，所以这样
  myGLCD.print("89", 24, 16); //停止
  myGLCD.print("0", 0, 32); //按
  myGLCD.print("3", 16, 32); //3 数字的X轴对应数字在12的基础上加4
  myGLCD.print("4567", 24, 32); //选择功能
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
  myGLCD.setFont(Hanzi16x16); //每行5个字
  myGLCD.print("456", 0, 0);
  myGLCD.setFont(Hanzi12x17); 
  myGLCD.print("0", 0, 16); //按
  myGLCD.print("1", 16, 16); //1 数字的X轴对应数字在12的基础上加4
  myGLCD.setFont(Hanzi12x16); //每行7个字（字宽12像素），最多一屏3行
  myGLCD.print("54", 24, 16); //排水
  myGLCD.setFont(Hanzi12x17); //字模量大于9时无法解决选字问题，所以这样
  myGLCD.print("0", 0, 32); //按
  myGLCD.print("3", 16, 32); //3 数字的X轴对应数字在12的基础上加4
  myGLCD.print("4567", 24, 32); //选择功能
  delay(150);
  digitalWrite(sign2, HIGH); //先关闭排水泵
  if(digitalRead(addtimes) < 1) 
  {
  delay(50);
  page = !page;
  Serial.println(page);
  choice = choice + page;
  Serial.println(choice);
  }
  if(digitalRead(startbutton) < 1) //下面这段用于打开排水泵
  {
  digitalWrite(sign2, LOW); //打开排水泵
  delay(400000);
 while (digitalRead(startbutton) > 0)
  {
    i++;
    Serial.println("Water Go Outing");

    myGLCD.clrScr();
    myGLCD.setFont(Hanzi16x16); //每行5个字
    myGLCD.print("896", 0, 0); //排水中
    myGLCD.setFont(Hanzi12x17); 
    myGLCD.print("0", 0, 16); //按
    myGLCD.print("1", 16, 16); //1 数字的X轴对应数字在12的基础上加4
    myGLCD.setFont(Hanzi12x17); //字模量大于9时无法解决选字问题，所以这样
    myGLCD.print("89", 24, 16); //停止
    myGLCD.print("0", 0, 32); //按
    myGLCD.print("3", 16, 32); //3 数字的X轴对应数字在12的基础上加4
    myGLCD.print("4567", 24, 32); //选择功能
    delay(150);
  }
  }
  else
  digitalWrite(sign2, HIGH); //关闭排水泵
  Serial.println("22222222222222");
  }
  if(choice == 3) //显示复位页面
  {

  myGLCD.clrScr();
  myGLCD.setFont(Hanzi16x16); //每行5个字
  myGLCD.print("456", 0, 0);
  myGLCD.setFont(Hanzi12x17); 
  myGLCD.print("0", 0, 16); //按
  myGLCD.print("1", 16, 16); //1 数字的X轴对应数字在12的基础上加4
  myGLCD.setFont(Hanzi12x16); //每行7个字（字宽12像素），最多一屏3行
  myGLCD.print("67", 24, 16); //重启
  myGLCD.setFont(Hanzi12x17); //字模量大于9时无法解决选字问题，所以这样
  myGLCD.print("0", 0, 32); //按
  myGLCD.print("3", 16, 32); //3 数字的X轴对应数字在12的基础上加4
  myGLCD.print("4567", 24, 32); //选择功能
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
  Serial.println("4444444444444444444");
  }
  //-------上面这段通过addtime脚的电平变低然后计算判断显示哪个页面-------//

}
}
//-------------------------结束判断是否暂停工作---------------//

void loop(){
  myGLCD.setFont(SmallFont); //设置LCD显示的字体大小
  //-------程序启动时先默认停止进/排水、停止马达-------------//
  digitalWrite(sign1, HIGH);  //关闭进水阀
  digitalWrite(sign2, HIGH);  //关闭排水泵
  digitalWrite(input1,HIGH);  //停止马达
  digitalWrite(input2,HIGH); 
  //-------程序启动时先默认停止进/排水、停止马达-------------//
  
//---------------------开始判断是否开始工作---------------//
for (int state = HIGH;state == HIGH;state = digitalRead(startbutton))
{
  delay(50);
  //Serial.println(state);
  Serial.println("Wait for pin4 Input LOW to start works..");
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
   basicouttimes = basicouttimes + 17.5; //每次添加的时间都是进水时间的3.5倍
   Serial.println(" The basicintimes is change to:");
   Serial.println(basicintimes);
   Serial.println(" The basicouttimes is change to:");
   Serial.println(basicouttimes);
   delay(100);
   }

  myGLCD.clrScr();
  myGLCD.setFont(Hanzi16x16); //每行5个字
  myGLCD.print("0123", 0, 0); //等待开始
  myGLCD.setFont(Hanzi12x16); //每行7个字（字宽12像素），最多一屏3行
  myGLCD.print("34", 0, 16); //进水
  myGLCD.print("54", 30, 16); //排水
  myGLCD.print("01", 60, 16); //还要
  myGLCD.setFont(MediumNumbers);//72对应X轴，40对应Y轴
  myGLCD.print(String(basicintimes), 0, 32); 
  myGLCD.print(String(basicouttimes), 30, 32); 
  myGLCD.print("", 60, 32); 
  delay(50);
  //----------------------结束进/排水时间信号输入------------//
}
Serial.println("Now We Work.");

  myGLCD.clrScr();
  myGLCD.setFont(Hanzi16x16); //每行5个字
  myGLCD.print("236", 0, 0); //开始中
  myGLCD.setFont(Hanzi12x16); //每行7个字（字宽12像素），最多一屏3行
  myGLCD.print("34", 0, 16); //进水
  myGLCD.print("54", 30, 16); //排水
  myGLCD.print("01", 60, 16); //还要
  myGLCD.setFont(MediumNumbers);//72对应X轴，40对应Y轴
  myGLCD.print(String(basicintimes), 0, 32); 
  myGLCD.print(String(basicouttimes), 30, 32); 
  remain = 30 + (basicintimes * 5 +  basicouttimes * 5)/60;//计算剩余时间，30为预计工作总时间,5为大循环个数，除60可得对应分钟数
  myGLCD.print(String(remain), 60, 32); 
  delay(50);
//-------------------------结束判断是否开始工作---------------//

//-------------------------开始判断是否需要浸泡工作---------------//
if(basicintimes == 60) //如果进水时间设定为60秒，则进入浸泡模式
{
  volatile long basicintimes = 15; //重置进水时间为默认
  volatile long basicouttimes = 55; //重置排水时间为默认
  //---------下面用于显示进入浸泡工作用--------//
  myGLCD.clrScr();
  myGLCD.setFont(Hanzi12x16);
  myGLCD.print("24", 0, 0); //泡水
  myGLCD.setFont(Hanzi16x16);
  myGLCD.print("6", 24, 0); //中
  myGLCD.setFont(Hanzi12x16); //每行7个字（字宽12像素），最多一屏3行
  myGLCD.print("01", 52, 16); //还要
  delay(50);
  //---------上面用于显示进入浸泡工作用--------//
   //----------------------控制进水阀进水
  digitalWrite(sign1, LOW); //打开进水阀
  
  delay(basicintimes*1000); //进水的时间默认15秒
  digitalWrite(sign1, HIGH);  //关闭进水阀
  
  delay(4000);
  
 for(int p = 0; p < 4; p++) //一个小循环15秒，4个共1分钟
 {
  Serial.println(p);
  Serial.println("------The PaoShui Loop------");
  
  //---------下面用于计算显示倒计时，已去计算倒计时代码，纯显示用--------//
  myGLCD.clrScr();
  myGLCD.setFont(Hanzi12x16);
  myGLCD.print("24", 0, 0); //泡水
  myGLCD.setFont(Hanzi16x16);
  myGLCD.print("6", 24, 0); //中
  myGLCD.setFont(Hanzi12x16); //每行7个字（字宽12像素），最多一屏3行
  myGLCD.print("01", 52, 16); //还要
  myGLCD.setFont(MediumNumbers);//72对应X轴，40对应Y轴
  myGLCD.print(String((600 - p * 15)/60), 58, 32); 
  delay(50);
  //---------上面用于计算显示倒计时，已去计算倒计时代码，纯显示用--------//
  
  //forward 向前转
  digitalWrite(input1,HIGH); //给高电平-顺时针转
  digitalWrite(input2,LOW);  //给低电平
  
  delay(6500);   //转动6.5秒
            
  //stop 停止
  digitalWrite(input1,HIGH);
  digitalWrite(input2,HIGH);  
  delay(1000);  //停止1秒
  
  //---------下面用于计算显示倒计时，已去计算倒计时代码，纯显示用--------//
  myGLCD.clrScr();
  myGLCD.setFont(Hanzi12x16);
  myGLCD.print("24", 0, 0); //泡水
  myGLCD.setFont(Hanzi16x16);
  myGLCD.print("6", 24, 0); //中
  myGLCD.setFont(Hanzi12x16); //每行7个字（字宽12像素），最多一屏3行
  myGLCD.print("01", 52, 16); //还要
  myGLCD.setFont(MediumNumbers);//72对应X轴，40对应Y轴
  myGLCD.print(String((600 - p * 15)/60), 58, 32); 
  delay(50);
  //---------上面用于计算显示倒计时，已去计算倒计时代码，纯显示用--------//
  
  //back 向后转
  digitalWrite(input1,LOW);  //给低电平-逆时针转
  digitalWrite(input2,HIGH); //给高电平  
   
  delay(6500);  //转动6.5秒

  //stop 停止
  digitalWrite(input1,HIGH);
  digitalWrite(input2,HIGH);  
  
  delay(1000);  //停止1秒
  //------------------下面用于计算显示倒计时---------------------//
  myGLCD.clrScr();
  myGLCD.setFont(Hanzi12x16);
  myGLCD.print("24", 0, 0); //泡水
  myGLCD.setFont(Hanzi16x16);
  myGLCD.print("6", 24, 0); //中
  myGLCD.setFont(Hanzi12x16); //每行7个字（字宽12像素），最多一屏3行
  myGLCD.print("01", 52, 16); //还要
  myGLCD.setFont(MediumNumbers);//72对应X轴，40对应Y轴
  myGLCD.print(String((600 - p * 15)/60), 58, 32); 
  delay(50);
 //------------------上面用于计算显示倒计时---------------------//
  }
  for(int t = 0; t < 54; t++) //此循环用于延时9分钟用
  {
    delay(10000);
    myGLCD.clrScr();
      myGLCD.setFont(Hanzi12x16);
      myGLCD.print("24", 0, 0); //泡水
      myGLCD.setFont(Hanzi16x16);
      myGLCD.print("6", 24, 0); //中
      myGLCD.setFont(Hanzi12x16); //每行7个字（字宽12像素），最多一屏3行
      myGLCD.print("01", 52, 16); //还要
      myGLCD.setFont(MediumNumbers);//72对应X轴，40对应Y轴
      myGLCD.print(String((540 - t * 10)/60), 58, 32); 
  }
 
 //----------------------控制排水泵排水
 digitalWrite(sign2, LOW); //打开排水泵
 
 delay(basicouttimes*1000); //排水的时间默认55秒
 digitalWrite(sign2, HIGH);  //关闭排水泵
 delay(50);
 //-------------------------结束浸泡工作---------------------------//
}
//-------------------------结束判断是否需要浸泡工作---------------//

//-------------------------开始工作---------------------------//
 for(int b = 0; b < 5; b++) //工作5个大循环共30分钟
 {
 Serial.println(b);
 Serial.println("------The Big Loop------");
 delay(3000);  //延时3秒启动

 //----------------------控制进水阀进水
 digitalWrite(sign1, LOW); //打开进水阀
 delay(basicintimes*1000); //进水的时间默认15秒
 digitalWrite(sign1, HIGH);  //关闭进水阀
 
 delay(4000);

 //----------------------控制马达左右转
 for(int c = 0; c < 20; c++) //一个小循环18秒，20个共6分钟
 {
  Serial.println(c);
  Serial.println("------The Small Loop------");
 
 //---------下面用于计算显示倒计时，已去计算倒计时代码，纯显示用--------//

  myGLCD.clrScr();
  myGLCD.setFont(Hanzi16x16); //每行5个字
  myGLCD.print("236", 0, 0); //开始中
  myGLCD.setFont(Hanzi12x16); //每行7个字（字宽12像素），最多一屏3行
  myGLCD.print("01", 52, 16); //还要
  myGLCD.setFont(MediumNumbers);//72对应X轴，40对应Y轴
  myGLCD.print(String(b + 1),0,24);
  myGLCD.print("-",14,24);
  myGLCD.print(String(c + 1),24,24);
  myGLCD.print(String(a), 52, 32); 
 delay(50);
  //---------上面用于计算显示倒计时，已去计算倒计时代码，纯显示用--------//
 
 //forward 向前转
 digitalWrite(input1,HIGH); //给高电平-顺时针转
 digitalWrite(input2,LOW);  //给低电平
 delay(8000);   //转动8秒
            
 //stop 停止
 digitalWrite(input1,HIGH);
 digitalWrite(input2,HIGH);  
 delay(1000);  //停止1秒

 //---------下面用于计算显示倒计时，已去计算倒计时代码，纯显示用--------//

  myGLCD.clrScr();
  myGLCD.setFont(Hanzi16x16); //每行5个字
  myGLCD.print("236", 0, 0); //开始中
  myGLCD.setFont(Hanzi12x16); //每行7个字（字宽12像素），最多一屏3行
  myGLCD.print("01", 52, 16); //还要
  myGLCD.setFont(MediumNumbers);//72对应X轴，40对应Y轴
  myGLCD.print(String(b + 1),0,24);
  myGLCD.print("-",14,24);
  myGLCD.print(String(c + 1),24,24);
  myGLCD.print(String(a), 52, 32); 
 delay(50);
  //---------上面用于计算显示倒计时，已去计算倒计时代码，纯显示用--------//
 
 //back 向后转
 digitalWrite(input1,LOW);  //给低电平-逆时针转
 digitalWrite(input2,HIGH); //给高电平  
 delay(8000);  //转动8秒

 //stop 停止
 digitalWrite(input1,HIGH);
 digitalWrite(input2,HIGH);  
 
 delay(1000);  //停止1秒
 //------------------下面用于计算显示倒计时---------------------//

  myGLCD.clrScr();
  myGLCD.setFont(Hanzi16x16); //每行5个字
  myGLCD.print("236", 0, 0); //开始中
  myGLCD.setFont(Hanzi12x16); //每行7个字（字宽12像素），最多一屏3行
  myGLCD.print("01", 52, 16); //还要
  myGLCD.setFont(MediumNumbers);//72对应X轴，40对应Y轴
  myGLCD.print(String(b + 1),0,24);
  myGLCD.print("-",14,24);
  myGLCD.print(String(c + 1),24,24);
  a = remain - long(c * 18 / 60);//18为一个小循环时间
  myGLCD.print(String(a), 52, 32); 
 delay(50);
 }
 remain = a - long(b * (basicintimes + basicouttimes) / 60); //将前面a的值赋予remain，一会再进循环继续减小
 //------------------上面用于计算显示倒计时---------------------//
 
 //----------------------控制排水泵排水
 digitalWrite(sign2, LOW); //打开排水泵
 delay(basicouttimes*1000); //排水的时间默认55秒
 digitalWrite(sign2, HIGH);  //关闭排水泵
 delay(50);
 //-------------------------结束工作---------------------------//
 }
}

