/*
//==============================================================================
  Project: Either-Or
  Client: AA Plasma / Greg Fridman

  Developer: Matt Starfield (matt.starfield@nextfab.com) | NextFab
  GitHub:    https://github.com/MattStarfield
//==============================================================================
  * Description:

  * CHANGELOG: see GitHub repo for commit history

  * TODOs: search file for "TODO"
    * Setup default pins selections to be compatible with Nano and Feather boards
      * use #ifdef / #endif pre-processor conditionals
    * reduce / optimize program size
      * enclose Simple-CLI lib code in #ifdef / #endif pre-processor conditionals
      * use small buffers with frequent output for large print statements (in SimpleCLI command callbacks)
    * incorporate power optimization, sleep cycles (see Adafruit_SleepyDog_Library)
    * change bool flags to bytes
      * ++ to set flag, -- to clear flag
      * allows for use of "stacked flags" / counts
//==============================================================================
*/

//==============================================================================
// == #include Dependencies == //
//==============================================================================

  // #include < > ONLY looks for files in the default "libraries" folder (~/Arduino/libraries)
  // #include " " looks for files in the current sketch folder 1st, then in the default "libraries" folder

  // Application Files
  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    #include "app_config.h"                 // customizable settings for app, #define MACROS
    #include "fsm_config.h"                 // define states of finite state machine
    #include "device_mode_config.h"         // define operating modes of device (e.g. standard, first run, burn-in, etc...)
    //#include "pixel_config.h"               // define configuration and scenes for NeoPixels

  // Arduino Standard libraries
  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    #include <Arduino.h>                    // includes all AVR & Arduino-specific definfitions (case sensitive file name on Linux)
    #include <Wire.h>                     // I2C
    #include <SPI.h>                      // SPI
    #include <SD.h>                       // SD card
    //#include <SoftwareSerial.h>           // Create additional Serial Port to communicate with ext. Serial Peripherals

  // C Standard libraries
  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //#include <string.h>                   // http://www.cplusplus.com/reference/cstring/
    //#include <stdio.h>                    // http://www.cplusplus.com/reference/cstdio/
    //#include <ctype.h>                    // http://www.cplusplus.com/reference/cctype/
    //#include <math.h>                     // http://www.cplusplus.com/reference/cmath/
    //#include <time.h>                     // http://www.cplusplus.com/reference/ctime/


  // Contributor libraries
  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  // see platformio.ini lib_deps

  // Debugging                            // Sets level of print statements for debugging
    #include "Arduino_DebugUtils.h"       // https://github.com/arduino-libraries/Arduino_DebugUtils
    #include "ArduinoTrace.h"             // https://github.com/bblanchon/ArduinoTrace

  // Watchdog Timer                       // Forces system reboot if master loop time takes too long (e.g. infinite loop)
    #include "Adafruit_SleepyDog.h"       // https://github.com/adafruit/Adafruit_SleepyDog

  // Math / Statistics                    // Used for Loop Statistics and analogRead Smoothing
    #include "Average.h"                  // https://github.com/MajenkoLibraries/Average

  // Unique ID, Mfr SN                    // Gets the MCU's Manufacturer Assigned Identifier
    #include "ArduinoUniqueID.h"          // https://github.com/ricaun/ArduinoUniqueID

  // RAM Usage                            // Provides available memory stats while device is in use
    #include "MemoryFree.h"               // https://github.com/mpflaga/Arduino-MemoryFree

  // UI: Buttons                          // Facilitates button assignment and multi-function (e.g. long press, double tap)
    #include "EasyButton.h"               // https://easybtn.earias.me/docs/fundamentals
    // investigate AceButton for simpler multifunction used         // https://github.com/bxparks/AceButton

  // UI: LEDs                            // Allows Blink patterns to be set and run autonomously
    #include "EasyLed.h"                 // https://github.com/evert-arias/EasyLed
    // investigate jLED for more advanced functions                 // https://github.com/jandelgado/jled

  // UI: Pixels                          // For use with NeoPixel RGB Smart LEDs
    // uncomment pixel_config.h in "Application Files" section above to use NeoPixel Library

  // UI: Sounds                          // Allows Beep patterns to be set and run autonomously
    #include "EasyBuzzer.h"              // https://github.com/evert-arias/EasyBuzzer
    #include "CuteBuzzerSounds.h"        //  https://github.com/GypsyRobot/CuteBuzzerSounds

  // UI: Command Line Interface          // Provides ability to issue commands to MCU through Serial Monitor
    #include "SimpleCLI.h"               // https://github.com/spacehuhn/SimpleCLI

  // EEPROM                              // Save & retrieve data to storage for recall after power cycle
    #if defined(__AVR__)  // Preprocessor directive to compile based on MCU architecture
      //#include "EEvar.h"               // (AVR) https://github.com/AlexIII/EEvar

// NOTE: may need to delete the FlashStorage lib folder from the sketch/src/ folder to properly compile for AVR.
// It seems like the IDE tries to process all src/ libs for the Architecture even if the lib isn't used?
    #elif defined(SAM)  // Preprocessor directive to compile based on MCU architecture
      #include "FlashStorage.h"          // (SAMD) https://github.com/cmaglie/FlashStorage

    #else
      #error "MCU Core not defined for EEPROM lib use"

    #endif  // END pre-processor directives

  // Application-specific libraries
  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  // see platformio.ini lib_deps

    #include "Adafruit_VS1053.h"        // MusicMaker Featherwing       // https://github.com/adafruit/Adafruit_VS1053_Library

    #include "Adafruit_MotorShield.h"   // Motor Shield V2 Featherwing  // https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library

    // Compile error probably related to original Arduino SD / SDFat.h and Adafruit Forked version
    //#include "SdFat.h"                  // Adafruit Forked Version      // https://github.com/adafruit/SdFat
    //#include "Adafruit_SPIFlash.h"      // Feather M0 Express Onboard 2MB Flash // https://github.com/adafruit/Adafruit_SPIFlash

//==============================================================================
// == Declare & Initialize Global Variables == //
//==============================================================================
// * Ensure a known value for all global variables
// * Operational values will be set in FSM_SYS_INIT_DEV
// * When possible, set to a "warning" value here
//  * this helps detect the case where proper operational values are missed in FSM_SYS_INIT_DEV

// Timestamp: snapshot of the current clock / counter value (uint32_t)
// Alarm: timestamp set in the future, watch for millis() to surpass this value
// Time: duration of an event from start to finish (finsh timestamp - start timestamp)
// Flag: indicator that an event has occured; handle event and reset flag

  // Device vars
  //----------------------------------------------------------------------------
    deviceModeEnum deviceMode = DEVICE_MODE_DEFAULT;    // deviceModeEnum typdef in device_mode_config.h

    int debugLevel = DEBUG_LEVEL_DEFAULT;               // debugLevel is defined in Arduino_DebugUtils.h

  // fsmEnum typedef in fsm_config.h
    fsmEnum fsmStatePrior     = FSM_NOT_SET;                  // store the prior fsm state when fsmStateNext is used (optional)
    fsmEnum fsmStateNow       = FSM_NOT_SET;                  // hold the current fsm state used by switch(case) in loop()
    fsmEnum fsmStateNext      = FSM_NOT_SET;                  // hold the next fsm state (optional)
    fsmEnum fsmStateAfterWait = FSM_NOT_SET;                  // hold the state after next (optional) - useful when going to a "Wait" state in between


  // Loop () vars
  //----------------------------------------------------------------------------
    uint32_t loopCount = 0;                                         // running count of times through the master loop()
    uint32_t loopCompleteTimestamp_us = 0;                          // captures the timestamp of the completion of each loop()
    uint32_t maxLoopTime_us = 0;                                    // record of the longest duration of the master loop()
    uint32_t avgLoopTime_us = 0;                                    // rolling average of the typical loop() duration

    Average<uint32_t> avgLoopTimeSamples_us(LOOP_TIME_NUM_SAMPLES); // store array of loop times for loop statistics / health reporting

  // Session vars
  //----------------------------------------------------------------------------
    bool      sessionIsActive = false;                              // flag/switch to control the status of the session
    uint32_t  sessionStartTimestamp_ms = 0;                         // timestamp of the start of the overall session

    int16_t  sessionCounter = -1;                                   // number of the upcoming session
    int16_t  sessionRepeatsListIndex = -1;                          // index of numSessionsSelected in sessionRepeatsList[] array
    int16_t  sessionRepeatsList[SESSION_REPEATS_NUM_OF_OPTIONS];    // user selectable options for number of consecutive sessions to run

    exitConditionEnum sessionExitCondition = EXIT_NOT_SET;  // exitConditionEnum typdef enum in fsm_config.h

  // Stage vars
  //----------------------------------------------------------------------------
    uint32_t  stageStartTimestamp_ms = 0;                             // timestamp of the start of a cycle (within the session)
    int16_t  stageTimeLeft_sec = -1;

    exitConditionEnum stageExitCondition = EXIT_NOT_SET;    // exitConditionEnum typdef enum in fsm_config.h

  // State-specific vars
  //----------------------------------------------------------------------------
    uint32_t waitStateRefreshTimestamp_ms = WAIT_MESSAGE_REFRESH_INTERVAL_MS;   //forces 1st run
    uint8_t waitStateTickCounter = 0;


  // UI Input vars
  //----------------------------------------------------------------------------

    // Button vars
    //----------------------------------------------------------------------------
      bool      uiButtonPressedFlag = false;
      bool      uiButtonHeldFlag = false;
      bool      uiButtonSequenceFlag = false;

    // Potentiometer
    //----------------------------------------------------------------------------
      //Average<uint16_t> potValSamples(POT_NUM_SAMPLES);

      //uint8_t potValIndex = 0;
      //int16_t potValAvg = -1;

      //bool     potValUpdateFlag = false;

  // UI Output vars
  //----------------------------------------------------------------------------
    bool guiSync = false;

    #ifdef PIXEL_CONFIG_H   // only include if pixel_config.h is being used

      pixelSceneEnum uiPixelScene = PIX_NOT_SET;        // pixelSceneEnum typedef in pixel_config.h

    #endif

  // Sensor Input vars
  //----------------------------------------------------------------------------
    bool pirIsStabilized = false;
    bool pirOutputPinState = false;
    bool pirMotionDetectedFlag = false;
    uint32_t pirMotionTimestamp_ms = 0;

    uint32_t countdownTimestamp_ms = 0;

    bool micEventDetectedFlag = 0;
    uint32_t micEventFanTimer_ms = 0;
    double micThreshold_db = 0.0;
    //uint8_t micEventFanPwm = 0;


  // Actuator Output vars
  //----------------------------------------------------------------------------
    bool fanState = false;        // On / Off
    uint8_t fanSpeedPwm = 0;      // Speed of fan when fanState = On

    bool fanIsRunning = false;    // Flag to read if the fan is currently running
    uint16_t fanTachCountNow = 0;
    uint16_t fanTachCountPrior = 0;
    uint32_t fanCheckIntervalTimestamp_ms = 0;
    uint32_t fanRunningTimestamp_ms = 0;                  // Timestamp for fan start/stop events
    bool fanSpeedUpdateFlag = true;                       // flag to update fan speed once per purification stage
    //uint8_t fanSpeedSetting = FAN_SPEED_SETTING_DEFAULT;  // default set in app_settings.h

    bool uvLedState = false;

    uint32_t occupiedTimeout_sec = 0;
    uint32_t unoccupiedTimeout_sec = 0;
    uint32_t dormantTimeout_sec = 0;

    bool limitSwitch1State = HIGH;    // assume limit switch is "open"
    bool limitSwitch2State = HIGH;    // assume limit switch is "open"

    bool shutterIsOpened = false;
    bool shutterIsClosed = false;

    shutterState shutterDesiredState = STATE_NOT_SET;
    shutterState shutterCurrentState = STATE_NOT_SET;

//==============================================================================
// == Declare Global Objects == //
//==============================================================================

  // SimpleCLI Objects     // https://github.com/spacehuhn/SimpleCLI
  //----------------------------------------------------------------------------

    // SimpleCLI Objects
    SimpleCLI cli;                              // command line interface object

    // list of SimpleCLI Commands
    Command cmdHelp;
    Command cmdSet;
    Command cmdGet;
    //Command cmdSum;
    //Command cmdPressure;

  // EasyButton Objects     // https://easybtn.earias.me/docs/fundamentals
  //----------------------------------------------------------------------------

    // EasyButton buttonName(PIN_NUMBER, debounceTime_ms, pullup_enable, invert);
    //  * uint8_t   PIN_NUMBER
    //  * uint32_t  debounceTime_ms (optional): default 35
    //  * bool      pullup_enable (optional):   default true
    //  * bool      invert (optional):          default true (pressed = LOW ), false (pressed = HIGH)

    EasyButton uiButton(PIN_UI_BUTTON, 100, true, true);

  // EasyLed Objects     // https://evert-arias.github.io/EasyLed/
  //----------------------------------------------------------------------------

    // EasyLed ledName(PIN_NUMBER, polaritySetting);
    // * PIN_NUMBER
    // * polaritySetting (optional): COMMON_NEGATIVE (default), COMMON_POSITIVE

    EasyLed sysLed(PIN_SYS_LED);
    EasyLed uiLed(PIN_UI_LED);

  // EasyBuzzer Objects     // https://github.com/evert-arias/EasyBuzzer/
  //----------------------------------------------------------------------------
    // as of v1.0.4, EasyBuzzer only supports a single object named EasyBuzzer,
    // so object does not need to be declared


  // NeoPixel Object      // https://github.com/adafruit/Adafruit_NeoPixel
  //----------------------------------------------------------------------------
  #ifdef PIXEL_CONFIG_H   // only include if pixel_config.h is being used

    Adafruit_NeoPixel pixels(NUM_PIXELS, PIN_PIXELS, NEO_GRB + NEO_KHZ800);
    // Argument 1 = Number of pixels in NeoPixel strip
    // Argument 2 = Arduino pin number (most are valid)
    // Argument 3 = Pixel type flags, add together as needed:
    //   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
    //   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
    //   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
    //   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
    //   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

  #endif  // PIXEL_CONFIG_H


  // MusicMaker Object
  //----------------------------------------------------------------------------
    Adafruit_VS1053_FilePlayer audioPlayer = Adafruit_VS1053_FilePlayer(PIN_VS1053_RESET,   // reset pin not used!
                                                                        PIN_SPI_VS1053_CS,
                                                                        PIN_VS1053_DSEL,
                                                                        PIN_VS1053_DREQ,
                                                                        PIN_SPI_SD_CS);


  // Motor Shield Object / Stepper
  //----------------------------------------------------------------------------
    Adafruit_MotorShield motorShield = Adafruit_MotorShield(MOTOR_SHIELD1_ADDR);

    // Connect a stepper motor with 200 steps per revolution (1.8 degree)
    // to motor port #2 (M3 and M4)
    Adafruit_StepperMotor *stepper1 = motorShield.getStepper(STEPPER1_STEPS_PER_REV, STEPPER1_PORT);


