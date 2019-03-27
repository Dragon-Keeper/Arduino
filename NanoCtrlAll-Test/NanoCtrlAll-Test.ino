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
//下面这行用于定义进/排水时间每摁一次加5秒的输入端口pin 14
int addtimes = 14;
//下面两行用于定义进水、排水时间初始化，单位：秒
long basicintimes = 10;
long basicouttimes = 20;

void setup() {
Serial.begin (9600);
//初始化各IO,模式为OUTPUT 输出模式
pinMode(input1,OUTPUT);
pinMode(input2,OUTPUT);
pinMode(sign1, OUTPUT); 
pinMode(sign2, OUTPUT); 
/* -----------下面是下一句代码里：INPUT_PULLUP的用法解释-------------
 * 1.由于Arduino上电后，数字I/O管脚处于悬空状态，此时通过digitalRead()读到的是一个不稳定的值(可能是高，也可能是低)。
 * 所以通过pinMode()函数设置按键引脚为上拉输入模式。即使用内部上拉电阻，按键未按下时，引脚将为高电平，按键按下为低电平。
 * 2.可以不使用内部上拉电阻，在电路上添加按键的上拉电阻或下拉电阻，可达到相同效果。
 */
pinMode(addtimes, INPUT_PULLUP); //设置管脚为进/排水时间信号输入上拉，令到它值为HIGH
pinMode(startbutton, INPUT); //设置管脚为开始信号输入
pinMode(stopbutton, INPUT); //设置管脚为暂停信号输入
attachInterrupt(digitalPinToInterrupt(stopbutton), stop, HIGH); //监视中断输入引脚3的电平是否低电平
}
//HIGH:断电:1
// LOW:通电:0
//-------------------------用于判断是否暂停工作---------------//
void stop(){
//HIGH:断电 LOW:通电
for (int stop = HIGH;stop == HIGH;stop = digitalRead(stopbutton))
{
  delay(500);
  Serial.println(stop);
  Serial.println("The pin3 Input HIGH.So now We Stopping.");
}
}
//-------------------------结束判断是否暂停工作---------------//

void loop(){
//-------------------------用于判断是否开始工作---------------//
for (int state = HIGH;state == HIGH;state = digitalRead(startbutton))
{
  delay(50);
  Serial.println(state);
  Serial.println("Wait for pin2 Input LOW.So now We Stopping.");
  delay(1000);
  //-------------------------用于进/排水时间信号输入------------//
  int timestate = digitalRead(addtimes);
  Serial.println(" The timestate is:");
  Serial.println(timestate);
  if(timestate == LOW)
  {
   Serial.println(" The basicintimes is:");
   Serial.println(basicintimes);
   delay(1000);
   basicintimes = basicintimes + 5;
   basicouttimes = basicouttimes + 5;
   Serial.println(" The basicintimes is change to:");
   Serial.println(basicintimes);
   Serial.println(" The basicouttimes is change to:");
   Serial.println(basicouttimes);
   delay(1000);
   }
  //-------------------------用于进/排水时间信号输入------------//
}
Serial.println("Now We Work.");
//-------------------------结束判断是否开始工作---------------//

//-------------------------开始工作---------------------------//
 for(int b = 0; b < 4; b++)
 {
 Serial.println(b);
 Serial.println("------------");
 delay(3000);  //延时3秒启动
 //----------------------控制进水阀进水
 digitalWrite(sign1, HIGH); //打开进水阀
 delay(basicintimes*1000); //进水的时间默认10秒
 digitalWrite(sign1, LOW);  //关闭进水阀
 delay(4000);

 //----------------------控制马达左右转
 for(int c = 0; c < 61; c++)
 {
  Serial.println(c);
  Serial.println("-------------------------------");
 //forward 向前转------一个循环10秒
 digitalWrite(input1,HIGH); //给高电平-顺时针转
 digitalWrite(input2,LOW);  //给低电平
 delay(3000);   //转动3秒

 //stop 停止
 digitalWrite(input1,LOW);
 digitalWrite(input2,LOW);  
 delay(2000);  //停止2秒
 
 //back 向后转
 digitalWrite(input1,LOW);  //给低电平-逆时针转
 digitalWrite(input2,HIGH); //给高电平   
 delay(3000);  //转动3秒

 //stop 停止
 digitalWrite(input1,LOW);
 digitalWrite(input2,LOW);  
 delay(2000);  //停止2秒
 }

 //----------------------控制排水阀排水
 digitalWrite(sign2, HIGH); //打开排水阀
 delay(basicouttimes*1000); //排水的时间默认20秒
 digitalWrite(sign2, LOW);  //关闭排水阀
 delay(2000);

 //-------------------------结束工作---------------------------//
 }
}



