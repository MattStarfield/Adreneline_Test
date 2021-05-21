
int led = 13;           // the PWM pin the LED is attached to
int fan = 11;
int tach = 12;

int period_ms = 5000;

unsigned long tachCount = 0;

unsigned long currentMillis = 0;

void ISR()
{
  tachCount++;
  Serial.println(tachCount);
}

// the setup routine runs once when you press reset:
void setup() 
{
  // declare pin 9 to be an output:
  pinMode(led, OUTPUT);
  pinMode(fan, OUTPUT);
  pinMode(tach, INPUT_PULLUP);
  
  Serial.begin(115200);

  while(!Serial){}
  
  attachInterrupt(digitalPinToInterrupt(tach), ISR, FALLING);

  currentMillis = millis();

  Serial.println("start!");
}

// the loop routine runs over and over again forever:
void loop() 
{

  while((millis() - currentMillis) <= period_ms)
  {
    analogWrite(fan, 255);
    analogWrite(led, 255);
  }
  
  currentMillis = millis();
  
  while((millis() - currentMillis) <= period_ms)
  {
    analogWrite(fan, 0);
    analogWrite(led, 0);
  }

  currentMillis = millis();
  
  while((millis() - currentMillis) <= period_ms)
  {
    analogWrite(fan, 52);
    analogWrite(led, 52);
  }

  currentMillis = millis();
  
  while((millis() - currentMillis) <= period_ms)
  {
    analogWrite(fan, 100);
    analogWrite(led, 100);
  }

  currentMillis = millis();

}