//==============================================================================
// == App Function Protoypes == //
//==============================================================================
  // Optional in Arduino IDE except for functions that take a reference arument
  // ex. bool functionName (int &referenceArg)

  void cliCheckForInput();
  void cliHelp_cb(cmd*);
  void cliError_cb(cmd_error*);
  void cliSet_cb(cmd*);

  void uiButtonISR();
  void uiButtonPressed_cb();
  void uiButtonHeld_cb();
  void uiButtonSequence_cb();

  void myISR();
  void fanTachISR();

  void printSymbolBreak(char);
  bool printAppInfo();
  bool printDeviceStats();
  bool printDeviceInfo();

  char* getFormattedMillisString(uint32_t);

  bool setFsmStateNext(enum fsmEnum);
  bool setFsmStateAfterWait(enum fsmEnum);
  bool goToFsmStateNext();

  void printProgMemString(const char *);
  char* getProgMemString(const char *);
  template <typename T> void PROGMEM_readAnything(const T *, T&);
  template <typename T> T PROGMEM_getAnything(const T *);

  bool closeShutter();
  bool openShutter();

  void stepClosed();
  void stepOpened();



//==============================================================================
// == Function Definitions == //
//==============================================================================

  // SimpleCLI Callback Functions
  //----------------------------------------------------------------------------

  void cliCheckForInput()
  {
    if (Serial.available())
    {
      String input = Serial.readStringUntil('\n');  // new line char
      //Serial.print(CLI_INPUT_DESIGNATOR + input);                // echo input back out to Serial Monitor for user to see

      cli.parse(input);                             // pare input through SimpleCLI library
    }
  } // END -- cliCheckForInput()



  void cliHelp_cb(cmd* cmdPtr)
  {
    // string to indicate that what follows is output from CLI
    // useful for parsing output stream to PC / GUI program
    Serial.print(CLI_OUTPUT_DESIGNATOR);

    //Serial.println("Help:");
    Serial.println(cli.toString());

  } // END -- cliHelp_cb()



  void cliSet_cb(cmd* cmdPtr)
  {
    //EasyBuzzer.beep(BEEP_CHIRP);    // audible inidicator that command has run

    audioPlayer.stopPlaying();                      // stop any prior audio
    audioPlayer.setVolume(AUDIO_VOLUME, AUDIO_VOLUME);
    audioPlayer.startPlayingFile(COMMAND_RECEIVED_AUDIO);  // Play Start Up Sound

    // string to indicate that what follows is output from CLI
    // useful for parsing output stream to PC / GUI program
    Serial.print(CLI_OUTPUT_DESIGNATOR);

    Command   inputCmd(cmdPtr);          // Create wrapper object for use inside cb
    Argument  inputArg;
    String    inputStr;

    const char defaultStr[] = "default";
    const char invalidStr[] = "INVALID";

    //uint8_t bufferSize = 32;                 // max number of char slots in buffer
    //char buffer[bufferSize];                  // char array of buffSize + 1 for null terminator
    //memset(buffer, 0, sizeof(buffer));        // initialize to null terminator

    // debugLevel Argument
      inputArg = inputCmd.getArgument("debugLevel");   // create local Arg object
      inputStr = inputArg.getValue();                // convert Arg to input String

      // If no value entered ("default"), make no change
      if(inputStr != defaultStr)
      {
        // Handle input value
        //----------------------------------------------------------------------------
        if(          (inputStr == F("n"))
                  //|| (inputStr == F("-1"))
                  //|| (inputStr == F("DBG_NONE"))
          )
          {
            debugLevel = DBG_NONE;
          }
        else if(     (inputStr == F("e"))
                  //|| (inputStr == F("0"))
                  //|| (inputStr == F("DBG_ERROR"))
          )
          {
            debugLevel = DBG_ERROR;
          }
        else if(     (inputStr == F("w"))
                  //|| (inputStr == F("1"))
                  //|| (inputStr == F("DBG_WARNING"))
          )
          {
            debugLevel = DBG_WARNING;
          }
        else if(     (inputStr == F("i"))
                  //|| (inputStr == F("2"))
                  //|| (inputStr == F("DBG_INFO"))
          )
          {
            debugLevel = DBG_INFO;
          }
        else if(     (inputStr == F("d"))
                  //|| (inputStr == F("3"))
                  //|| (inputStr == F("DBG_DEBUG"))
          )
          {
            debugLevel = DBG_DEBUG;
          }
        else if(     (inputStr == F("v"))
                  //|| (inputStr == F("4"))
                  //|| (inputStr == F("DBG_VERBOSE"))
          )
          {
            debugLevel = DBG_VERBOSE;
          }
        else
          {
            //Serial.println(F("debugLevel: INVALID"));
            Serial.println(invalidStr);

          }

          Serial.print(F("[debugLevel: "));
          Serial.print(getProgMemString(debugLevelString[debugLevel+1]));
          Serial.print(F("]"));

          Debug.setDebugLevel(debugLevel);
      }
    // END -- // debugLevel Argument



    // deviceMode Argument
      inputArg = inputCmd.getArgument("deviceMode");   // create local Arg object
      inputStr = inputArg.getValue();                // convert Arg to input String

      // If no value entered ("default"), make no change
      //if(inputStr != "default")
      if(inputStr != defaultStr)
      {
        // Handle input value
        //----------------------------------------------------------------------------
        if(          (inputStr == F("n"))
                  || (inputStr == F("normal"))
                  //|| (inputStr == F("1"))
                  //|| (inputStr == F("NORMAL_MODE"))
          )
          {
            deviceMode = NORMAL_MODE;
          }
        else if(     (inputStr == F("f"))
                  //|| (inputStr == F("first"))
                  //|| (inputStr == F("firstrun"))
                  //|| (inputStr == F("2"))
                  //|| (inputStr == F("FIRST_RUN_MODE"))
          )
          {
            deviceMode = FIRST_RUN_MODE;
          }
        else if(     (inputStr == F("s"))
                  //|| (inputStr == F("setup"))
                  //|| (inputStr == F("3"))
                  //|| (inputStr == F("SETUP_MODE"))
          )
          {
            deviceMode = SETUP_MODE;
          }
        else if(     (inputStr == F("d"))
                  || (inputStr == F("dev"))
                  //|| (inputStr == F("development"))
                  //|| (inputStr == F("4"))
                  //|| (inputStr == F("DEVELOPMENT_MODE"))
          )
          {
            deviceMode = DEVELOPMENT_MODE;
          }
        else if(     (inputStr == F("t"))
                  //|| (inputStr == F("test"))
                  //|| (inputStr == F("5"))
                  //|| (inputStr == F("TEST_MODE"))
          )
          {
            deviceMode = TEST_MODE;
          }
        else if(     (inputStr == F("b"))
                  //|| (inputStr == F("burn"))
                  //|| (inputStr == F("burnin"))
                  //|| (inputStr == F("6"))
                  //|| (inputStr == F("BURN_IN_MODE"))
          )
          {
            deviceMode = BURN_IN_MODE;
          }
        else
          {
            //Serial.println(F("deviceMode: INVALID"));
            Serial.println(invalidStr);

          }

          Serial.print(F("[deviceMode: "));
          Serial.print(getProgMemString(deviceModeString[deviceMode]));
          Serial.print(F("]"));

          setFsmStateNext(FSM_SYS_INIT_DEV);    // warm reboot device
          goToFsmStateNext();

      }
    // END -- // deviceMode Argument


/*
    // fsmStateNow Argument
      inputArg = inputCmd.getArgument("fsmStateNow");   // create local Arg object
      inputStr = inputArg.getValue();                // convert Arg to input String

      // If no value entered ("default"), make no change
      //if(inputStr != "default")
      if(inputStr != defaultStr)
      {
        // Handle input value
        //----------------------------------------------------------------------------
        if(          (inputStr == F("di"))
                  //|| (inputStr == F("devinit"))
                  || (inputStr == F("reboot"))
                  //|| (inputStr == F("FSM_SYS_INIT_DEV"))
          )
          {
            setFsmStateNext(FSM_SYS_INIT_DEV);
          }
        else if(     (inputStr == F("w"))
                //|| (inputStr == F("wait"))
                //|| (inputStr == F("FSM_SYS_WAIT"))
          )
          {
            setFsmStateNext(FSM_SYS_WAIT);
          }
        else if(     (inputStr == F("ai"))
                  //|| (inputStr == F("appinit"))
                  //|| (inputStr == F("FSM_SYS_INIT_APP"))
          )
          {
            setFsmStateNext(FSM_SYS_INIT_APP);
          }
        else if(     (inputStr == F("si"))
                  //|| (inputStr == F("sessinit"))
                  //|| (inputStr == F("FSM_SES_INIT"))
          )
          {
            setFsmStateNext(FSM_SES_INIT);
          }
        else if(     (inputStr == F("s1"))
                  //|| (inputStr == F("stage1"))
                  //|| (inputStr == F("FSM_STG_INIT_STAGE1"))
          )
          {
            setFsmStateNext(FSM_STG_INIT_STAGE1);
          }
        else if(     (inputStr == F("a"))
                  //|| (inputStr == F("alert"))
                  //|| (inputStr == F("FSM_SYS_INIT_ALERT"))
          )
          {
            setFsmStateNext(FSM_SYS_INIT_ALERT);
          }
        else if(     (inputStr == F("e"))
                  //|| (inputStr == F("error"))
                  //|| (inputStr == F("FSM_SYS_INIT_ERROR"))
          )
          {
            setFsmStateNext(FSM_SYS_INIT_ERROR);
          }
        else
          {
            //Serial.println(F("fsmStateNow: INVALID"));
            Serial.println(invalidStr);
          }

          Serial.print(F("[fsmStateNow: "));
          Serial.print(getProgMemString(fsmStateString[fsmStateNext])); // display fsmStateNext since this is what was ACTUALLY set by user
          Serial.print(F("]"));

          goToFsmStateNext();

      }
    // END -- // fsmStateNow Argument
*/

    // uiButton Argument
      inputArg = inputCmd.getArgument("uiButton");   // create local Arg object
      inputStr = inputArg.getValue();                // convert Arg to input String

      // If no value entered ("default"), make no change
      //if(inputStr != "default")
      if(inputStr != defaultStr)
      {
        // Handle input value
        //----------------------------------------------------------------------------
        if(          (inputStr == F("press"))
                  || (inputStr == F("1"))
          )
          {
            uiButtonPressed_cb();
          }
        else if (    (inputStr == F("hold"))
                  || (inputStr == F("2"))
                )
          {
            uiButtonHeld_cb();
          }
        else
          {
            //Serial.print(F("[uiButton: INVALID]"));
            Serial.println(invalidStr);
          }
      }
    // END -- // uiButton Argument



    // fanState Argument
      inputArg = inputCmd.getArgument("fanState");   // create local Arg object
      inputStr = inputArg.getValue();                // convert Arg to input String

      // If no value entered ("default"), make no change
      //if(inputStr != "default")
      if(inputStr != defaultStr)
      {
        //Handle input value
        //----------------------------------------------------------------------------
        fanState = (bool) inputStr.toInt();           // convert to expected data type

        Serial.print(F("[fanState: "));
        Serial.print(fanState);
        Serial.println(F("] "));
      }
    // END -- fanState Arugment



    // fanSpeedPwm Argument
      inputArg = inputCmd.getArgument("fanSpeedPwm");   // create local Arg object
      inputStr = inputArg.getValue();                // convert Arg to input String

      //if(inputStr != "default")
      if(inputStr != defaultStr)
      {
        // Handle input value
        int tempValue = inputStr.toInt();           // convert to expected data type

        if(    (tempValue >= 0)
            && (tempValue <= 255)
          )
          {
            fanSpeedPwm = tempValue;
          }
        else
          {
            //Serial.print(tempValue);
            //Serial.println(F("INVALID"));
            Serial.println(invalidStr);
          }

          Serial.print(F("[fanSpeedPwm: "));
          Serial.print(fanSpeedPwm);
          Serial.println(F("] "));
      }
    // END -- // fanSpeedPwm Argument


    // uvLedState Argument
      inputArg = inputCmd.getArgument("uvLedState");   // create local Arg object
      inputStr = inputArg.getValue();                // convert Arg to input String

      // If no value entered ("default"), make no change
      if(inputStr != defaultStr)
      {
        //Handle input value
        //----------------------------------------------------------------------------
        uvLedState = (bool) inputStr.toInt();           // convert to expected data type

        Serial.print(F("[uvLedState: "));
        Serial.print(uvLedState);
        Serial.println(F("] "));
      }
    // END -- uvLedState Arugment


    // shutterDesiredState Argument
      inputArg = inputCmd.getArgument("shutterDesiredState");   // create local Arg object
      inputStr = inputArg.getValue();                // convert Arg to input String

      // If no value entered ("default"), make no change
      if(inputStr != defaultStr)
      {
        // Handle input value
        //----------------------------------------------------------------------------
        if(          (inputStr == F("c"))
                  || (inputStr == F("1"))
          )
          {
            shutterDesiredState = CLOSED;
          }
        else if(     (inputStr == F("o"))
                  || (inputStr == F("2"))
          )
          {
            shutterDesiredState = OPEN;
          }
        else if(     (inputStr == F("r"))
                  || (inputStr == F("3"))
          )
          {
            shutterDesiredState = RELEASED;
          }
        else
          {
            Serial.println(invalidStr);
          }

          Serial.print(F("[shutterDesiredState: "));
          Serial.print(getProgMemString(shutterStateString[shutterDesiredState]));
          Serial.println(F("]"));

      }
    // END -- shutterDesiredState Argument



    // occupiedTimeout Argument
      inputArg = inputCmd.getArgument("occupiedTimeout");   // create local Arg object
      inputStr = inputArg.getValue();                // convert Arg to input String

      if(inputStr != defaultStr)
      {
        // Handle input value
        int tempValue = inputStr.toInt();           // convert to expected data type


        if(    (tempValue >= 0)                     // timeout value must be non-negative
            && (tempValue <= 2147483647)           // int max
          )
          {
            occupiedTimeout_sec = tempValue;
          }
        else
          {
            Serial.println(invalidStr);
          }

          Serial.print(F("[occupiedTimeout: "));
          Serial.print(occupiedTimeout_sec);
          Serial.println(F(" sec] "));
      }
    // END -- // occupiedTimeout Argument


    // unoccupiedTimeout Argument
      inputArg = inputCmd.getArgument("unoccupiedTimeout");   // create local Arg object
      inputStr = inputArg.getValue();                // convert Arg to input String

      if(inputStr != defaultStr)
      {
        // Handle input value
        int tempValue = inputStr.toInt();           // convert to expected data type


        if(     (tempValue >= 0)                     // timeout value must be non-negative
            && (tempValue <= 2147483647)            // int max
          )
          {
            unoccupiedTimeout_sec = tempValue;
          }
        else
          {
            Serial.println(invalidStr);
          }

          Serial.print(F("[unoccupiedTimeout: "));
          Serial.print(unoccupiedTimeout_sec);
          Serial.println(F(" sec] "));
      }
    // END -- // unoccupiedTimeout Argument


    // dormantTimeout Argument
      inputArg = inputCmd.getArgument("dormantTimeout");   // create local Arg object
      inputStr = inputArg.getValue();                // convert Arg to input String

      if(inputStr != defaultStr)
      {
        // Handle input value
        int tempValue = inputStr.toInt();           // convert to expected data type

        if(    (tempValue >= 0)                     // timeout value must be non-negative
            && (tempValue <= 2147483647)          // int max
          )
          {
            dormantTimeout_sec = tempValue;
          }
        else
          {
            Serial.println(invalidStr);
          }

          Serial.print(F("[dormantTimeout: "));
          Serial.print(dormantTimeout_sec);
          Serial.println(F(" sec] "));
      }
    // END -- // dormantTimeout Argument



    // micThreshold Argument
      inputArg = inputCmd.getArgument("micThreshold");   // create local Arg object
      inputStr = inputArg.getValue();                // convert Arg to input String

      //if(inputStr != "default")
      if(inputStr != defaultStr)
      {
        // Handle input value
        int tempValue = inputStr.toInt();           // convert to expected data type

        if(    (tempValue >= 35)
            && (tempValue <= 150)
          )
          {
            micThreshold_db = tempValue;
          }
        else
          {
            //Serial.print(tempValue);
            //Serial.println(F("INVALID"));
            Serial.println(invalidStr);
          }

          Serial.print(F("[micThreshold: "));
          Serial.print(micThreshold_db);
          Serial.println(F(" dB] "));
      }
    // END -- // micThreshold Argument


  } // END -- cliSet_cb()


