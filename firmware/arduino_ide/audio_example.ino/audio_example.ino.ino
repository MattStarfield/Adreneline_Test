// Specifically for use with the Adafruit Feather, the pins are pre-set here!

// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>

// These are the pins used
#define PIN_VS1053_RESET   -1     // VS1053 reset pin (not used!)

// Feather M4, M0, 328, nRF52840 or 32u4
  #define PIN_SPI_VS1053_CS       6     // VS1053 chip select pin (output)
  #define PIN_VS1053_DSEL     10     // VS1053 Data/command select pin (output)
  #define PIN_SPI_SD_CS          5     // Card chip select pin
  // DREQ should be an Int pin *if possible* (not possible on 32u4)
  #define PIN_VS1053_DREQ     9     // VS1053 Data request, ideally an Interrupt pin



Adafruit_VS1053_FilePlayer audioPlayer = 
  Adafruit_VS1053_FilePlayer(PIN_VS1053_RESET, PIN_SPI_VS1053_CS, PIN_VS1053_DSEL, PIN_VS1053_DREQ, PIN_SPI_SD_CS);

void setup() {
  Serial.begin(115200);

  // if you're using Bluefruit or LoRa/RFM Feather, disable the radio module
  //pinMode(8, INPUT_PULLUP);

  pinMode(1, INPUT_PULLUP);   // pull up to prevent known bug https://learn.adafruit.com/adafruit-music-maker-featherwing/troubleshooting

  // Wait for serial port to be opened, remove this line for 'standalone' operation
  while (!Serial) { delay(1); }
  delay(500);
  Serial.println("\n\nAdafruit VS1053 Feather Test");
  
  if (! audioPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }

  Serial.println(F("VS1053 found"));
 
  audioPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working
  
  if (!SD.begin(PIN_SPI_SD_CS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }
  Serial.println("SD OK!");
  
  // list files
  printDirectory(SD.open("/"), 0);
  
  // Set volume for left, right channels. lower numbers == louder volume!
  //audioPlayer.setVolume(10,10);
  audioPlayer.setVolume(2,2);
  
  // If DREQ is on an interrupt pin we can do background
  // audio playing
  audioPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int

  
  // Play a file in the background, REQUIRES interrupts!
  Serial.println(F("Playing startup1.mp3"));
  audioPlayer.playFullFile("/startup1.mp3");
  //audioPlayer.startPlayingFile("/startup1.mp3");

  Serial.println(F("Playing countdwn.mp3"));
  audioPlayer.playFullFile("/countdwn.mp3");

  Serial.println(F("Playing begin001.mp3"));
  audioPlayer.playFullFile("/begin001.mp3");

  Serial.println(F("Playing alert001.mp3"));
  audioPlayer.playFullFile("/alert001.mp3");

  Serial.println(F("Playing alert002.mp3"));
  audioPlayer.playFullFile("/alert002.mp3");

  Serial.println(F("Playing alert003.mp3"));
  audioPlayer.playFullFile("/alert003.mp3");

  Serial.println(F("Playing alert004.mp3"));
  audioPlayer.playFullFile("/alert004.mp3");
}

void loop() {
  Serial.print(".");
  // File is playing in the background
  if (audioPlayer.stopped()) {
    Serial.println("Done playing music");
    while (1) {
      delay(10);  // we're done! do nothing...
    }
  }
  if (Serial.available()) {
    char c = Serial.read();
    
    // if we get an 's' on the serial console, stop!
    if (c == 's') {
      audioPlayer.stopPlaying();
    }
    
    // if we get an 'p' on the serial console, pause/unpause!
    if (c == 'p') {
      if (! audioPlayer.paused()) {
        Serial.println("Paused");
        audioPlayer.pausePlaying(true);
      } else { 
        Serial.println("Resumed");
        audioPlayer.pausePlaying(false);
      }
    }
  }
  delay(100);
}



/// File listing helper
void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}
