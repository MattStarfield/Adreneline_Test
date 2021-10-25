int led = A4;                // the pin that the LED is atteched to
int sensor = A0;              // the pin that the sensor is atteched to
int fanPwm = 11;
int state = LOW;             // by default, no motion detected
int val = 0;                 // variable to store the sensor status (value)

int pirStabilizationTime_sec = 30;
bool pirIsStable = false;

uint32_t motionTimer = 0;

void setup() {
  pinMode(led, OUTPUT);      // initalize LED as an output
  pinMode(sensor, INPUT);    // initialize sensor as an input
  pinMode(fanPwm, OUTPUT);
  Serial.begin(115200);        // initialize serial

  digitalWrite(fanPwm, LOW);   // turn Fan Off

  while(!Serial);             // wait here for serial connection
  
  Serial.println("Wait for PIR to stabilize...");
}

void loop()
{
  if(     (millis() > (pirStabilizationTime_sec*1000) )
      &&  (pirIsStable == false)
    )
  {
    pirIsStable = true;
    Serial.println("PIR is Stable!\n\r");
  }

  if(pirIsStable)
  {
    val = digitalRead(sensor);   // read sensor value
    
    if (val == HIGH)             // check if the sensor is HIGH
    {           
      digitalWrite(led, HIGH);   // turn LED ON
      delay(500);                
      
      if (state == LOW) 
      {
        Serial.println("Motion detected!"); 
        state = HIGH;       // update variable state to HIGH

        motionTimer = millis();
      }
    } 
    else 
    {
        digitalWrite(led, LOW); // turn LED OFF
        delay(500);            
        
        if (state == HIGH)
        {
          Serial.print("Motion stopped! ");
          Serial.print(millis() - motionTimer);
          Serial.println(" ms");

          motionTimer =0;
          
          state = LOW;       // update variable state to LOW
          
        }
    }
  } // end pirIsStable
  
}