/*
  void cliExample_cb(cmd* cmdPtr)
  {
    // string to indicate that what follows is output from CLI
    // useful for parsing output stream to PC / GUI program
    Serial.print(CLI_OUTPUT_DESIGNATOR);

    Command   inputCmd(cmdPtr);          // Create wrapper object for use inside cb
    //Argument  inputArg;
    //String    inputStr;

    // Integer Input Argument
      Argument argNum = inputCmd.getArgument("num");   // create local Arg object
      String   strNum = argNum.getValue();        // convert Arg to input String
      int num         = strNum.toInt();           // convert to expected data type

    // String Input Argument
      Argument argMode = inputCmd.getArgument("mode"); // create local Arg object
      String   valMode = argMode.getValue();       // convert Arg to input String

    // Flag Input Argument
      Argument argEcho = inputCmd.getArgument("echo");       // create local Arg object
      bool     echo    = argEcho.isSet();               // convert to bool

    // Do on Flag Set
      if (echo) valMode.toUpperCase();

      for (int i = 0; i < num; i++)
      {
          Serial.println(valMode);
      }
  } // END -- cliExample_cb()
*/

  // Example for BoundlessCommand
  // Callback function for sum command
/*
  void cliSum_cb(cmd* cmdPtr)
  {
    // string to indicate that what follows is output from CLI
    // useful for parsing output stream to PC / GUI program
    Serial.print(CLI_OUTPUT_DESIGNATOR);

    Command inputCmd(cmdPtr);          // Create wrapper object for use inside cb

    int argNum = inputCmd.countArgs(); // Get number of arguments from BoundlessCmd
    int sum    = 0;

    // Go through all arguments (BoundlessCmd)
    for (int i = 0; i<argNum; i++)
    {
        Argument arg      = inputCmd.getArg(i);      // make input string to cli an Argument object
        String   argValue = arg.getValue();     // turn into String for conversion
        int argIntValue   = argValue.toInt();   // decode into expected data type

        sum += argIntValue;                     // perform function task
    }

    // Print result
    Serial.print(sum);                          // cli output result

  } // END -- cliSum_cb()
*/

  void cliError_cb(cmd_error* errorPtr)
  {
    // string to indicate that what follows is output from CLI
    // useful for parsing output stream to PC / GUI program
    //Serial.print(CLI_OUTPUT_DESIGNATOR);

    CommandError e(errorPtr);

    Serial.println("\n\r\n\rERROR: " + e.toString() + "\n\r");

    if (e.hasCommand())
    {
      //Serial.print(CLI_OUTPUT_DESIGNATOR);
      Serial.println("Cmd Syntax: " + e.getCommand().toString());
    }
    else
    {
      //Serial.print(CLI_OUTPUT_DESIGNATOR);
      Serial.println(cli.toString());
    }
  } // END -- cliError_cb()


/*
  void cliPressure_cb(cmd* cmdPtr)
  {
    //EasyBuzzer.beep(BEEP_CHIRP);    // audible inidicator that command has run
    //uiLed.blink(LED_ACTIVITY);       // set print led scene
    uiLed.offUntil(100, uiLedOnAgain);  // flash LED OFF for 100 ms, then turn back on


    // string to indicate that what follows is output from CLI
    // useful for parsing output stream to PC / GUI program
    //Serial.print(CLI_OUTPUT_DESIGNATOR);

    Command   inputCmd(cmdPtr);          // Create wrapper object for use inside cb
    Argument  inputArg;
    String    inputStr;

    const char defaultStr[] = "default";
    const char invalidStr[] = "INVALID";

    //uint8_t bufferSize = 32;                 // max number of char slots in buffer
    //char buffer[bufferSize];                  // char array of buffSize + 1 for null terminator
    //memset(buffer, 0, sizeof(buffer));        // initialize to null terminator


    // Zero Flag Argument
    //----------------------------------------------------------------------------
    //  * check for Zero Offset Flag first so that zeroing happens before pressure output

      inputArg = inputCmd.getArgument("zero");   // create local Arg object
      //inputStr = inputArg.getValue();                // convert Arg to input String
      bool zeroFlag = inputArg.isSet();

      if(zeroFlag)
      {
        setPressureZeroOffset();
        // Printing moved to inside setPressureZeroOffset()
        //Serial.print("ZEROED");
        //Serial.print("\r\n");             //default End of Line sequence for Bonsai
        return; // end function without printng out pressure reading
      }
    // END -- Zero Offset Argument


    // Unit Argument
    //----------------------------------------------------------------------------
      inputArg = inputCmd.getArgument("unit");   // create local Arg object
      inputStr = inputArg.getValue();                // convert Arg to input String


      // take pressure reading in kPa based on datasheet transfer function
      // subtract Zero Offset Value (if set)
      float pressureOffset_kPa = getPressure_kPa();
      float pressureOutput = 0.0;             // output value after unit conversion and Zero-Offset

      // Make sure the pressure reading is within the valid range before offset
      if(pressureOffset_kPa >= (PRESSURE_MAX_VALUE_KPA - PRESSURE_MAX_ERROR_KPA) )
      {
        Serial.print("MAXED\r\n");    // warn if out of valid range
        return;
      }
      else                            // else apply offset value
      {
        pressureOffset_kPa = pressureOffset_kPa - pressureZeroOffset_kPa;
      }


      // If no value entered ("default"), make no change
      if(inputStr != defaultStr)
      {
        // Handle input value
        //----------------------------------------------------------------------------
        if(          (inputStr == F("k"))         // kPa
                  //|| (inputStr == F("-1"))
          )
          {
            //pressureUnit = 'k';

            pressureOutput = pressureOffset_kPa;        // convert units
            Serial.print(pressureOutput);
            Serial.print(" kPa");

          }
        else if(     (inputStr == F("p"))         // Pa
                  //|| (inputStr == F("0"))
          )
          {
            //pressureUnit = 'p';

            pressureOutput = pressureOffset_kPa * 1000.0;  // convert units
            Serial.print(pressureOutput);
            Serial.print(" Pa");

          }
        else if(     (inputStr == F("c"))         // cmH2O
                  //|| (inputStr == F("1"))
          )
          {
            //pressureUnit = 'c';

            pressureOutput = pressureOffset_kPa * 10.1972;  // convert units
            Serial.print(pressureOutput);
            Serial.print(" cmH2O");

          }
        else if(     (inputStr == F("m"))         // mmH2O
                  //|| (inputStr == F("2"))
          )
          {
            //pressureUnit = 'm';

            pressureOutput = pressureOffset_kPa * 101.9716;  // convert units
            Serial.print(pressureOutput);
            Serial.print(" mmH2O");
          }
        else if(     (inputStr == F("i"))         // inH2O
                  //|| (inputStr == F("3"))
          )
          {
            //pressureUnit = 'i';

            pressureOutput = pressureOffset_kPa * 4.01865;  // convert units
            Serial.print(pressureOutput);
            Serial.print(" inH2O");
          }
        else
          {
            //Serial.println(F("debugLevel: INVALID"));
            Serial.println(invalidStr);
          }

          Serial.print("\r\n");             //default End of Line sequence for Bonsai

      } // END if argument was provided

      else  // else if no argument was provided, return default unit of cmH20
      {
        pressureOutput = pressureOffset_kPa * 10.1972;  // convert units
        Serial.print(pressureOutput);
        Serial.print(" cmH2O");
        Serial.print("\r\n");             //default End of Line sequence for Bonsai
      }
    // END -- Unit Argument


  } // END -- cliPressure_cb()
*/


  // END SimpleCLI Callback Functions
  //----------------------------------------------------------------------------


  // EasyButton Callback Functions
  //----------------------------------------------------------------------------
    void uiButtonISR()           // Interrupt driven button response, redirects to callback functions below
    {
      DEBUG_PRINT_FUNCTION_LINE;    // inline debug print to show functionName() & Line#

      uiButton.read();
    }


    void uiButtonPressed_cb()
    {
      DEBUG_PRINT_FUNCTION_LINE;    // inline debug print to show functionName() & Line#

      // Register button action here via flag
      // Handle button action and reset flag in main loop() > Handle Inputs
      uiButtonPressedFlag = true;

    } // END uiButtonPressed_cb()


    void uiButtonHeld_cb()
    {
      DEBUG_PRINT_FUNCTION_LINE;    // inline debug print to show functionName() & Line#

      // Register button action here via flag
      // Handle button action and reset flag in main loop() > Handle Inputs
      uiButtonHeldFlag = true;

    } // END uiButtonHeld_cb()


    void uiButtonSequence_cb()
    {
      DEBUG_PRINT_FUNCTION_LINE;    // inline debug print to show functionName() & Line#

      // Register button action here via flag
      // Handle button action and reset flag in main loop() > Handle Inputs
      uiButtonSequenceFlag = true;
    } // END uiButtonSequence_cb()


  // END EasyButton Callback Functions
  //----------------------------------------------------------------------------


  // Custom Interrupt Service Routines
  //----------------------------------------------------------------------------
    void myISR()
    {
      // * Capture ISR events here immediately
      // * Handle the ISR event in the main loop() and clear the event flag
      //  * this helps ISR events in an orderly fashion and prevents rapid fire changes in ISR
      // isrFlag = true;

    } // END myISR()


    void fanTachISR()
    {

      fanTachCountNow++;

      //Debug.print(DBG_DEBUG, F("[D] fan ISR"));

    } // END fanTachISR()


  // Print Helper Functions
  //----------------------------------------------------------------------------
  // * Conserve RAM by storing string symbols in PROGMEM
  //  * print using getProgMemString() function
  //  * Easier to make changes to format of reused print statements in a single location

    void printSymbolBreak(char c)
    {

      for(int i=0; i<40; i++)
      {
        Serial.print(c);
      }
      Serial.println();

    } // END printSymbolBreak()


    bool printAppInfo()
    {
      uint8_t bufferSize = 128;                 // max number of char slots in buffer
      char buffer[bufferSize];                  // char array of buffSize + 1 for null terminator

      // http://www.cplusplus.com/reference/cstdio/snprintf/
      snprintf(buffer, sizeof(buffer),           // "prints" formatted output to a char array (string)
                "\n\r"
                "App:\t\t%s %s\n\r"
                "Compiled:\t%s %s\n\r"
                ,
                FIRMWARE_NAME,
                FIRMWARE_VERSION,
                __DATE__,
                __TIME__
            );

      Serial.print(buffer);
      return true;

    } // END -- printAppInfo()


    bool printDeviceStats()
    {
      uint8_t bufferSize = 128;                 // max number of char slots in buffer
      char buffer[bufferSize];                  // char array of buffSize + 1 for null terminator

      // http://www.cplusplus.com/reference/cstdio/snprintf/
      snprintf(buffer, sizeof(buffer),           // "prints" formatted output to a char array (string)
                //"\n\r"
                "Debug:\t\t%s\n\r"
                "Free RAM:\t%d bytes\n\r"
                ,
                getProgMemString(debugLevelString[debugLevel+1]),
                //availableRam()  // AVR only
                freeMemory()
              );

      Serial.print(buffer);
      return true;

    } // END -- printDeviceStats()


    bool printDeviceInfo()
    {
      uint8_t bufferSize = 128;                 // max number of char slots in buffer
      char buffer[bufferSize];                  // char array of buffSize + 1 for null terminator

      // http://www.cplusplus.com/reference/cstdio/snprintf/
      snprintf(buffer, sizeof(buffer),           // "prints" formatted output to a char array (string)
                //"\n\r"
                "Device Mode:\t%s\n\r"
                "Dev SN:\t\t%s\n\r"
                "MFR ID:\t\t\n\r"                   // print last to utilize UniqueIDdump(Serial) function
                ,
                getProgMemString(deviceModeString[deviceMode]),
                "YYWW.###"                    // 8-char serial number format based on production date YYWW
              );

      Serial.print(buffer);
      UniqueIDdump(Serial);                   // print Mfg ID using ArduinoUniqueID library
      Serial.println();
      return true;

    } // END -- printDeviceInfo()



    // return a millisecond value as a string formatted as "HH:MM:SS.mmm"
    char  formattedMillisBuffer[17];                    // global char buffer for the printing of milliseconds in formatted time
    char* getFormattedMillisString(uint32_t msCount)
    {
      uint16_t milliseconds  = msCount % 1000;          // ms remaining when converted to seconds
      uint16_t allSeconds   = msCount / 1000;           // total number of seconds to calculate remaining values

      uint16_t hours            = allSeconds / 3600;    // convert seconds to hours
      uint16_t secondsRemaining = allSeconds % 3600;    // seconds left over

      uint16_t minutes  = secondsRemaining / 60 ;       // convert seconds left over to minutes
      uint16_t seconds  = secondsRemaining % 60;        // seconds left over

      snprintf(formattedMillisBuffer, sizeof(formattedMillisBuffer),           // "prints" formatted output to a char array (string)
                "%02d:"   //HH:
                "%02d:"   //MM:
                "%02d."   //SS.
                "%03d"    //MMM
                ,
                hours,
                minutes,
                seconds,
                milliseconds
            );

      return formattedMillisBuffer;

    } // END -- getFormattedMillisString()


  // Other Custom Functions
  //----------------------------------------------------------------------------

    bool setFsmStateNext(enum fsmEnum nextState)
    {
      // Test for valid fsmStateNext
      if(     (nextState > FSM_NOT_SET)
          &&  (nextState < FSM_NUM_STATES)
        )
      {
        fsmStateNext = nextState;

        return true;
      }
      else  // fsmStateNext is out of bounds / not defined
      {
        fsmStateNext = FSM_SYS_INIT_ERROR;

        Debug.print(DBG_ERROR,    F("[E] * ERROR @ %s"), getProgMemString(fsmStateString[fsmStateNow]));

        if(nextState == FSM_NOT_SET)
        {
          sessionExitCondition = EXIT_FSM_NOT_SET;
          Debug.print(DBG_ERROR,    F("[E]   * setFsmStateNext(%s)"), getProgMemString(fsmStateString[nextState]) );
        }
        else
        {
          sessionExitCondition = EXIT_FSM_INVALID;
          Debug.print(DBG_ERROR,    F("[E]   * setFsmStateNext(%d) invalid"), nextState);
        }

        Debug.print(DBG_ERROR,      F("[E]   * go to %s"), getProgMemString(fsmStateString[fsmStateNext]));

        return false;
      }

    } // END -- setFsmStateNext()


    bool setFsmStateAfterWait(enum fsmEnum stateAfterWait)
    {
      // Test for valid fsmStateNext
      if(     (stateAfterWait > FSM_NOT_SET)
          &&  (stateAfterWait < FSM_NUM_STATES)
        )
      {
        fsmStateAfterWait = stateAfterWait;

        return true;
      }
      else    // fsmStateAfterWait is out of bounds / not defined
      {
        fsmStateNext = FSM_SYS_INIT_ERROR;

        Debug.print(DBG_WARNING,    F("[W] * WARNING @ %s"), getProgMemString(fsmStateString[fsmStateNow]));

        if(stateAfterWait == FSM_NOT_SET)
        {
          //sessionExitCondition = EXIT_FSM_NOT_SET;
          Debug.print(DBG_WARNING,    F("[W]   * setFsmStateNext(%s)"), getProgMemString(fsmStateString[stateAfterWait]) );
        }
        else
        {
          //sessionExitCondition = EXIT_FSM_INVALID;
          Debug.print(DBG_WARNING,    F("[W]   * setFsmStateNext(%d) invalid"), stateAfterWait);
        }

        Debug.print(DBG_WARNING,      F("[W]   * go to %s"), getProgMemString(fsmStateString[fsmStateNext]));

        return false;
      }

    } // END -- setFsmStateAfterWait()


    bool goToFsmStateNext()
    {
      // Shift all states down the line...
      fsmStatePrior = fsmStateNow;
      fsmStateNow = fsmStateNext;
      fsmStateNext = fsmStateAfterWait;
      fsmStateAfterWait = FSM_NOT_SET;

      bool newLine = false;   // var to track if a new line is printed at start of print based on debug level

      // use Serial.print instead of Debug.print() to ignore the state of Debug.timestampXX()
      if(debugLevel >= DBG_DEBUG)
      {

        newLine = true;
        Serial.println();

        Serial.print(F("\n\r["));
        Serial.print(getFormattedMillisString(millis()));
        Serial.print(F("] "));

      }

      // use Serial.print instead of Debug.print() to ignore the state of Debug.timestampXX()
      if(debugLevel >= DBG_DEBUG)
      {
        if(!newLine){Serial.println();}

        Serial.println(getProgMemString(fsmStateString[fsmStateNow]));
        printSymbolBreak('-');
      }

      return true;
    } // END -- goToFsmStateNext()



    // PROGMEM Functions
    //----------------------------------------------------------------------------

    // Print a string from PROGMEM directly to save RAM
    void printProgMemString(const char * progMemString)
    {
      char c;
      if (!progMemString)
        return;
      while ((c = pgm_read_byte(progMemString++)))
        Serial.print (c);

    } // END -- printProgMemString()



    char progMemStringBuffer [ENUM_STRING_BUFFER_SIZE];      // global char buffer for getProgMemString()

    char* getProgMemString(const char * progMemString)      // Return a string from PROGMEM directly, use with Serial.print() or sprintf()
    {
      // PROGMEM is handled differently by different MCU cores (platforms, architectures, etc)
      // We can direct the IDE to compile only th code relevant to the MCU core in use.
      // This allows us to use the same code base for a variety of MCUs with no wasted compiled code size
      // https://www.deviceplus.com/arduino/arduino-preprocessor-directives-tutorial/

      // Preprocessor directive to compile based on MCU architecture
      #if defined(__AVR__)

        // Retrieve strings stored in PROGMEM
        snprintf_P (  progMemStringBuffer,
                      sizeof(progMemStringBuffer),      // limits input strings from PROGMEM to size of buffer
                      PSTR ("%S"),                      // use %S (not %s) for PROGMEM string flag
                      progMemString
                    );

        return progMemStringBuffer;

        // END __AVR__

      // Preprocessor directive to compile based on MCU architecture
      #elif defined(SAM)

        // SAM MCUs don't separate Flash and RAM, so "PROGMEM" is not employed
        snprintf_P (  progMemStringBuffer,
                      sizeof(progMemStringBuffer),      // limits input strings from PROGMEM to size of buffer
                      PSTR ("%s"),                      // HERE: use %s (not %S) for non-PROGMEM strings
                      progMemString
                    );

        return progMemStringBuffer;
        // END SAM

      #else   // Handle other cases

        #error "MCU Core not defined in getProgMemString()"

      #endif  // END pre-processor directives

    } // END -- getProgMemString()


    // Assigns value from entry in PROGMEM table[] to newVar, e.g....
    //  * float newVar;
    //  * PROGMEM_readAnything(&table[i], newVar);
    //  * Serial.println(newVar);
    template <typename T> void PROGMEM_readAnything(const T * sce, T& dest)
    {
      memcpy_P (&dest, sce, sizeof (T));

    } // END -- PROGMEM_readAnything()


    // Directly returns value in entry of PROGMEM table[]. e.g...
    //   * Serial.println(PROGMEM_getAnything(&table[i]));
    template <typename T> T PROGMEM_getAnything(const T * sce)
    {
      static T temp;
      memcpy_P (&temp, sce, sizeof (T));
      return temp;
    } // END -- PROGMEM_getAnything()


    bool closeShutter()
    {
      while(digitalRead(PIN_LIMIT_SWITCH1) != LOW)
      {
        stepper1->onestep(FORWARD, DOUBLE);
      }

      return true;
    }


    bool openShutter()
    {
      while(digitalRead(PIN_LIMIT_SWITCH2) != LOW)
      {
        stepper1->onestep(BACKWARD, DOUBLE);
      }

      return true;
    }

    void stepClosed()
    {
      stepper1->onestep(FORWARD, DOUBLE);
    }

    void stepOpened()
    {
      stepper1->onestep(BACKWARD, DOUBLE);
    }


