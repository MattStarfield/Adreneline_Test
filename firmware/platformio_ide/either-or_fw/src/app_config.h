/*
//==============================================================================
  Project:   CHOP Lab Instrumentation
  Client:    Steven Zderic

  Developer: Matt Starfield (matt.starfield@nextfab.com) | NextFab
  GitHub:    https://github.com/MattStarfield
//==============================================================================
  * Description:
    Define configuration and customizable settings for application firmware in a
    single, easily editable location.
//==============================================================================
*/

#ifndef APP_CONFIG_H
#define APP_CONFIG_H
#include "Arduino.h"                    // includes all AVR & Arduino-specific definfitions (case sensitive file name on Linux)


//==============================================================================
// == App Info == //
//==============================================================================

  #define FIRMWARE_VERSION                  "v0.0.1"
  #define FIRMWARE_NAME                     "Either-Or"

//==============================================================================
// == App Settings == //
//==============================================================================

  // Device Operating Mode
  //----------------------------------------------------------------------------
    /*
      DEVICE_MODE_NOT_SET
      NORMAL_MODE
      FIRST_RUN_MODE
      SETUP_MODE
      DEVELOPMENT_MODE
      TEST_MODE
    */
    #define DEVICE_MODE_DEFAULT               DEVELOPMENT_MODE    // options listed in device_mode_config.h
    //#define DEVICE_MODE_DEFAULT               NORMAL_MODE    // options listed in device_mode_config.h

  // Debug Mode
  //----------------------------------------------------------------------------
    /*
      DBG_NONE - no debug output is shown
      DBG_ERROR - critical errors
      DBG_WARNING - non-critical errors
      DBG_INFO - information
      DBG_DEBUG - more information
      DBG_VERBOSE - most information
    */
    #define DEBUG_LEVEL_DEFAULT                DBG_NONE       // sets the debugLevel prior to FSM_SYS_INIT_DEV
    // Use deviceMode definitions in FSM_SYS_INIT_DEV to set debugLevel for each deviceMode


  // Session Settings
  //----------------------------------------------------------------------------
    #define SESSION_COUNTDOWN_TIME            5     // seconds
    #define SESSION_REPEATS_NUM_OF_OPTIONS    2     // number of options in sessionRepeatsList[]

  // Sensor Settings
  //----------------------------------------------------------------------------

    #define PRESSURE_NUM_SAMPLES              10     // number of Pressure Sensor readings to average over
    #define PRESSURE_MAX_VALUE_KPA            10.0   // kPa - maximum value of pressure sensor range
    #define PRESSURE_MAX_ERROR_KPA            0.5    // kPa - maximu error of pressure sensor reading

  // Actuator Settings
  //----------------------------------------------------------------------------
    #define FAN_PWM_DEFAULT                   255  // (0 - 255) default PWM duty cycle
    #define FAN_PWM_MAX                       255  // (0 - 255) Max PWM duty cycle allowed in software
    #define FAN_PWM_MIN                       70   // (0 - 255) Min PWM duty cycle allowed in software

  // Watchdog Timer Settings
  //----------------------------------------------------------------------------
    #define WATCHDOG_TIMER_MS                 2000 // (ms), watchdog will reboot system if it hangs for this long

