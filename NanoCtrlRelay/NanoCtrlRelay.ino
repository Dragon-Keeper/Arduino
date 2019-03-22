int sign= 4;
void setup(){
 pinMode(sign, OUTPUT); 
}
 
void loop(){
  delay(2000);
  digitalWrite(sign, HIGH);
  delay(10000);
  digitalWrite(sign, LOW); 
}