//=========================================================================//
// == Setup == //
//=========================================================================//

  void setup()
  {

    // I/O Pin Configuration
    //----------------------------------------------------------------------------
    // Options: OUTPUT, INPUT, INPUT_PULLUP

    // Adafruit Feather M0 Express Pinout: https://learn.adafruit.com/assets/96531
    //----------------------------------------------------------------------------
      //I2C_SCL                                 // 27
      //I2C_SDA                                 // 26

      //SPI_SCK                                 // 24
      //SPI_MOSI                                // 23
      //SPI_MISO                                // 22

      pinMode(PIN_UV_LED_RELAY, OUTPUT);        // A5
      pinMode(PIN_UI_LED, OUTPUT);              // A4
      pinMode(PIN_MIC, INPUT);                  // A3
      pinMode(PIN_LIMIT_SWITCH2, INPUT_PULLUP); // A2
      pinMode(PIN_LIMIT_SWITCH1, INPUT_PULLUP); // A1
      pinMode(PIN_PIR_SENSOR, INPUT);           // A0

      pinMode(PIN_SYS_LED, OUTPUT);             // 13
      pinMode(PIN_FAN_TACH, INPUT_PULLUP);      // 12
      pinMode(PIN_FAN_PWM, OUTPUT);             // 11
      //VS1053_DSEL                             // 10
      //VS1053_DREQ                             //  9
      //SPI_VS1053_CS                           //  6
      //SPI_SD_CS                               //  5

      pinMode(PIN_UI_BUTTON, INPUT);            //  1 // HW Pull-up; pull up to prevent known audio hang bug https://learn.adafruit.com/adafruit-music-maker-featherwing/troubleshooting

    // Initialize Output Pins
    //----------------------------------------------------------------------------
      digitalWrite(PIN_UV_LED_RELAY, LOW);
      digitalWrite(PIN_UI_LED, LOW);
      digitalWrite(PIN_SYS_LED, LOW);
      //digitalWrite(PIN_BUZZER, LOW);
      digitalWrite(PIN_FAN_PWM, LOW);


    // Initialize Analog Input Range
    //----------------------------------------------------------------------------
    // https://www.arduino.cc/reference/en/language/functions/analog-io/analogreference/
    // Configures the reference voltage used for analog input (i.e. the value used as the top of the input range)
    // * DEFAULT: the default analog reference of 5 volts (on 5V Arduino boards) or 3.3 volts (on 3.3V Arduino boards)
    // * EXTERNAL: the voltage applied to the AREF pin (0 to 5V only) is used as the reference.

      // analogReference(EXTERNAL);       // Command used for AVR-based boards (e.g. Uno, Nano, Feather 32u4)
      // analogReference(AR_EXTERNAL);   // Command used for SAMD-based boards (e.g. Feather M0)


    //=========================================================================//
    // == Object Setup == //
    //=========================================================================//

      // Arduino Standard Objects Setup
      //----------------------------------------------------------------------------
        //Wire.begin();                               // Start I2C
        //SPI.begin                                   // Start SPI
        Serial.begin(SERIAL_BAUD_RATE);               // Sets the Serial communication data rate

      // Debug Setup
      //----------------------------------------------------------------------------
        // * Example use: Debug.print(DBG_WARNING,    F("[E] * ERROR from %s"), getProgMemString(fsmStateString[fsmStateNow]) );

        Debug.setDebugLevel(debugLevel);           // initialized to DEBUG_LEVEL_DEFAULT from app_config.h
        //Debug.timestampOn();
        //Debug.setDebugOutputStream(&softSerial);          // to send output to other Serial stream (e.g. SoftwareSerial)


      // NeoPixel Setup
      //----------------------------------------------------------------------------
      #ifdef PIXEL_CONFIG_H   // only include if pixel_config.h is being used

        pixels.begin();
        pixels.clear();                             // Set all pixels to OFF
        pixels.show();                              // refresh pixels

      #endif  // PIXEL_CONFIG_H

      // EasyButton Setup
      //----------------------------------------------------------------------------

        // uiButton Setup
          uiButton.begin();
          uiButton.onPressed(uiButtonPressed_cb);                                             // single press (callback function)
          uiButton.onPressedFor(BTN_HOLD_DURATION_MS, uiButtonHeld_cb);                       // pressed and held for BTN_DURATION_MS (callback function)
          uiButton.onSequence(BTN_SEQ_NUM_CLICKS, BTN_SEQ_TIMEOUT_MS, uiButtonSequence_cb);   // button pressed BTN_SEQ_NUM_CLICKS times within BTN_SEQ_TIMEOUT_MS (callback function)
          //uiButton.enableInterrupt(uiButtonISR);    // enable button Interrupt here

        // END uiButton setup

        // ... add more Button setups here

      // -- END EasyButton Setup

      // EasyLed Setup
      //----------------------------------------------------------------------------
        sysLed.off();
        sysLed.update();

        uiLed.off();
        uiLed.update();

      // -- END EasyLed Setup

     // EasyBuzzer Setup
     //----------------------------------------------------------------------------
        //EasyBuzzer.setPin(PIN_BUZZER);    // this would normally go in Object Declaration, but EasyBuzzer only supports 1 instance, so there's no declaration
        //EasyBuzzer.stopBeep();
      // -- END EasyBuzzer Setup


      // SimpleCLI Command Setup
      //----------------------------------------------------------------------------
        // addPositionalArgument, addPosArg:
        // * User has the option to enter by position (order, separated by space) or by name (e.g. "-str")
        // * Position is determined by order added to command below

        // addArgument, addArg:
        // * adds "named" argument to command. User MUST specify name in command
        // * can be entered in any order in command line by user

        // addFlagArgument, addFlagArg:
        // * Flag set by user entering argument name only
        // * Take no value
        // * Use for response preferences, NOT application bools - cannot be UNset in command

        // Default values
        // * always defined as a string (user input will be string) for all data types
        // * data type is "decoded" in callback function

        // Assign cb function to handle errors
        cli.setOnError(cliError_cb);

        // HELP: display "help" info to user
        cmdHelp = cli.addCommand("h/elp", cliHelp_cb);
        cmdHelp.setDescription("* Show command list");

        // SET: master cmd to set app variables
        cmdSet = cli.addCommand("s/et", cliSet_cb);   // ("cmdName, cmdNameAlias", cmdCallbackFunction); cmdName should be alpha-numeric chars ONLY
        cmdSet.setDescription("* Set variable values");
        cmdSet.addPositionalArgument("debug/Level,dl", "default");     // ("argName, argAlias", "defaultValue");
        cmdSet.addPositionalArgument("device/Mode,dm", "default");     // ("argName, argAlias", "defaultValue");
        //cmdSet.addPositionalArgument("fsm/State/Now", "default");     // ("argName, argAlias", "defaultValue");
        cmdSet.addPositionalArgument("uiButton,uiBtn,b/tn", "default");     // ("argName, argAlias", "defaultValue");

        cmdSet.addPositionalArgument("fan/State", "default");               // ("argName, argAlias", "defaultValue");
        cmdSet.addPositionalArgument("fanSpeedPwm,fanPwm,p/wm", "default");     // ("argName, argAlias", "defaultValue");

        cmdSet.addPositionalArgument("uv/Led/State", "default");

        cmdSet.addPositionalArgument("s/hutter/Desired/State", "default");

        cmdSet.addPositionalArgument("o/ccupied/Timeout", "default");
        cmdSet.addPositionalArgument("u/noccupied/Timeout", "default");
        cmdSet.addPositionalArgument("d/ormant/Timeout", "default");

        cmdSet.addPositionalArgument("m/ic/Threshold", "default");


        // PRESSURE: output pressure sensor reading
        //cmdPressure = cli.addCommand("p/ress/ure", cliPressure_cb);   // ("cmdName, cmdNameAlias", cmdCallbackFunction); cmdName should be alpha-numeric chars ONLY
        //cmdPressure.setDescription("* Pressure reading, set units, or zero-out");
        //cmdPressure.addPositionalArgument("u/nit", "default");     // ("argName, argAlias", "defaultValue");
        //cmdPressure.addFlagArgument("z/ero");                // ("flagName", default = Not Set)

        //cmdDebug = cli.addCommand("debug", cliDebug_cb);   // ("cmdName, cmdNameAlias", cmdCallbackFunction); cmdName should be alpha-numeric chars ONLY
        //cmdDebug.addPositionalArgument("m/ode", "d");     // ("argName", "defaultValue");
        //cmdDebug.addArgument("n/um/ber", "1");            // ("argName", "defaultValue"); ArgName with / makes everything after "/" optional
        //cmdDebug.addFlagArgument("e/cho");                // ("flagName", default = Not Set)

        // addBoundlessCommand, addBoundlessCmd:
        // * can accept any number of user input VALUES (callback must know how to handle these values)
        // * canNOT use .addArgument for Positional or Named Arguments
        // * useful for inputs of unknown or user decided format, iike strings with spaces (names, address, passphrase)
        //cmdSum = cli.addBoundlessCommand("sum", cliSum_cb);  // ("cmdName, cmdNameAlias", cmdCallbackFunction);


      // MusicMaker Setup
      //----------------------------------------------------------------------------
        if(!audioPlayer.begin())        // initialize the audio IC (VS1053)
        {
          Debug.print(DBG_WARNING, F("[W] * Audio Chip \t NOT FOUND"));
        }

        if (!SD.begin(PIN_SPI_SD_CS))   // initialize the audio SD Card
        {
          Debug.print(DBG_WARNING, F("[W] * Audio SD Card \t NOT FOUND"));
        }

        audioPlayer.reset();


        // Set volume for left, right channels from 1 to 10. lower numbers == louder volume!
        audioPlayer.setVolume(AUDIO_VOLUME, AUDIO_VOLUME);   // max volume

        // If DREQ is on an interrupt pin we can use audioPlayer.startPlayingFile()
        // to play audio in background while MCU does other tasks
        audioPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int



      // Stepper Setup
      //----------------------------------------------------------------------------
        motorShield.begin();        // create with the default frequency 1.6KHz
        //motorShield.begin(1000);  // OR with a different frequency, say 1KHz

        stepper1->setSpeed(STEPPER1_RPM_DEFAULT);     // RPM

    // == END Object Setup


    //=========================================================================//
    // == Setup() Exit Actions == //
    //=========================================================================//

    // ** Consider if actions should be moved to FSM_SYS_INIT_DEV ("warm reboot")

      // Enable Watchdog Timer
      //----------------------------------------------------------------------------
        Watchdog.enable(WATCHDOG_TIMER_MS);   // turn on watchdog timer just before entering loop, reset watchdog each loop iteraton
        //Watchdog.disable();


      // Initialize PotVal Smoothing Array
      //----------------------------------------------------------------------------
        //for(int i = 0; i < POT_NUM_SAMPLES; i++)
        //{
        //  potValSamples.push(analogRead(PIN_POT));   // fill Average object
        //}


      // Attach Interrupts
      //----------------------------------------------------------------------------
      // https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/

        // enable EasyButton Interrupts in EasyButton Setup with uiButton.enableInterrupt(uiButtonISR);

        attachInterrupt(digitalPinToInterrupt(PIN_FAN_TACH), fanTachISR, FALLING);


      // Set up transition to next fsmState
      //----------------------------------------------------------------------------
        setFsmStateNext(FSM_SYS_INIT_DEV);

        Debug.setDebugLevel(DBG_NONE); // Turn off debug print statememt for this one instance while leaving setup()
        goToFsmStateNext();
        Debug.setDebugLevel(debugLevel); // reset debugLevel. Runtime debugLevel will be set by deviceMode in FSM_SYS_INIT_DEV

    // == END Setup() Exit Actions


  } // == END setup()