//==============================================================================
// == Pin Configuration == //
//==============================================================================

  // Adafruit Feather M0 Express Pinout: https://learn.adafruit.com/assets/96531
  // NOTE: https://learn.adafruit.com/adafruit-feather-m0-express-designed-for-circuit-python-circuitpython/adapting-sketches-to-m0
  //----------------------------------------------------------------------------

    #define PIN_I2C_SCL                       27  // MotorDriver I2C-SCL  //
    #define PIN_I2C_SDA                       26  // MotorDriver I2C-SDA  //

    #define PIN_SPI_SCK                       24  // MusicMaker SPI-SCK  //
    #define PIN_SPI_MOSI                      23  // MusicMaker SPI-MOSI  //
    #define PIN_SPI_MISO                      22  // MusicMaker SPI-MISO  //

    #define PIN_UV_LED_RELAY                  A5  // D19; Analog In  // not used
    #define PIN_UI_LED                        A4  // D18; Analog In; PWM //
    #define PIN_UI_BUTTON                     A3  // D17; Analog In; PWM //
    #define PIN_LIMIT_SWITCH2                 A2  // D15; Analog In  //
    #define PIN_LIMIT_SWITCH1                 A1  // D14; Analog In  //
    #define PIN_PIR_SENSOR                    A0  // DAC; Analog In    //

    #define PIN_SYS_LED                       13  // OB-LED; PWM  //
    #define PIN_FAN_TACH                      12  // PWM          //
    #define PIN_FAN_PWM                       11  // PWM          //
    #define PIN_VS1053_DSEL                   10  // MusicMaker XD-CS; PWM  //
    #define PIN_VS1053_DREQ                    9  // MusicMaker DREQ; PWM; A7_VBAT_Read //
    #define PIN_SPI_VS1053_CS                  6  // MusicMaker MP3-CS; PWM  //
    #define PIN_SPI_SD_CS                      5  // MusicMaker SD-CS; PWM  //

    //#define PIN_                             1  // TX1; PWM//
    //#define PIN_                             0  // RX0 (avoid use) //

    // Special Pin Defines for Feather M0 Express
    //--------------------------------------------------------------------------
    #define PIN_VS1053_RESET                  -1  // VS1053 reset pin (not used!)

    //#define PIN_VBAT                          A7  // Shared with D9

    #define PIN_PIXEL                          8  // Onboard NeoPixel, No GPIO Pin

    // requires Adafruit_SPIFlash Lib: https://github.com/adafruit/Adafruit_SPIFlash
    #define PIN_FLASH_SCK                      3  // SPI1; Onboard 2MB Flash Memory, No GPIO Pin
    #define PIN_FLASH_MISO                     2  // SPI1; Onboard 2MB Flash Memory, No GPIO Pin
    #define PIN_FLASH_MOSI                     4  // SPI1; Onboard 2MB Flash Memory, No GPIO Pin
    #define PIN_FLASH_CS                      38  // SPI1; Onboard 2MB Flash Memory, No GPIO Pin


//==============================================================================
// == Peripheral Settings == //
//==============================================================================

  // Serial Settings
  //----------------------------------------------------------------------------
    #define SERIAL_BAUD_RATE                  115200    // (bps); e.g. 9600, 19200, 57600, 115200

  // EasyButton Settings
  //----------------------------------------------------------------------------
    #define BTN_HOLD_DURATION_MS              3000    // duration for onPressedFor
    #define BTN_SEQ_NUM_CLICKS                2       // number of clicks for onSequence
    #define BTN_SEQ_TIMEOUT_MS                (BTN_SEQ_NUM_CLICKS*200)    // timeout window for onSequence

  // EasyLed Settings
  //----------------------------------------------------------------------------

    //          led.blink(    onFor_ms, offFor_ms,  cycles,  pauseFor_ms,  numOfSequeces,  onFinished_cb() );
    #define LED_HEARTBEAT     30,       200,        2,        1000,         0,              NULL
    #define LED_ATTENTION     50,       100,        5,        500,          0,              NULL
    #define LED_STABILIZING   100,      150,        2,        0,            0,              NULL
    #define LED_ERROR         500,      500,        2,        0,            0,              NULL
    #define LED_ACTIVITY      100,      100,        1,        10,           1,              NULL

  // EasyBuzzer Settings
  //----------------------------------------------------------------------------
    // usable frequency range: ~500 to ~12,000 Hz,
    // EasyBuzzer.beep(      frequency, onFor_ms, offFor_ms,  cycles, pauseFor_ms, numOfSequeces, onFinished_cb() );
    #define BEEP_OK           4000,     50,       0,          1,      10,           1,             NULL
    #define BEEP_ATTENTION    9000,     50,       100,        4,      10,           1,             NULL
    #define BEEP_CHIRP        10000,    15,       0,          1,      10,           1,             NULL
    #define BEEP_ERROR        500,      500,      500,        2,      1000,         1,             NULL
    #define BEEP_WARNING
    #define BEEP_HEARTBEAT

  // NeoPixel Settings
  //----------------------------------------------------------------------------
    // defined in pixel_config.h

  // Potentiometer Settings
  //----------------------------------------------------------------------------
    //#define POT_TOLERANCE                  10
    //#define POT_NUM_SAMPLES                5


  // Sensor Settings
  //----------------------------------------------------------------------------
    #define PIR_STABILIZATION_TIME_MS         30000 // ms, time required for PIR sensor to stabilize after power up
    #define PIR_RESET_TIME_MS                 6000  // ms, time required for PIR to be ready after motion is no longer detected

  // Actuator Settings
  //----------------------------------------------------------------------------
    #define FAN_PWM_DEFAULT                   255  // (0 - 255) default PWM duty cycle
    #define FAN_PWM_MAX                       255  // (0 - 255) Max PWM duty cycle allowed in software
    #define FAN_PWM_MIN                       50   // (0 - 255) Min PWM duty cycle allowed in software
    #define FAN_TIMEOUT_MS                    2000  // (2 sec min) amount of time allowed for fan to start spinning before error is issued
    #define FAN_CHECK_INTERVAL_MS             100  // period to wait between checking if fan is running

