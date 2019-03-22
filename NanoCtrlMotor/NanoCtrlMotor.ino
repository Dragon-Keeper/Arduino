//下面两行用于定义电机控制输出端口
int input1 = 8; // 定义uno的pin 3 向 input1 输出 
int input2 = 9; // 定义uno的pin 2 向 input2 输出

void setup() {
//  Serial.begin (9600);
//初始化各IO,模式为OUTPUT 输出模式
pinMode(input1,OUTPUT);
pinMode(input2,OUTPUT);
}

void loop() {
 delay(3000);  //延时3秒启动
 //forward 向前转
 digitalWrite(input1,HIGH); //给高电平
 digitalWrite(input2,LOW);  //给低电平
 delay(3000);   //转动3秒

 //stop 停止
 digitalWrite(input1,LOW);
 digitalWrite(input2,LOW);  
 delay(2000);  //停止2秒
 
 //back 向后转
 digitalWrite(input1,LOW);
 digitalWrite(input2,HIGH);   
 delay(3000);  //转动3秒

  //stop 停止
 digitalWrite(input1,LOW);
 digitalWrite(input2,LOW);  
 delay(3000);  //停止3秒
 
}