//==============================================================================
// == Loop == //
//==============================================================================

  void loop()
  {
    //----------------------------------------------------------------------------
    // -- Declare & Update Loop() Variables -- //
    //----------------------------------------------------------------------------
      // loopTime is calculated at end of loop()
      loopCount++;                       // increment loop count

      // END -- Declare & Update Loop() Variables

    //----------------------------------------------------------------------------
    // -- Service Watchdog Timer -- //
    //----------------------------------------------------------------------------
      Watchdog.reset();     // reset watchdog timer every loop


    //----------------------------------------------------------------------------
    // -- Handle Inputs (Buttons, Serial, Sensor statuses, etc.) -- //
    //----------------------------------------------------------------------------

      // Handle Polled Inputs
      //----------------------------------------------------------------------------

        // Check for User Input to Command Line Interface
        //----------------------------------------------------------------------------
          cliCheckForInput();      // read and parse full line of CLI input


        // Update EasyButton polled button states
        //----------------------------------------------------------------------------
          //uiButton.update();  // EasyButton: update() function must be called repeatedly only if onPressedFor functionality is being used and interrupt is enabled
          uiButton.read(); // EasyButton: read() checks button for non-interrupt enabled pins at each loop cycle


        // Update PIR Motion Sensor Reading
        //----------------------------------------------------------------------------

          if(pirIsStabilized)                     // make sure PIR Stabilization is complete
          {
            if(digitalRead(PIN_PIR_SENSOR))       // if PIR sensor pin is HIGH
            {
              // print message once per motion event
              if(!pirOutputPinState)
              {
                Debug.print(DBG_DEBUG, F("[D] * PIR Motion Detected %s"), getFormattedMillisString(millis()));
              }

              pirOutputPinState = true;           // set pir flag
              pirMotionTimestamp_ms = millis();   // update timestamp (used to eliminate hysterisis)

              uiLed.on();                         // set motion indicator LED (updtes in Output Actuators section)

            }
            else if ((millis() - pirMotionTimestamp_ms) >= PIR_RESET_TIME_MS) // prevent PIR LED from flickering while sensor settles
            {
              pirOutputPinState = false;          // turn off pir sensor flag
              uiLed.off();                        // turn off indicator LED

              //Debug.print(DBG_DEBUG, F("[D] * PIR = LOW"));
            }
          }

        // Update shutterCurrentState
        //----------------------------------------------------------------------------
          limitSwitch1State = digitalRead(PIN_LIMIT_SWITCH1);
          limitSwitch2State = digitalRead(PIN_LIMIT_SWITCH2);

          if( shutterDesiredState != RELEASED )
          {
            if(         (limitSwitch1State == LOW)
                     && (limitSwitch2State == HIGH)
            )
            {
              shutterCurrentState = CLOSED;
            }
            else if(   (limitSwitch1State == HIGH)
                    && (limitSwitch2State == LOW)
            )
            {
              shutterCurrentState = OPEN;
            }
            else if(   (limitSwitch1State == HIGH)
                    && (limitSwitch2State == HIGH)
            )
            {
              shutterCurrentState = TRANSITIONING;
            }
            else
            {
              shutterCurrentState = SHUTTER_ERROR;
            }
          } // END -- if( shutterDesiredState != RELEASED )
        // END -- Update shutterCurrentState


        // Update Potentiometer Reading
        //----------------------------------------------------------------------------
/*
          potValSamples.push(analogRead(PIN_POT));             // add new pot reading to the array

          // Use PotVal to set Fan speed, actuated at end of loop()
          int16_t tempPotValAvg = potValSamples.mean();        // calculate rolling average of potValReading array
          //int16_t tempPotValAvg = potValSamples.mode();

          if(     (tempPotValAvg > potValAvg+POT_TOLERANCE)    // test if value is beyond jitter tolerance
              ||  (tempPotValAvg < potValAvg-POT_TOLERANCE)
            )
          {
            potValAvg = tempPotValAvg;                        // update value
            potValUpdateFlag = true;                        // flag to handle input refresh

            fanSpeedPwm = map(potValAvg, 0, 900, FAN_PWM_MIN, FAN_PWM_MAX);   // map potVal range to fanSpeedPwm range

            //Debug.print(DBG_DEBUG,    F("\n\r[D] potValAvg: %d"), potValAvg );
            //Debug.print(DBG_DEBUG,    F("[D] fanSpeedPwm: %d"), fanSpeedPwm );

          }
*/
        // END -- Update Potentiometer Reading


      // -- END Handle Polled Iputs

      // Handle ISR Input Flags
      //----------------------------------------------------------------------------
        // Loop time is much faster than fanTach triggers ISR,
        // limit checking if fan is running to longer interval to
        // allow ISR to accumulate fanTachCounterNow counts before checking if fan is running...
        if( (millis() - fanCheckIntervalTimestamp_ms) >= FAN_CHECK_INTERVAL_MS)
        {
          fanCheckIntervalTimestamp_ms = millis();

          //if((fanTachCountNow - fanTachCountPrior) > 0 )  // fan is running
          if(fanTachCountNow != fanTachCountPrior)  // fan is running, allows for count overflow without throwing fan error
          {
            fanTachCountPrior = fanTachCountNow;    // equate counters for next check

            fanIsRunning = true;
            fanRunningTimestamp_ms = millis();      // refresh timestamp
          }
          else                                          // fan is not running
          {
            fanIsRunning = false;
          }

        } // -- END fan check interval

      // -- END Handle ISR Input Flags

    // END -- Handle Inputs
    //----------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------------------------------------- //
    // -- Finite State Machine(s) -- //
    //----------------------------------------------------------------------------------------------------------------------------- //

    switch(fsmStateNow)
    {


      //----------------------------------------------------------------------------
      case FSM_SYS_INIT_DEV:
      //----------------------------------------------------------------------------
      {

        // Initialize Device Settings & variables with Default Operational values
        //----------------------------------------------------------------------------
        // NOTE: This state handles all actions that should be performed on a "warm reboot" command

          // Device vars
          //----------------------------------------------------------------------------
            // deviceMode               // set below in // == Setup Device Operating Mode == //
            // debugLevel               // operates above the scope of FSM_SYS_INIT_DEV
            // fsmStatePrior            // operates above the scope of FSM_SYS_INIT_DEV
            // fsmStateNow              // operates above the scope of FSM_SYS_INIT_DEV
            // fsmStateNext             // operates above the scope of FSM_SYS_INIT_DEV

          // Loop () vars
          //----------------------------------------------------------------------------
            loopCount = 0;
            // loopCompleteTimestamp_us    // operates above the scope of FSM_SYS_INIT_DEV
            maxLoopTime_us = 0;
            avgLoopTime_us = 0;

            // Initialize Loop Statistics Array
              for(int i = 0; i < LOOP_TIME_NUM_SAMPLES; i++)
              {
                avgLoopTimeSamples_us.push(0);                 // fill Average object
              }

          // Session vars
          //----------------------------------------------------------------------------
            sessionIsActive = false;

            sessionStartTimestamp_ms = 0;
            sessionCounter = 0;
            sessionRepeatsListIndex = 0;

            // TODO setup for() loop to initialize
            sessionRepeatsList[0] = 1;
            sessionRepeatsList[1] = 5;

            sessionExitCondition = EXIT_NOT_SET;

          // Stage vars
          //----------------------------------------------------------------------------
            stageStartTimestamp_ms = 0;
            stageTimeLeft_sec = 0;

            stageExitCondition = EXIT_NOT_SET;

          // UI Input vars
          //----------------------------------------------------------------------------
            uiButtonPressedFlag = false;
            uiButtonHeldFlag = false;
            uiButtonSequenceFlag = false;

            // potVal                 // operates above the scope of FSM_SYS_INIT_DEV

          // UI Output vars
          //----------------------------------------------------------------------------
            // guiSync                // operates above the scope of FSM_SYS_INIT_DEV

            #ifdef PIXEL_CONFIG_H   // only include if pixel_config.h is being used

              uiPixelScene = PIX_OFF;

            #endif  // PIXEL_CONFIG_H


          // Sensor Input vars
          //----------------------------------------------------------------------------
            // potValSamples initialized in setup() bc pot state is evalated at top of loop()
            micThreshold_db = MIC_THRESHOLD_DEFAULT_DB;
            //micEventFanPwm = MIC_EVENT_FAN_PWM;


          // Actuator Output vars
          //----------------------------------------------------------------------------
            fanState = false;
            fanSpeedPwm = FAN_PWM_MIN;

            uvLedState = false;

            audioPlayer.stopPlaying();

            shutterDesiredState   = RELEASED;

            occupiedTimeout_sec   = OCCUPIED_NO_MOTION_TIMEOUT_SEC;
            unoccupiedTimeout_sec = UNOCCUPIED_TIMEOUT_SEC;
            dormantTimeout_sec    = DORMANT_TIMEOUT_SEC;


        //=========================================================================//
        // == Setup Device Operating Mode == //
        //=========================================================================//
        // Check flags / triggers to see if device operating mode should change
        //  * Button held during start up
        //  * Jumper settings
        //  * EEPROM setting

          // Define what flags / actions / triggers will select deviceMode
          //----------------------------------------------------------------------------
            if(uiButton.isPressed())                             // uiButton engaged during power up sequence
            {
              deviceMode = TEST_MODE;
              //deviceMode = DEVELOPMENT_MODE;
            }

        // END -- Setup Device Operating Mode



        //=========================================================================//
        // == Handle actions unique to selected deviceMode == //
        //=========================================================================//
          switch(deviceMode)
          {

            case DEV_MODE_NOT_SET:
            //----------------------------------------------------------------------------
            // Catch-all to ensure deviceMode is explicitly set
            {
              setFsmStateNext(FSM_SYS_INIT_ERROR);
              sessionExitCondition = EXIT_DEV_MODE_NOT_SET;

              Debug.print(DBG_ERROR,    F("[E] * ERROR @ %s"), getProgMemString(fsmStateString[fsmStateNow]) );
              Debug.print(DBG_ERROR,    F("[E]   * deviceMode(%s)"), getProgMemString(deviceModeString[deviceMode]) );
              Debug.print(DBG_ERROR,    F("[E]   * go to %s"), getProgMemString(fsmStateString[fsmStateNext]) );

            } // END -- DEV_MODE_NOT_SET
            break;

            case NORMAL_MODE:
            //----------------------------------------------------------------------------
            // Normal user-facing deviceMode operation
            //  * debug print statements suppressed
            {
              // Set debugLevel
                //debugLevel = DBG_NONE;            // Turn debug print statements OFF in NORMAL_MODE
                debugLevel = DBG_VERBOSE;

              // Set LED & Buzzer Scenes
                sysLed.blink(LED_HEARTBEAT);       // set led scene
                //EasyBuzzer.beep(BEEP_OK);          // set buzzer beep


              // Set up fsmState
              //----------------------------------------------------------------------------
                setFsmStateNext(FSM_SYS_INIT_APP);
                //setFsmStateNext(FSM_SYS_WAIT);                // go to WAIT state
                //setFsmStateAfterWait(FSM_SYS_INIT_APP);      // then go to next state

            } // END -- NORMAL_MODE
            break;

            case FIRST_RUN_MODE:
            //----------------------------------------------------------------------------
            // Production-facing deviceMode for devices that require factory configuration, calibration, provisioning, etc
            {
              // Set debugLevel

              // Set LED & Buzzer Scenes

              // Set up fsmState
              //----------------------------------------------------------------------------
                //setFsmStateNext(FSM_SYS_INIT_APP);

            } // END -- FIRST_RUN_MODE
            break;

            case SETUP_MODE:
            //----------------------------------------------------------------------------
            // User-facing deviceMode for initial device setup, configuration, preferences, credentials, etc
            {
              // Set debugLevel

              // Set LED & Buzzer Scenes

              // Set up fsmState
              //----------------------------------------------------------------------------
                //setFsmStateNext(FSM_SYS_INIT_APP);

            } // END -- SETUP_MODE
            break;

            case DEVELOPMENT_MODE:
            //----------------------------------------------------------------------------
            // Developer-facing deviceMode
            //  * Go immediately to FSM_SYS_WAIT upon boot and wait for user action to proceed (e.g. button press, Serial input, etc)
            //  * Turn on Verbose Debug statements
            {
              // Set debugLevel
                debugLevel = DBG_VERBOSE;

              // Set LED & Buzzer Scenes
                sysLed.blink(LED_ATTENTION);       // set attention led scene
                //EasyBuzzer.beep(BEEP_ATTENTION);  // set attention buzzer beep

                audioPlayer.stopPlaying();
                audioPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working


              // Set up fsmState
              //----------------------------------------------------------------------------
                setFsmStateNext(FSM_SYS_WAIT);          // go to WAIT state
                setFsmStateAfterWait(FSM_SYS_INIT_APP);      // then go to next state

                //setFsmStateAfterWait(FSM_SYS_INIT_ERROR);      // used to TEST ERROR STATE
                //setFsmStateAfterWait(FSM_SYS_INIT_ALERT);      // used to TEST ALERT STATE

            } // END -- DEVELOPMENT_MODE
            break;

            case TEST_MODE:
            //----------------------------------------------------------------------------
            // Production-facing deviceMode to run a device self-test routine
            //  * Test UI elements (LEDs, Buzzers, buttons, etc) after assembly
            {
              // Set debugLevel
                debugLevel = DBG_VERBOSE;

              // Set LED & Buzzer Scenes
                // cannot use EasyBuzzer here bc of hold in while() below
              //analogWrite(PIN_BUZZER, 32);        // set buzzer to low buzz to test

              while(uiButton.isPressed())         // hold here until uiButton is released
              {
                uiButton.read();                  // update button state
                Watchdog.reset();                 // keep watchdog alive while stuck here
              }

              uiButtonPressedFlag = false;           // reset uiButtonPressedFlag


              // Set up fsmState
              //----------------------------------------------------------------------------
                // Go to special self-test Production FSM state (if applicable)
                setFsmStateNext(FSM_SYS_INIT_APP);

            } // END -- TEST_MODE
            break;

            case BURN_IN_MODE:
            //----------------------------------------------------------------------------
            // Production-facing deviceMode to put the device into a multiple-run cycle and collect performance data
            {
              // Set debugLevel

              // Set LED & Buzzer Scenes

              // Set up fsmState
              //----------------------------------------------------------------------------
                //setFsmStateNext(FSM_SYS_INIT_APP);

            } // END -- BURN_IN_MODE
            break;

            default:
            {
              // Set up fsmState
              //----------------------------------------------------------------------------
                setFsmStateNext(FSM_SYS_INIT_ERROR);
                sessionExitCondition = EXIT_DEV_MODE_INVALID;

                Debug.print(DBG_ERROR,    F("[E] * ERROR @ %s"), getProgMemString(fsmStateString[fsmStateNow]) );
                Debug.print(DBG_ERROR,    F("[E]   * deviceMode(%d) invalid"), deviceMode);
                Debug.print(DBG_ERROR,    F("[E]   * go to = %s"), getProgMemString(fsmStateString[fsmStateNext]));

            } // END -- default case
            break;

          } // END -- switch(deviceMode)

          Debug.setDebugLevel(debugLevel);    // update debugLevel based on deviceMode setting

          // Special case: Use deviceMode debugLevel to print header for FSM_SYS_INIT_DEV
          //  * allows deviceMode to determine initial print statements
          Debug.print(DBG_DEBUG, F("\n\r[%s] %s"), getFormattedMillisString(millis()), getProgMemString(fsmStateString[fsmStateNow]));
          if(debugLevel >= DBG_DEBUG){printSymbolBreak('-');}

        // END -- Setup Device Operating Mode


        // Notify user if buttons are capable of supporting Interrupt
          if (uiButton.supportsInterrupt())         // interrupt driven button
          {
            Debug.print(DBG_VERBOSE, F("[V] uiButton: INTERRUPT supported"));
          }
          else
          {
            Debug.print(DBG_VERBOSE, F("[V] uiButton: POLLED only"));
          }


        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          // fsmStateNext should be set in a deviceMode case above, then transition here...
          goToFsmStateNext();

      } // END -- FSM_SYS_INIT_DEV
      break;



      //----------------------------------------------------------------------------
      case FSM_SYS_WAIT:
      //----------------------------------------------------------------------------
      {

        uint32_t currentMillis_ms = 0;            // print wait message once

        if(WAIT_MESSAGE_PRINT_PERIODICALLY)       // print periodically (shows activity on Serial Monitor)
        {
          currentMillis_ms = millis();
        }

        uint8_t currentTick = (currentMillis_ms - waitStateRefreshTimestamp_ms) / 1000;  // whole number of seconds since timestamp

        // Check to refresh Wait Message and tick marks
        //----------------------------------------------------------------------------
          if ( (currentMillis_ms - waitStateRefreshTimestamp_ms) >= WAIT_MESSAGE_REFRESH_INTERVAL_MS  ) // duration since timestamp is greater than interval
          {
            waitStateRefreshTimestamp_ms = currentMillis_ms;           // reset timestamp

            // Print formatted time
              //Serial.print(F("\n\r["));
              //Serial.print(getFormattedMillisString(currentMillis_ms));
              //Serial.print(F("] "));
              //Serial.print(F("* Waiting for user "));                    // print wait message

            Debug.print(DBG_INFO,    F("[I] Waiting for user "));

            audioPlayer.stopPlaying();  // ensure sine test has stopped sounding

          }
          // Check to refresh ticks
          else if ( currentTick != waitStateTickCounter )         // tick counter has changed
          {
            waitStateTickCounter = currentTick;                       // reset tick counter

            // use Serial.print instead of Debug.print() to ignore the state of Debug.timestampXX()
            if(DEBUG_LEVEL_DEFAULT >= DBG_INFO)
            {
              Serial.print(".");                                       // print tick mark
            }
          }

        // Check if we can leave the Wait state
        //----------------------------------------------------------------------------
          if (uiButtonPressedFlag)                  // prevents accidentally triggering session from uiButton being held down during power up
          {
            uiButtonPressedFlag = false;            // reset buttonState

            waitStateRefreshTimestamp_ms = WAIT_MESSAGE_REFRESH_INTERVAL_MS;       // reset timestamp so it will automatically run once upon returning

            // Set up transition to next fsmState
            //----------------------------------------------------------------------------

              // NOTE setFsmStateAfterWait() must be used before entering FSM_SYS_WAIT
              goToFsmStateNext();
          }

      } // END ----- FSM_SYS_WAIT
      break;


      //----------------------------------------------------------------------------
      case FSM_SYS_INIT_APP:
      //----------------------------------------------------------------------------
      {
        sysLed.blink(LED_HEARTBEAT);   // set led scene for App Initialization
        //uiLed.blink(LED_STABILIZING);   // set led scene to show PIR needs to stabilize
        //EasyBuzzer.stopBeep();

        // use Serial.print instead of Debug.print() to ignore the state of Debug.timestampXX()
        if(debugLevel >= DBG_INFO)
        {
          // Print App Info
          printAppInfo();

          // Print Device Info
          Serial.println();
          printDeviceStats();

          Serial.println();
          printDeviceInfo();

        }

        //Serial.print(getProgMemString(symbolLineBreak));

        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          setFsmStateNext(FSM_SYS_INIT_STARTUP);
          goToFsmStateNext();

      } // END -- FSM_SYS_INIT_APP
      break;




      //----------------------------------------------------------------------------
      case FSM_SYS_INIT_STARTUP:
      //----------------------------------------------------------------------------
      {

        stageStartTimestamp_ms = millis();  // Start recording duration in stage


        audioPlayer.stopPlaying();                      // stop any prior audio
        audioPlayer.setVolume(AUDIO_VOLUME, AUDIO_VOLUME);
        audioPlayer.startPlayingFile(STARTUP_AUDIO);  // Play Start Up Sound


        uiLed.blink(LED_STABILIZING);   // set led scene to show PIR needs to stabilize

        // Turn Fan on Low
        fanState = true;
        fanSpeedPwm = FAN_PWM_MIN;
        Debug.print(DBG_DEBUG, F("[D] * Fan:\t LOW") );

        shutterDesiredState = CLOSED;
        Debug.print(DBG_DEBUG, F("[D] * Shutter:\t CLOSED") );

        // keep UV LEDs off until:
        //  * shutter is confirmed CLOSED
        //  * fan is confirmed ON
        uvLedState = false;

        //pirIsStabilized = false;  // this is set as false power up and will still be stable after "warm boot"


        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          setFsmStateNext(FSM_SYS_RUN_STARTUP);
          goToFsmStateNext();

      } // END -- FSM_SYS_INIT_STARTUP
      break;


      //----------------------------------------------------------------------------
      case FSM_SYS_RUN_STARTUP:
      //----------------------------------------------------------------------------
      {

        // Wait for PIR to Stabilize...
        //----------------------------------------------------------------------------

          uint32_t currentMillis_ms = millis();  // uncomment to print periodically
          //uint32_t currentMillis_ms = 0;            // uncomment to print once

          if(currentMillis_ms <= PIR_STABILIZATION_TIME_MS)      // Check to see if device has been powered on long enough for PIR to stabilize
          {
            uint8_t currentTick = (currentMillis_ms - waitStateRefreshTimestamp_ms) / 1000;  // whole number of seconds since timestamp

            // Check to refresh Wait Message and tick marks
            //----------------------------------------------------------------------------
              if (    ((currentMillis_ms - waitStateRefreshTimestamp_ms) >= WAIT_MESSAGE_REFRESH_INTERVAL_MS  ) // duration since timestamp is greater than interval
                  &&  (debugLevel >= DBG_INFO)
                )
              {
                waitStateRefreshTimestamp_ms = currentMillis_ms;           // reset timestamp

                // Print formatted time
                Serial.print(F("\n\r["));
                Serial.print(getFormattedMillisString(currentMillis_ms));
                Serial.print(F("] "));

                Serial.print(F("* "));                    // print wait message
                Serial.print((PIR_STABILIZATION_TIME_MS-currentMillis_ms)/1000);                    // print wait message
                Serial.print(F(" sec until PIR stabilization "));                    // print wait message
              }
              // Check to refresh ticks
              else if (     currentTick != waitStateTickCounter )         // tick counter has changed
              {
                waitStateTickCounter = currentTick;                       // reset tick counter
                Serial.print(".");                                       // print tick mark
              }

          }
          else if (!pirIsStabilized)                                    // PIR is stabilized...
          {
            waitStateRefreshTimestamp_ms = WAIT_MESSAGE_REFRESH_INTERVAL_MS;       // reset timestamp so it will automatically run once upon returning

            uiLed.off();   // stop uiLed from indicating that it is still stabilizing

            pirIsStabilized = true;

            Serial.println();
            Debug.print(DBG_DEBUG, F("[D] * PIR is Stabilized") );
          }
        // END Wait for PIR to Stabilize


        // Check if UV LEDs can turn on while PIR is stabilizing...
          if(     (shutterCurrentState == CLOSED)
              &&  fanIsRunning
              &&  !uvLedState
            )
          {
            uvLedState = true;
            Serial.println();
            Debug.print(DBG_DEBUG, F("[D] * UV LED:\t ON") );
          }


        // Check if we can leave this state...
          if(     pirIsStabilized
              &&  (shutterCurrentState == CLOSED)
            )
          {
            // Set up transition to next fsmState
            //----------------------------------------------------------------------------
              setFsmStateNext(FSM_SYS_EXIT_STARTUP);
              goToFsmStateNext();
          }

      } // END -- FSM_SYS_RUN_STARTUP
      break;


      //----------------------------------------------------------------------------
      case FSM_SYS_EXIT_STARTUP:
      //----------------------------------------------------------------------------
      {
        Debug.print(DBG_DEBUG, F("[D] * stage runtime: %s ms"), getFormattedMillisString(millis()-stageStartTimestamp_ms) );

        stageStartTimestamp_ms = 0;     // reset timestamp

        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          // TEST CODE: TODO remove
          ///setFsmStateNext(FSM_SYS_WAIT);
          //setFsmStateAfterWait(FSM_SES_INIT);      // then go to next state

          setFsmStateNext(FSM_SES_INIT);
          goToFsmStateNext();

      } // END -- FSM_SYS_EXIT_STARTUP
      break;




      //----------------------------------------------------------------------------
      case FSM_SES_INIT:
      //----------------------------------------------------------------------------
      {
        //EasyBuzzer.beep(BEEP_OK);     // set buzzer beep
        //EasyBuzzer.beep(BEEP_ATTENTION);     // set buzzer beep

        // Initialize Session variables
        //----------------------------------------------------------------------------

          stageStartTimestamp_ms = 0;
          uiButtonPressedFlag  = false;

          sessionIsActive = true;
          sessionStartTimestamp_ms = millis();

          sessionCounter++;

        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          setFsmStateNext(FSM_STG_INIT_OCCUPIED);
          goToFsmStateNext();

      } // END -- FSM_SES_INIT
      break;



      //----------------------------------------------------------------------------
      case FSM_STG_INIT_OCCUPIED:
      //----------------------------------------------------------------------------
      {

        stageStartTimestamp_ms = millis();  // Start recording duration in stage

        audioPlayer.stopPlaying();                      // stop any prior audio
        audioPlayer.setVolume(AUDIO_VOLUME, AUDIO_VOLUME);
        audioPlayer.startPlayingFile(OCCUPIED_NOTIFICATION_AUDIO);  // Play Alert Sound

        pirMotionTimestamp_ms = millis();   // Start PIR Motion timer

        fanState = true;
        fanSpeedPwm = FAN_PWM_MIN;
        Debug.print(DBG_DEBUG, F("[D] * Fan:\t LOW") );

        shutterDesiredState = CLOSED;
        Debug.print(DBG_DEBUG, F("[D] * Shutter:\t CLOSED") );

        // uvLedState set and monitored in RUN for safety
        Debug.print(DBG_DEBUG, F("[D] * UV LED:\t ON") );

        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          setFsmStateNext(FSM_STG_RUN_OCCUPIED);
          goToFsmStateNext();

      } // END -- FSM_STG_INIT_OCCUPIED
      break;


      //----------------------------------------------------------------------------
      case FSM_STG_RUN_OCCUPIED:
      //----------------------------------------------------------------------------
      {
        // Safety: Monitor shutterCurrentState allow UV LEDs to be ON ...
          if(     (shutterCurrentState == shutterDesiredState)    // shutterDesiredState set in FSM_STG_INIT_OCCUPIED
              &&  fanIsRunning
              //&&  !uvLedState
            )
          {
            uvLedState = true;
            //Debug.print(DBG_DEBUG, F("[D] * UV LEDs are ON") );
          }
          else
          {
            uvLedState = false;
          }

        // Safety: Monitor fanIsRunning allow UV LEDs to be ON ...
          if(     !fanIsRunning
              &&  ((millis() - stageStartTimestamp_ms) >= FAN_TIMEOUT_MS)   // give the fan some time to spin up
            )
          {
            uvLedState = false;

            // GOTO ERROR STATE...
            sessionExitCondition = EXIT_SES_FAN_ERROR;

            // Set up transition to next fsmState
            //----------------------------------------------------------------------------
              setFsmStateNext(FSM_SYS_INIT_ERROR);
              goToFsmStateNext();
          }
        // END uvLedState Safety Check

        // Mic Sound Level Test - ONLY needs to run in FSM_STG_RUN_OCCUPIED
          if(    audioPlayer.stopped()          // Ensure Audio is not playing before sound level reading
              //&& (fanSpeedPwm <= FAN_PWM_MIN)   // Ensure Fan is not running above lowest (quietest) setting; Max Fan noise = 48dB, so won't trigger mic threshold = 50dB
              // Servo only steps between loops, so will not be running during this check
          )
          {
            uint16_t micSample = 0;             // var to hold mic reading

            uint16_t micSamplePeakToPeak = 0;  // peak-to-peak voltage amplitude of sound recording
            uint16_t micSampleMax = 0;         // initial value to estalish recording MAX at floor
            uint16_t micSampleMin = 1024;      // initial value to estalish recording MIN at ceiling

            uint32_t listeningStartTimestamp_ms = millis();

            double micVoltagePeakToPeak = 0.0;
            double micPeak_dB = 0.0;

            // BLOCKING section - keep as short as possible!
            while( (millis() - listeningStartTimestamp_ms) < MIC_SAMPLE_DURATION_MS)
            {
              micSample = analogRead(PIN_MIC);  // sample mic voltage

              if (micSample < 1024)  // toss out spurious readings
              {
                 if (micSample > micSampleMax)
                 {
                    micSampleMax = micSample;  // save just the max levels
                 }
                 else if (micSample < micSampleMin)
                 {
                    micSampleMin = micSample;  // save just the min levels
                 }
              }
            } // END BLOCKING while()

            // Calculate dB level from mic readings
            micSamplePeakToPeak = micSampleMax - micSampleMin;            // calculate V peak-to-peak amplitude of listening window

            micVoltagePeakToPeak = (micSamplePeakToPeak * (double)MIC_VCC) / 1024.0;   // convert raw value to volts

            micPeak_dB = 20.0*log(10)*(micVoltagePeakToPeak/(double)MIC_VCC);                        // convert volts to dB
            micPeak_dB = micPeak_dB + MIC_CALIBRATION_DB;                 // add calibration value adjustment

            if(micPeak_dB > micThreshold_db)        // if noise detected above threshold value
            {

              Debug.print(DBG_DEBUG, F("[D] * %.2f dB Mic Event Detected %s"), micPeak_dB, getFormattedMillisString(millis()) );

              //micEventDetectedFlag = true;        // handle event flag in "Update Actuator Outputs"
              micEventFanTimer_ms = millis() + (MIC_EVENT_FAN_DURATION_SEC * 1000);  // set timer for increase fan speed due to mic event
              Debug.print(DBG_DEBUG, F("[D] * micEventFanTimer_ms = %s"), getFormattedMillisString(micEventFanTimer_ms) );

            }

          } // END Mic Sound Level Test


        // Decide if we can leave this state due to No Motion Timeout ...
        // pirMotionTimestamp_ms updated in Handle Polled Inputs section
          if( (millis() - pirMotionTimestamp_ms) >= (occupiedTimeout_sec * 1000))
          {
            // Set up transition to next fsmState
            //----------------------------------------------------------------------------
              setFsmStateNext(FSM_STG_EXIT_OCCUPIED);
              goToFsmStateNext();
          }


      } // END -- FSM_STG_RUN_OCCUPIED
      break;


      //----------------------------------------------------------------------------
      case FSM_STG_EXIT_OCCUPIED:
      //----------------------------------------------------------------------------
      {
        Debug.print(DBG_DEBUG, F("[D] * stage runtime: %s ms"), getFormattedMillisString(millis()-stageStartTimestamp_ms) );

        stageStartTimestamp_ms = 0;     // reset timestamp

        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          setFsmStateNext(FSM_STG_INIT_COUNTDOWN);
          goToFsmStateNext();

      } // END -- FSM_STG_EXIT_OCCUPIED
      break;



      //----------------------------------------------------------------------------
      case FSM_STG_INIT_COUNTDOWN:
      //----------------------------------------------------------------------------
      {
        stageStartTimestamp_ms = millis();  // Start recording duration in stage

        //countdownTimestamp_ms = millis();   // Start countdown timer

        audioPlayer.stopPlaying();                      // stop any prior audio
        audioPlayer.setVolume(AUDIO_VOLUME, AUDIO_VOLUME);
        audioPlayer.startPlayingFile(COUNTDOWN_AUDIO);  // Start playing Countdown audio

        fanState = true;
        fanSpeedPwm = FAN_PWM_MIN;
        Debug.print(DBG_DEBUG, F("[D] * Fan:\t LOW") );

        shutterDesiredState = CLOSED;
        Debug.print(DBG_DEBUG, F("[D] * Shutter:\t CLOSED") );

        // uvLedState set and monitored in RUN for safety
        Debug.print(DBG_DEBUG, F("[D] * UV LED:\t ON") );

        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          setFsmStateNext(FSM_STG_RUN_COUNTDOWN);
          goToFsmStateNext();

      } // END -- FSM_STG_INIT_COUNTDOWN
      break;


      //----------------------------------------------------------------------------
      case FSM_STG_RUN_COUNTDOWN:
      //----------------------------------------------------------------------------
      {

        // Safety: Monitor shutterCurrentState allow UV LEDs to be ON ...
          if(     (shutterCurrentState == shutterDesiredState)    // shutterDesiredState set in FSM_STG_INIT_OCCUPIED
              //&&  fanIsRunning
              //&&  !uvLedState
            )
          {
            uvLedState = true;
            //Debug.print(DBG_DEBUG, F("[D] * UV LEDs are ON") );
          }
          else
          {
            uvLedState = false;
          }

        // Safety: Monitor fanIsRunning allow UV LEDs to be ON ...
          if(     !fanIsRunning
              &&  ((millis() - stageStartTimestamp_ms) >= FAN_TIMEOUT_MS)   // give the fan some time to spin up
            )
          {
            uvLedState = false;

            // GOTO ERROR STATE...
            sessionExitCondition = EXIT_SES_FAN_ERROR;

            // Set up transition to next fsmState
            //----------------------------------------------------------------------------
              setFsmStateNext(FSM_SYS_INIT_ERROR);
              goToFsmStateNext();
          }
        // END uvLedState Safety Check

        // Watch for motion ...
          if(pirOutputPinState)                         // if motion is detected ...
          {
            //Debug.print(DBG_DEBUG, F("[D] * PIR motion detected") );

            audioPlayer.stopPlaying();                  // stop countdown audio

            // Set up transition to next fsmState
            //----------------------------------------------------------------------------
              setFsmStateNext(FSM_STG_INIT_OCCUPIED);   // return to OCCUPIED state
              goToFsmStateNext();
          }

        // Decide if we can go to next state ...
          if(     audioPlayer.stopped()                 // countdown audio is finished
              &&  ((millis() - stageStartTimestamp_ms) >= (COUNTDOWN_TIME_SEC * 1000))   // min countdown time has elapsed
            )
          {
            Debug.print(DBG_DEBUG, F("[D] * Countdown complete") );

            // Set up transition to next fsmState
            //----------------------------------------------------------------------------
              setFsmStateNext(FSM_STG_EXIT_COUNTDOWN);
              goToFsmStateNext();
          }

      } // END -- FSM_STG_RUN_COUNTDOWN
      break;


      //----------------------------------------------------------------------------
      case FSM_STG_EXIT_COUNTDOWN:
      //----------------------------------------------------------------------------
      {
        Debug.print(DBG_DEBUG, F("[D] * stage runtime: %s ms"), getFormattedMillisString(millis()-stageStartTimestamp_ms) );

        stageStartTimestamp_ms = 0;     // reset timestamp

        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          setFsmStateNext(FSM_STG_INIT_UNOCCUPIED);
          goToFsmStateNext();

      } // END -- FSM_STG_EXIT_COUNTDOWN
      break;


      //----------------------------------------------------------------------------
      case FSM_STG_INIT_UNOCCUPIED:
      //----------------------------------------------------------------------------
      {

        stageStartTimestamp_ms = millis();  // Start recording duration in stage

        audioPlayer.stopPlaying();                      // stop any prior audio
        audioPlayer.setVolume(AUDIO_VOLUME, AUDIO_VOLUME);
        audioPlayer.startPlayingFile(UNOCCUPIED_NOTIFICATION_AUDIO);  // Play UNOCCUPIED notification

        fanState = true;
        fanSpeedPwm = FAN_PWM_MAX;
        Debug.print(DBG_DEBUG, F("[D] * Fan:\t HIGH") );

        shutterDesiredState = OPEN;
        Debug.print(DBG_DEBUG, F("[D] * Shutter:\t OPEN") );

        // uvLedState NOT monitored in RUN bc can be left ON when UNOCCUPIED
        Debug.print(DBG_DEBUG, F("[D] * UV LED:\t ON") );

        micEventFanTimer_ms = 0;  // Release fan control from Mic Event

        pirOutputPinState = false; // clear pre-existing motion detection


        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          setFsmStateNext(FSM_STG_RUN_UNOCCUPIED);
          goToFsmStateNext();

      } // END -- FSM_STG_INIT_UNOCCUPIED
      break;


      //----------------------------------------------------------------------------
      case FSM_STG_RUN_UNOCCUPIED:
      //----------------------------------------------------------------------------
      {

        // Safety: Monitor fanIsRunning allow UV LEDs to be ON ...
          if(     !fanIsRunning
              &&  ((millis() - stageStartTimestamp_ms) >= FAN_TIMEOUT_MS)   // give the fan some time to spin up
            )
          {
            uvLedState = false;

            // GOTO ERROR STATE...
            sessionExitCondition = EXIT_SES_FAN_ERROR;

            // Set up transition to next fsmState
            //----------------------------------------------------------------------------
              setFsmStateNext(FSM_SYS_INIT_ERROR);
              goToFsmStateNext();
          }
        // END uvLedState Safety Check

        // Watch for motion ...
          if(    (shutterCurrentState == OPEN)                  // wait until shutter is open to prevent shutter from triggering PIR Motion Sensor
          )
          {
            if(pirOutputPinState)               // if motion is detected ...
            {
              //Debug.print(DBG_DEBUG, F("[D] * PIR motion detected") );

              audioPlayer.stopPlaying();                  // stop audio

              // Set up transition to next fsmState
              //----------------------------------------------------------------------------
                setFsmStateNext(FSM_STG_INIT_OCCUPIED);   // return to OCCUPIED state
                goToFsmStateNext();
            }

          } // END if shutterCurrentState == OPEN

        // Decide if we can go to next state ...
          if(  ((millis() - stageStartTimestamp_ms) >= (unoccupiedTimeout_sec * 1000))   // max duration has elapsed
            )
          {
            Debug.print(DBG_DEBUG, F("[D] * UNOCCUPIED duration timeout") );

            // Set up transition to next fsmState
            //----------------------------------------------------------------------------
              setFsmStateNext(FSM_STG_EXIT_UNOCCUPIED);
              goToFsmStateNext();
          }

      } // END -- FSM_STG_RUN_UNOCCUPIED
      break;


      //----------------------------------------------------------------------------
      case FSM_STG_EXIT_UNOCCUPIED:
      //----------------------------------------------------------------------------
      {
        Debug.print(DBG_DEBUG, F("[D] * stage runtime: %s ms"), getFormattedMillisString(millis()-stageStartTimestamp_ms) );

        stageStartTimestamp_ms = 0;     // reset timestamp

        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          setFsmStateNext(FSM_STG_INIT_DORMANT);
          goToFsmStateNext();

      } // END -- FSM_STG_EXIT_UNOCCUPIED
      break;



      //----------------------------------------------------------------------------
      case FSM_STG_INIT_DORMANT:
      //----------------------------------------------------------------------------
      {

        stageStartTimestamp_ms = millis();  // Start recording duration in stage

        audioPlayer.stopPlaying();                      // stop any prior audio
        audioPlayer.setVolume(AUDIO_VOLUME, AUDIO_VOLUME);
        audioPlayer.startPlayingFile(DORMANT_NOTIFICATION_AUDIO);  // Play alert Sound

        fanState = true;
        fanSpeedPwm = FAN_PWM_MAX;
        Debug.print(DBG_DEBUG, F("[D] * Fan:\t HIGH for %d s"), (FAN_UV_LED_COOLDOWN_TIME_MS/1000) );

        shutterDesiredState = CLOSED;
        Debug.print(DBG_DEBUG, F("[D] * Shutter:\t CLOSED") );

        uvLedState = false;
        Debug.print(DBG_DEBUG, F("[D] * UV LED:\t OFF") );

        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          setFsmStateNext(FSM_STG_RUN_DORMANT);
          goToFsmStateNext();

      } // END -- FSM_STG_INIT_DORMANT
      break;


      //----------------------------------------------------------------------------
      case FSM_STG_RUN_DORMANT:
      //----------------------------------------------------------------------------
      {

        // Watch for motion ...
          if(     (pirOutputPinState)                             // if motion is detected ...
              //&&  ((millis() - stageStartTimestamp_ms) >= 2000)   // NOT NEEDED - delay PIR monitor to allow heat cloud cuased by closing shutter to dissapate
          )
          {
            //Debug.print(DBG_DEBUG, F("[D] * PIR motion detected") );

            audioPlayer.stopPlaying();                  // stop countdown audio

            // Set up transition to next fsmState
            //----------------------------------------------------------------------------
              setFsmStateNext(FSM_STG_INIT_OCCUPIED);   // return to OCCUPIED state
              goToFsmStateNext();
          }

        // Keep fan running to cool off UV LEDs for a bit
          if(  ((millis() - stageStartTimestamp_ms) <= FAN_UV_LED_COOLDOWN_TIME_MS)
            )
          {
            fanState = true;
          }
          else if (fanState)        // run once after duration has expired
          {
            fanState = false;       // turn fan OFF when duration has expired

            Debug.print(DBG_DEBUG, F("[D] * Fan cooldown complete") );
          }


        // Decide if we can go to next state ...
          if(  ((millis() - stageStartTimestamp_ms) >= (dormantTimeout_sec * 1000))   // max duration has elapsed
            )
          {
            Debug.print(DBG_DEBUG, F("[D] * DORMANT duration timeout") );

            // Set up transition to next fsmState
            //----------------------------------------------------------------------------
              setFsmStateNext(FSM_STG_EXIT_DORMANT);
              goToFsmStateNext();
          }

      } // END -- FSM_STG_RUN_DORMANT
      break;


      //----------------------------------------------------------------------------
      case FSM_STG_EXIT_DORMANT:
      //----------------------------------------------------------------------------
      {
        Debug.print(DBG_DEBUG, F("[D] * stage runtime: %s ms"), getFormattedMillisString(millis()-stageStartTimestamp_ms) );

        stageStartTimestamp_ms = 0;     // reset timestamp

        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          setFsmStateNext(FSM_SES_COUNT_CHECK);
          goToFsmStateNext();

      } // END -- FSM_STG_EXIT_DORMANT
      break;


      //----------------------------------------------------------------------------
      case FSM_SES_COUNT_CHECK:
      //----------------------------------------------------------------------------
      {

        Debug.print(DBG_DEBUG,    F("[D] * sessionCounter: %d"), sessionCounter);

        if(sessionCounter >= sessionRepeatsList[sessionRepeatsListIndex]) // number of session repeats completed
        {
          // Reset Session Repeats variables
          sessionCounter = 0;
          sessionRepeatsListIndex = 0;

          // Set up transition to next fsmState
          //----------------------------------------------------------------------------
            setFsmStateNext(FSM_SES_INIT_CLOSE);

        }
        else
        {
          sessionCounter++;     // increment sessionCounter

          // Proceed to next Session iteration...

          // Set up transition to next fsmState
          //----------------------------------------------------------------------------
            setFsmStateNext(FSM_SES_INIT);

            if(debugLevel >= DBG_DEBUG){printSymbolBreak('-');}
        }

        if(debugLevel >= DBG_DEBUG){Serial.println(); printSymbolBreak('=');}

        goToFsmStateNext();

      } // END -- FSM_SES_COUNT_CHECK
      break;


      //----------------------------------------------------------------------------
      case FSM_SES_INIT_CLOSE:
      //----------------------------------------------------------------------------
      {
        uint32_t sessionRunTime_ms = millis()-sessionStartTimestamp_ms;
        Debug.print(DBG_DEBUG, F("[D] * session runtime: %s ms"), getFormattedMillisString(sessionRunTime_ms) );

        sessionStartTimestamp_ms = 0;     // reset timestamp

        sessionIsActive = false;          // end session flag

        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          setFsmStateNext(FSM_SES_RUN_CLOSE);
          goToFsmStateNext();

      } // END -- FSM_SES_INIT_CLOSE
      break;



      //----------------------------------------------------------------------------
      case FSM_SES_RUN_CLOSE:
      //----------------------------------------------------------------------------
      {

        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          setFsmStateNext(FSM_SES_EXIT_CLOSE);
          goToFsmStateNext();

      }   // END -- FSM_SES_RUN_CLOSE
      break;



      //----------------------------------------------------------------------------
      case FSM_SES_EXIT_CLOSE:
      //----------------------------------------------------------------------------
      {


        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          //setFsmStateNext(FSM_SYS_WAIT);            // go to WAIT state
          //setFsmStateAfterWait(FSM_SES_INIT);       // then go to next state

          setFsmStateNext(FSM_SES_INIT);            // restart session

          goToFsmStateNext();

      } // END -- FSM_SES_EXIT_CLOSE
      break;




      //----------------------------------------------------------------------------
      case FSM_SYS_INIT_ALERT:
      //----------------------------------------------------------------------------
      {

        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          setFsmStateNext(FSM_SYS_RUN_ALERT);
          goToFsmStateNext();

      } // END -- FSM_SYS_INIT_ALERT
      break;



      //----------------------------------------------------------------------------
      case FSM_SYS_RUN_ALERT:
      //----------------------------------------------------------------------------
      {


        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          setFsmStateNext(FSM_SYS_EXIT_ALERT);
          goToFsmStateNext();

      } // END -- FSM_SYS_RUN_ALERT
      break;



      //----------------------------------------------------------------------------
      case FSM_SYS_EXIT_ALERT:
      //----------------------------------------------------------------------------
      {


        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          setFsmStateNext(FSM_SES_INIT);
          goToFsmStateNext();


      } // END -- FSM_SYS_EXIT_ALERT
      break;



      //----------------------------------------------------------------------------
      case FSM_SYS_INIT_ERROR:
      //----------------------------------------------------------------------------
      {
        sysLed.blink(LED_ERROR);       // set led scene
        //EasyBuzzer.beep(BEEP_ERROR);  // set attention buzzer beep

        audioPlayer.stopPlaying();                      // stop any prior audio
        audioPlayer.setVolume(AUDIO_VOLUME, AUDIO_VOLUME);
        audioPlayer.startPlayingFile(ERROR_NOTIFICATION_AUDIO);  // Play alert Sound

        fanState = false;
        Debug.print(DBG_DEBUG, F("[D] * Fan:\t OFF") );

        shutterDesiredState = RELEASED;
        Debug.print(DBG_DEBUG, F("[D] * Shutter:\t RELEASED") );

        uvLedState = false;
        Debug.print(DBG_DEBUG, F("[D] * UV LED:\t OFF") );

        Serial.println();

        Debug.print(DBG_ERROR,    F("[E] * ERROR from %s: %s"), getProgMemString(fsmStateString[fsmStatePrior]), getProgMemString(exitConditionString[sessionExitCondition]));
        Debug.print(DBG_ERROR,    F("[E]   * Push button to continue...\n\r"));
        if(debugLevel >= DBG_ERROR){printSymbolBreak('*');}

        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          setFsmStateNext(FSM_SYS_RUN_ERROR);
          goToFsmStateNext();

      } // END -- FSM_SYS_INIT_ERROR
      break;



      //----------------------------------------------------------------------------
      case FSM_SYS_RUN_ERROR:
      //----------------------------------------------------------------------------
      {

        // Wait for button press to proceed
        if (uiButtonPressedFlag)
        {
          uiButtonPressedFlag = false;            // reset buttonState

          // Set up transition to next fsmState
          //----------------------------------------------------------------------------
            setFsmStateNext(FSM_SYS_EXIT_ERROR);
            goToFsmStateNext();
        }

      } // END -- FSM_SYS_RUN_ERROR
      break;



      //----------------------------------------------------------------------------
      case FSM_SYS_EXIT_ERROR:
      //----------------------------------------------------------------------------
      {


        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          setFsmStateNext(FSM_SYS_INIT_DEV);
          goToFsmStateNext();


      } // END -- FSM_SYS_EXIT_ERROR
      break;




      //----------------------------------------------------------------------------
      case FSM_NOT_SET:
      //----------------------------------------------------------------------------
      {

        Debug.print(DBG_ERROR, F("[E] * ERROR: %s"), getProgMemString(fsmStateString[fsmStateNow]));

        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          //setFsmStateNext(FSM_SYS_INIT_DEV);
          //Debug.print(DBG_WARNING, F("[W] * WARNING: %s, redirecting to %s"), getProgMemString(fsmStateString[fsmStateNow]), getProgMemString(fsmStateString[fsmStateNext]));

          setFsmStateNext(FSM_SYS_INIT_ERROR);
          goToFsmStateNext();


      } // END -- FSM_NOT_SET
      break;


      //----------------------------------------------------------------------------
      default:
      // Catch-all FSM state. Entering this state indicates unhandled fsmState change.
      //----------------------------------------------------------------------------
      {

        Debug.print(DBG_ERROR, F("[E] * ERROR: fsmStateNow = %d is not valid\n\r%"), fsmStateNow);

        // Set up transition to next fsmState
        //----------------------------------------------------------------------------
          //setFsmStateNext(FSM_SES_INIT);
          setFsmStateNext(FSM_SYS_INIT_ERROR);
          goToFsmStateNext();

      } // END -- default fsm case
      break;

    }

    // END -- Finite State Machine(s)
    //----------------------------------------------------------------------------



    //----------------------------------------------------------------------------
    // -- Update Outputs (LED Displays, Serial, Actuators, etc) -- //
    //----------------------------------------------------------------------------

      //----------------------------------------------------------------------------
      // Update Actuator Outputs
      //----------------------------------------------------------------------------

        // Fan
        //----------------------------------------------------------------------------
        if(fanState)
        {
          if(millis() < micEventFanTimer_ms)           // timer not expired for increased fan speed due to mic event
          {
            analogWrite(PIN_FAN_PWM, MIC_EVENT_FAN_PWM);  // increased fan speed PWM
          }
          else                                          // retun fan to normal speed after mic event duration
          {
            analogWrite(PIN_FAN_PWM, fanSpeedPwm);       // Turn fan ON to normal PWM setting
          }
        }
        else
        {
          analogWrite(PIN_FAN_PWM, 0);                 // Turn fan OFF
        }

        // UV LED Module
        //----------------------------------------------------------------------------
        if(fanIsRunning)    //fanIsRunning is determined from fanTachCount in "Handle ISRs"
        {
          digitalWrite(PIN_UV_LED_RELAY, uvLedState);
        }
        else
        {
          // Safety: Automatically turn UV LEDs OFF if fan isn't running
          // UV LED block may build up heat if fan is not running
          digitalWrite(PIN_UV_LED_RELAY, LOW);
        }



        // Shutter / Stepper
        //----------------------------------------------------------------------------
        if(shutterCurrentState != shutterDesiredState)
        // when current and desired shutter states do not match, actuate stepper here.
        // limit switches are checked at "Poll Inputs" to update shutterCurrentState
        {
          switch(shutterDesiredState)
          {
            case CLOSED:
            //----------------------------------------------------------------------------
            {
              // https://learn.adafruit.com/adafruit-motor-shield-v2-for-arduino/library-reference#uint8-t-onestep-uint8-t-dir-uint8-t-style-2161523-11
              // * onestep() is a low-level internal function (typically used by step())
              // that does not include a built-in delay.
              // * Here, onestep() is used to implement the step actions in a non-blocking manner.
              // * onestep() ignores setSpeed() command
                stepper1->onestep(FORWARD, DOUBLE);
                //stepper1->onestep(FORWARD, SINGLE);

              // * step() function is "blocking", meaning it must finish all of its steps before moving on
              // * Using step() for 1 step includes a delay function set by setSpeed()
                //stepper1->step(1, FORWARD, SINGLE);       // activate 1 coil at a time, less power draw & less torque
                //stepper1->step(1, FORWARD, DOUBLE);         // activate 2 coil at a time, more power draw & more torque
                //stepper1->step(1, FORWARD, INTERLEAVE);   // alternate between SINGLE & DOUBLE, higher resolution but slower speed
                //stepper1->step(1, FORWARD, MICROSTEP);    // PWM coils for smooth motion between steps
            } // END -- CLOSED
            break;

            case OPEN:
            //----------------------------------------------------------------------------
            {
              // https://learn.adafruit.com/adafruit-motor-shield-v2-for-arduino/library-reference#uint8-t-onestep-uint8-t-dir-uint8-t-style-2161523-11
              // * onestep() is a low-level internal function (typically used by step())
              // that does not include a built-in delay.
              // * Here, onestep() is used to implement the step actions in a non-blocking manner.
              // * onestep() ignores setSpeed() command
                stepper1->onestep(BACKWARD, DOUBLE);  //onestep is an internal function called by step(), not recommended
                //stepper1->onestep(BACKWARD, SINGLE);  //onestep is an internal function called by step(), not recommended

              // * step() function is "blocking", meaning it must finish all of its steps before moving on
              // * Using step() for 1 step includes a delay function set by setSpeed()
                //stepper1->step(1, BACKWARD, SINGLE);      // activate 1 coil at a time, less power draw & less torque
                //stepper1->step(1, BACKWARD, DOUBLE);        // activate 2 coil at a time, more power draw & more torque
                //stepper1->step(1, BACKWARD, INTERLEAVE);  // alternate between SINGLE & DOUBLE, higher resolution but slower speed
                //stepper1->step(1, BACKWARD, MICROSTEP);   // PWM coils for smooth motion between steps
            } // END -- OPEN
            break;

            case RELEASED:
            //----------------------------------------------------------------------------
            {
              stepper1->release();
              shutterCurrentState = RELEASED;

              Debug.print(DBG_DEBUG, F("[D] Shutter RELEASED"));
            } // END -- RELEASED
            break;

            default:
            //----------------------------------------------------------------------------
            {
              Debug.print(DBG_WARNING, F("[W] Invalid shutterDesiredState: %s"), shutterStateString[shutterDesiredState]);
              shutterDesiredState = shutterCurrentState;
            } // END -- default
            break;

          } // END switch()

        } // END if Update Shutter State


        // Update LED Outputs
        //----------------------------------------------------------------------------
          sysLed.update();
          uiLed.update();

        // Update Buzzer Outputs
        //----------------------------------------------------------------------------
          //EasyBuzzer.update();


      //----------------------------------------------------------------------------
      // Update NeoPixels
      //----------------------------------------------------------------------------
      #ifdef PIXEL_CONFIG_H   // only include if pixel_config.h is being used

        // PixelStates set directly at code events

        // Local variables reused for each pixels.setPixelColor() command
        // - easier to make generic case statements to copy for each Pixel
        // - must use unique PIXEL number to set each pixels.setPixelColor() at end of switch()
        uint16_t pix;
        uint16_t hue;
        uint8_t sat;
        uint8_t bri;

        // Define display characteristics for each PixelState
        switch(uiPixelScene)
        {
          //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
          case PIX_ALERT:
          {
            hue = RED;
            sat = 255;
            //bri set by sine wave equation below
            uint8_t briMax = BRI_DEFAULT;
            uint8_t briMin = BRI_MIN;

            // full "sine wave" pulse
            int period_ms = 1000;                                           // period of "wave"
            int briTemp = millis()%period_ms;                                // running count of ms from 0 to period_ms

            if(briTemp <= (period_ms/2))                                     // if value is less than half of period duration
            {
              bri = map(briTemp, 0, (period_ms/2), briMin, briMax);   // increasing intensity
            }
            else                                                            // if value is more than half of period duration
            {
              bri = map(briTemp, (period_ms/2), period_ms, briMax, briMin);   // decreasing intensity
            }
          }
            break;
          //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
          //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
          case PIX_OFF:
          {
            hue = 0;   // irrelevant, not shown with Bri = 0
            sat = 0;
            bri = 0;
          }
            break;
          //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
          //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
          case PIX_COUNTDOWN:
          {
            hue = GREEN;
            sat = 255;
            //bri set by sine wave equation below
            uint8_t briMax = BRI_DEFAULT;
            uint8_t briMin = BRI_MIN;

            // full "sine wave" pulse
            int period_ms = 1000;                                             // period of "wave"
            int briTemp = (millis()-sessionStartTimestamp_ms)%period_ms;  // running count of ms from 0 to period_ms
            bri = map(briTemp, 0, period_ms, briMax, briMin);

          }
            break;
          //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
          //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
          case PIX_WAITING:
          {
            hue = AMBER;
            sat = 255;
            //bri set by sine wave equation below
            uint8_t briMax = BRI_DEFAULT;
            //uint8_t briMin = BRI_DEFAULT/2;
            uint8_t briMin = BRI_MIN;

            // full "sine wave" pulse
            int period_ms = 2000;                                           // period of "wave"
            int briTemp = millis()%period_ms;                                // running count of ms from 0 to period_ms

            if(briTemp <= (period_ms/2))                                     // if value is less than half of period duration
            {
              bri = map(briTemp, 0, (period_ms/2), briMin, briMax);   // increasing intensity
            }
            else                                                            // if value is more than half of period duration
            {
              bri = map(briTemp, (period_ms/2), period_ms, briMax, briMin);   // decreasing intensity
            }
          }
            break;
          //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
          //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
          default:
          // catch-all Pixel color for invalid switch(case)
          {
            hue = 0;              // irrelevant for sat = 0
            sat = 0;              // makes any hue WHITE
            bri = BRI_DEFAULT/2;  // dim to draw attention
          }
            break;
          //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        } // END switch()

          // Update Pixel Color
          pix = UI_PIXEL;
          pixels.setPixelColor(pix, pixels.gamma32(pixels.ColorHSV(hue, sat, bri)));

        // END -- Update uiPixelScene
        //----------------------------------------------------------------------------


        // Display all Pixel Updates
        //----------------------------------------------------------------------------
          pixels.show();      // Update strip with new contents


      #endif  // PIXEL_CONFIG_H

      // END -- Update NeoPixels
      //----------------------------------------------------------------------------



      //----------------------------------------------------------------------------
      // Update Loop Statistics
      //----------------------------------------------------------------------------

        // Update Loop Time measurement
          uint32_t loopTime_us = (micros() - loopCompleteTimestamp_us);


        // Update Average Loop Time
          avgLoopTimeSamples_us.push(loopTime_us);           // push loop time into sample array

          if(loopCount >= LOOP_TIME_NUM_SAMPLES)            // if array is full, calculate average
          {
            avgLoopTime_us = avgLoopTimeSamples_us.mean();  // update rolling average
          }

        // Check maxLoopTime and update if it has increased
          if(loopTime_us > maxLoopTime_us)
          {

            maxLoopTime_us = loopTime_us;

            uint16_t tempMaxLoopTime_ms = maxLoopTime_us/1000;

            Debug.print(DBG_DEBUG,    F("\n\r[D] * maxLoopTime: %d ms"), tempMaxLoopTime_ms );

            // Warn if maxLoopTime_ms is approaching WATCHDOG_TIMER_MS
            if(tempMaxLoopTime_ms >= (0.9 * WATCHDOG_TIMER_MS))
            {
              Debug.print(DBG_WARNING,    F("[W] * loopTime is 90% of WATCHDOG_TIMER_MS") );
            }
          }

        // END -- Update Loop Statistics
        //----------------------------------------------------------------------------


      //----------------------------------------------------------------------------
      // Refresh Serial Data Output
      //----------------------------------------------------------------------------


      // END -- Refresh Serial Data Output
      //----------------------------------------------------------------------------


    // END -- Handle Outputs (LED Displays, Serial, Actuators, etc) -- //
    //----------------------------------------------------------------------------


    loopCompleteTimestamp_us = micros();    // leave as LAST LINE of main loop() to accurately calculate loopTime

}   // END == loop()
