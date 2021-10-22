/****************************************
Example Sound Level Sketch for the 
Adafruit Microphone Amplifier
****************************************/


#define PIN_MIC             A3
#define PIN_FAN_PWM         11

#define VCC                 3.3 //volts
#define SERIAL_BAUD_RATE    115200
#define GAIN_FACTOR         115     // measure here: https://forums.adafruit.com/viewtopic.php?f=8&t=179776&p=875029&hilit=Electret+Microphone+Amplifier#p875175

#define DB_CALIBRATION      32      // dB meter reading with mic @ ~0V

const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

void setup() 
{
   Serial.begin(SERIAL_BAUD_RATE);

   pinMode(PIN_MIC, INPUT);
   pinMode(PIN_FAN_PWM, OUTPUT);

   digitalWrite(PIN_FAN_PWM, LOW);
}


void loop() 
{
   unsigned long startMillis= millis();  // Start of sample window
   unsigned int peakToPeak = 0;   // peak-to-peak level

   unsigned int signalMax = 0;      // initial value to estalish MAX
   unsigned int signalMin = 1024;   // initial value to estalish MIN

   // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(PIN_MIC);
      if (sample < 1024)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
   
   double volts = (peakToPeak * (double)VCC) / 1024.0;  // convert to volts

   double dB = 20.0*log(10)*(volts/VCC);

   Serial.print(volts);
   Serial.print(" , ");
   Serial.println(dB + DB_CALIBRATION); // 
}