//==============================================================================
// == Other Macros == //
//==============================================================================

  #define ENUM_STRING_BUFFER_SIZE           64    // length of longest enum string; used for PROGMEM print

  #define LOOP_TIME_NUM_SAMPLES             10

  #define WAIT_MESSAGE_REFRESH_INTERVAL_MS  5000
  #define WAIT_MESSAGE_PRINT_PERIODICALLY   true // determines if FSM_SYS_WAIT message prints once or refreshes periodically

  #define CLI_INPUT_DESIGNATOR              "\n\r<< "
  #define CLI_OUTPUT_DESIGNATOR             "\n\r>> "

  // LED & Pixel PWM Brightness values
  //----------------------------------------------------------------------------
    #define BRI_MAX                           255    // (0 - 255) Max allowable brightness
    #define BRI_DEFAULT                       200
    #define BRI_MIN                           50     // (0 - 255) Min visible brightness


  // Active Low Logic
  //----------------------------------------------------------------------------
    #define AL_DISABLE                        true
    #define AL_ENABLE                         false


    // Data Types Value Ranges
    //------------
    //#define INT16_MAX                         SHRT_MAX   // int           // 2 bytes //  32,768
    //#define INT16_MIN                         SHRT_MIN   // int           // 2 bytes // -32,768

    //#define UINT16_MAX                        USHRT_MAX  // unsigned int  // 2 bytes //  65,535
    //#define UINT16_MIN                        0          // unsigned int  // 2 bytes //  0

    //#define INT32_MAX                         INT_MAX    // long          // 4 bytes //  2,147,483,647
    //#define INT32_MIN                         INT_MIN    // long          // 4 bytes // -2,147,483,648

    //#define UINT32_MAX                        UINT_MAX   // unsigned long // 4 bytes // 4294967295
    //#define UINT32_MIN                        0          // unsigned long // 4 bytes // 0

    //#define FLOAT_MAX                         FLT_MAX    // float         // 4 bytes  //  3.4028235E+38
    //#define FLOAT_MIN                         FLT_MIN    // float         // 4 bytes  // -3.4028235E+38

//==============================================================================
// == Debug Print Helper Functions == //
//==============================================================================

  // Must be #define inline function to preserve intended use of function name and line number keywords
  #define DEBUG_PRINT_FUNCTION_LINE Debug.print(DBG_DEBUG, "\n\r"); Debug.print(DBG_DEBUG, "[D] [%s(): L#%d]", __func__, __LINE__)

  // Enum Strings stored in PROGMEM
  //----------------------------------------------------------------------------
    // const char stringName [NUMBER_OF_ELEMENTS] [MAX_SIZE] PROGMEM = {
    // http://www.gammon.com.au/forum/?id=12615
    const char debugLevelString [6] [ENUM_STRING_BUFFER_SIZE] PROGMEM =
    {
      {"DBG_NONE"},
      {"DBG_ERROR"},
      {"DBG_WARNING"},
      {"DBG_INFO"},
      {"DBG_DEBUG"},
      {"DBG_VERBOSE"},
    };  // END -- debugLevelString[][]

#endif  // END -- APP_CONFIG_H
