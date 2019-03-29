int pin4;
int pin5;

void setup() {
  pinMode(4,INPUT);   //设置引脚 4 为输入模式
  pinMode(5,INPUT);   //设置引脚 5 为输入模式
  Serial.begin(9600); //设置波特率为9600
}
 
void loop() {
    /*
  Serial.println(analogRead(4)); //串口输出 4读取到的值
  pin4 = digitalRead(4);
  Serial.println(pin4);
  delay(500);
  Serial.println("-------------4------------");
  */
  Serial.println(analogRead(5)); //串口输出 5读取到的值
  pin5 = digitalRead(5);
  Serial.println(pin5);
  delay(500);
  Serial.println("-------------5------------");

}
