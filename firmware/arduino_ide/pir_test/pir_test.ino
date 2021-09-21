int led = A4;                // the pin that the LED is atteched to
int sensor = A0;              // the pin that the sensor is atteched to
int fanPwm = 11;
int state = LOW;             // by default, no motion detected
int val = 0;                 // variable to store the sensor status (value)

int pirStabilizationTime_sec = 45;
bool pirIsStable = false;

void setup() {
  pinMode(led, OUTPUT);      // initalize LED as an output
  pinMode(sensor, INPUT);    // initialize sensor as an input
  pinMode(fanPwm, OUTPUT);
  Serial.begin(115200);        // initialize serial

  digitalWrite(fanPwm, LOW);   // turn Fan Off

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
    if (val == HIGH) {           // check if the sensor is HIGH
      digitalWrite(led, HIGH);   // turn LED ON
      delay(500);                // delay 100 milliseconds 
      
      if (state == LOW) {
        Serial.println("Motion detected!"); 
        state = HIGH;       // update variable state to HIGH
      }
    } 
    else {
        digitalWrite(led, LOW); // turn LED OFF
        delay(500);             // delay 200 milliseconds 
        
        if (state == HIGH){
          Serial.println("Motion stopped!");
          state = LOW;       // update variable state to LOW
      }
    }
  } // end pirIsStable
  
}
