float pH_Value;
int pH_analog; 
float Voltage;

void setup() 
{ 
  Serial.begin(9600);
  pinMode(pH_Value, INPUT); 
} 
 
void loop() 
{ 
  pH_analog = analogRead(A0); 
  Voltage = pH_analog * (5.0 / 1023.0); 
  pH_Value = 7 - (7/2.5)*(Voltage-2.5);
  Serial.println(pH_Value); 
   delay(500); 
  Serial.println("Voltage->"); 
  Serial.println(Voltage);
  delay(500); 
}