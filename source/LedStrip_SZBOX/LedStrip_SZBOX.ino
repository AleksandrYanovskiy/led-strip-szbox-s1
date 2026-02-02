// LedStrip_SZBOX
//
//  ####################################
//
//  v1.00 - 01.2016 - adding a project
//
//  ####################################
// After the original LED strip stopped working, this program was written to control it.
// This sketch allows you to restore the LED backlight of the mini PC SZBOX S1 (ACEMAGIC S1).
// The program runs on Digispark Attiny85 and Arduino UNO.
//
// By default, the Serial port is enabled.
// This allows you to program effects onto the LED strip and read debug data from Digispark.
// If Serial port not used, you need to comment out the line with:
// #define UART_SERIAL_ENABLE
// An attempt was made to create original mini PC effects. New ones were also added.
//
// The following Digispark ports are used:
// PD1  ==> Out to LED strip
// PB2  ==> RX signal from Serial port 
// PB0  ==> TX signal to Serial port
//
// Sketch allows almost any RGB effects, with a description of the steps in an EFFECT ARRAYS DATA.
// The bits in each byte of the array are represented by the colors of a single RGB led.
// Bits 0-1 represent blue, 2-3 represent green, and 4-5 represent red. Bits 6-7 are unused.
// The value of each pair of color bits represents the brightness.
// 00 means brightness is off.
// 01 means brightness is 50% of maximum.
// 10 means brightness is 100%.
// Value 11 is unused.
// The LED strip uses six of these LEDs.
//
// The transition from one color phase to another phase occurs gradually, in each main cycle of the program, until the desired brightness value is reached.
// Also, in each cycle, a poll is made of the Serial port (if enabled) for the command to change the LED strip mode.
//
// If the Serial port is disabled, the program cycles through each effect in turn.
// It is possible to force any effect to be specified via a value DEFAULT_THEME, which will be constantly displayed in a circle.
// Of course, not all modes, brightness, and speed are exactly the same as the original.
// But it's better than a completely non-functional LED strip.
//
// The serial port speed is set to 4800 by default, as the most stable.
// The LED control commands are as follows (thanks to the author of the project https://github.com/tjaworski/AceMagic-S1-LED-TFT-Linux):
//  COMMAND:
//    struct led_command {
//        uint8_t signature; // 0xfa
//        uint8_t theme;     
//        uint8_t intensity;
//        uint8_t speed; 
//        uint8_t checksum;
//    };
//
//  THEME:
//    0x01 = rainbow
//    0x02 = breathing
//    0x03 = color cycle
//    0x04 = off
//    0x05 = automatic    (by default, it loops through all the themes)
//
//    ** additional themes **
//    0x06 = red alert
//    0x07 = running fire
//    0x08 = police siren
//    0x09 = candle
//
//  INTENSITY:
//    0x01 = level 5
//    0x02 = level 4
//    0x03 = level 3
//    0x04 = level 2
//    0x05 = level 1
//
//  SPEED:
//    0x01 = level 5
//    0x02 = level 4
//    0x03 = level 3
//    0x04 = level 2
//    0x05 = level 1
//
//  CHECKSUM:
//    CHS = byte(signature + theme + intensity + speed)
//
// Sketch programming is different from regular Arduino boards.


#include <Arduino.h>

//comment out the following line if you will not be using the UART control theme setting.
#define UART_SERIAL_ENABLE

#ifdef ARDUINO_AVR_UNO //Arduino UNU with WS2812B led
  //data pin to LED
  #define PIN_LED_OUT PD2

  //type RGB LED
  #define LED_COLOR NEO_GRB

  #ifdef UART_SERIAL_ENABLE
    //serial settings
    #define SERIAL_SPEED 4800  //stable serial connection speed. 9600 speed also works, but it's not stable.
    HardwareSerial& softSerial = Serial;
  #endif  

#else //Arduino Digispark
  //data pin to LED
  #define PIN_LED_OUT PB1

  //type RGB LED
  #define LED_COLOR NEO_GRB

  //serial settings
  #define SERIAL_RX PB2      //pin for read data
  #define SERIAL_TX PB0      //pin for transmin data
  #define SERIAL_SPEED 4800  //stable serial connection speed. 9600 speed also works, but it's not stable.
                                
  #ifdef UART_SERIAL_ENABLE
    #include <SoftwareSerial.h>
    SoftwareSerial softSerial(SERIAL_RX, SERIAL_TX); // RX, TX
  #endif

#endif

//##########################
//by default - all modes periodically
#define DEFAULT_THEME 5        //default theme after power on
#define FIRST_DEFAULT_THEME 1  //first theme for cycle mode

#define LED_COUNT 6
#define LAST_THEME 9  //last theme number for scroll all theme in auto mode

// Maximum bright of LED
#define MAX_BRIGHT 80
#define BRIGHT_K 16       //only even values!!!
#define DEFAULT_BRIGHT 3
#define MAX_INTENSITY 5

// loop interval delay
#define DELAY_INTERV 10
#define SPEED_K 15
#define DEFAULT_SPEED 3
#define MAX_SPEED 5

//##########################
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(LED_COUNT, PIN_LED_OUT, LED_COLOR + NEO_KHZ800);


//####################################################
// EFFECT ARRAYS DATA
// xxRRGGBB - byte for one led
// 00 - 0% bright
// 01 - 50% bright
// 10 - 100% bright
// 11 - reserved

const uint8_t dataRainbow_01[][LED_COUNT] PROGMEM = {
          { //00 - phase - red-violet
            0b00100000, //1 led-red
            0b00100000, //2 led-red
            0b00100000, //3 led-red
            0b00100010, //4 led-violet
            0b00100010, //5 led-violet
            0b00100010, //6 led-violet
          },
          { //02 - phase - yellow-red-violet
            0b00101000, //1 led-yellow
            0b00100000, //2 led-red
            0b00100000, //3 led-red
            0b00100000, //4 led-red
            0b00100010, //5 led-violet
            0b00100010, //6 led-violet
          },
          { //03 - phase - yellow-red-violet
            0b00101000, //1 led-yellow
            0b00101000, //2 led-yellow
            0b00100000, //3 led-red
            0b00100000, //4 led-red
            0b00100000, //5 led-red
            0b00100010, //6 led-violet
          },
          { //04 - phase - yellow-red
            0b00101000, //1 led-yellow
            0b00101000, //2 led-yellow
            0b00101000, //3 led-yellow
            0b00100000, //4 led-red
            0b00100000, //5 led-red
            0b00100000, //6 led-red
          },
          { //05 - phase - green-yellow-red
            0b00001000, //1 led-green
            0b00101000, //2 led-yellow
            0b00101000, //3 led-yellow
            0b00101000, //4 led-yellow
            0b00100000, //5 led-red
            0b00100000, //6 led-red
          },
          { //06 - phase - green-yellow-red
            0b00001000, //1 led-green
            0b00001000, //2 led-green
            0b00101000, //3 led-yellow
            0b00101000, //4 led-yellow
            0b00101000, //5 led-yellow
            0b00100000, //6 led-red
          },
          { //07 - phase - green-yellow
            0b00001000, //1 led-green
            0b00001000, //2 led-green
            0b00001000, //3 led-green
            0b00101000, //4 led-yellow
            0b00101000, //5 led-yellow
            0b00101000, //6 led-yellow
          },
          { //08 - phase - blue-green-yellow
            0b00000010, //1 led-blue
            0b00001000, //2 led-green
            0b00001000, //3 led-green
            0b00001000, //4 led-green
            0b00101000, //5 led-yellow
            0b00101000, //6 led-yellow
          },
          { //09 - phase - blue-green-yellow
            0b00000010, //1 led-blue
            0b00000010, //2 led-blue
            0b00001000, //3 led-green
            0b00001000, //4 led-green
            0b00001000, //5 led-green
            0b00101000, //6 led-yellow
          },
          { //10 - phase - blue-green
            0b00000010, //1 led-blue
            0b00000010, //2 led-blue
            0b00000010, //3 led-blue
            0b00001000, //4 led-green
            0b00001000, //5 led-green
            0b00001000, //6 led-green
          },
          { //11 - phase - violet-blue-green
            0b00100010, //1 led-violet
            0b00000010, //2 led-blue
            0b00000010, //3 led-blue
            0b00000010, //4 led-blue
            0b00001000, //5 led-green
            0b00001000, //6 led-green
          },
          { //12 - phase - violet-blue-green
            0b00100010, //1 led-violet
            0b00100010, //2 led-violet
            0b00000010, //3 led-blue
            0b00000010, //4 led-blue
            0b00000010, //5 led-blue
            0b00001000, //6 led-green
          },
          { //13 - phase - violet-blue
            0b00100010, //1 led-violet
            0b00100010, //2 led-violet
            0b00100010, //3 led-violet
            0b00000010, //4 led-blue
            0b00000010, //5 led-blue
            0b00000010, //6 led-blue
          },
          { //14 - phase - red-violet-blue
            0b00100000, //1 led-red
            0b00100010, //2 led-violet
            0b00100010, //3 led-violet
            0b00100010, //4 led-violet
            0b00000010, //5 led-blue
            0b00000010, //6 led-blue
          },
          { //15 - phase - red-violet-blue
            0b00100000, //1 led-red
            0b00100000, //2 led-red
            0b00100010, //3 led-violet
            0b00100010, //4 led-violet
            0b00100010, //5 led-violet
            0b00000010, //6 led-blue
          },
};

const uint8_t dataBreathing_02[][LED_COUNT] PROGMEM = {
          { //00 - phase - all off
            0b00000000, //1 led
            0b00000000, //2 led
            0b00000000, //3 led
            0b00000000, //4 led
            0b00000000, //5 led
            0b00000000, //6 led
          },
          { //01 - phase - red up
            0b00100000, //1 led
            0b00100000, //2 led
            0b00100000, //3 led
            0b00100000, //4 led
            0b00100000, //5 led
            0b00100000, //6 led
          },
          { //02 - phase - red down
            0b00000000, //1 led
            0b00000000, //2 led
            0b00000000, //3 led
            0b00000000, //4 led
            0b00000000, //5 led
            0b00000000, //6 led
          },
          { //03 - phase - orange up
            0b00100100, //1 led
            0b00100100, //2 led
            0b00100100, //3 led
            0b00100100, //4 led
            0b00100100, //5 led
            0b00100100, //6 led
          },
          { //04 - phase - orange down
            0b00000000, //1 led
            0b00000000, //2 led
            0b00000000, //3 led
            0b00000000, //4 led
            0b00000000, //5 led
            0b00000000, //6 led
          },
          { //05 - phase - yellow up
            0b00101000, //1 led
            0b00101000, //2 led
            0b00101000, //3 led
            0b00101000, //4 led
            0b00101000, //5 led
            0b00101000, //6 led
          },
          { //06 - phase - yellow down
            0b00000000, //1 led
            0b00000000, //2 led
            0b00000000, //3 led
            0b00000000, //4 led
            0b00000000, //5 led
            0b00000000, //6 led
          },
          { //07 - phase - green up
            0b00001000, //1 led
            0b00001000, //2 led
            0b00001000, //3 led
            0b00001000, //4 led
            0b00001000, //5 led
            0b00001000, //6 led
          },
          { //08 - phase - green down
            0b00000000, //1 led
            0b00000000, //2 led
            0b00000000, //3 led
            0b00000000, //4 led
            0b00000000, //5 led
            0b00000000, //6 led
          },
          { //09 - phase - blue up
            0b00000010, //1 led
            0b00000010, //2 led
            0b00000010, //3 led
            0b00000010, //4 led
            0b00000010, //5 led
            0b00000010, //6 led
          },
          { //10 - phase - blue down
            0b00000000, //1 led
            0b00000000, //2 led
            0b00000000, //3 led
            0b00000000, //4 led
            0b00000000, //5 led
            0b00000000, //6 led
          },
          { //11 - phase - cyan up
            0b00001001, //1 led
            0b00001001, //2 led
            0b00001001, //3 led
            0b00001001, //4 led
            0b00001001, //5 led
            0b00001001, //6 led
          },
          { //12 - phase - cyan down
            0b00000000, //1 led
            0b00000000, //2 led
            0b00000000, //3 led
            0b00000000, //4 led
            0b00000000, //5 led
            0b00000000, //6 led
          },
          { //13 - phase - violet up
            0b00100010, //1 led
            0b00100010, //2 led
            0b00100010, //3 led
            0b00100010, //4 led
            0b00100010, //5 led
            0b00100010, //6 led
          },
          { //14 - phase - violet down
            0b00000000, //1 led
            0b00000000, //2 led
            0b00000000, //3 led
            0b00000000, //4 led
            0b00000000, //5 led
            0b00000000, //6 led
          },

};

const uint8_t dataColorCycle_03[][LED_COUNT] PROGMEM = {
          { //00 - phase - red up
            0b00100000, //1 led
            0b00100000, //2 led
            0b00100000, //3 led
            0b00100000, //4 led
            0b00100000, //5 led
            0b00100000, //6 led
          },
          { //01 - phase - orange up
            0b00100100, //1 led
            0b00100100, //2 led
            0b00100100, //3 led
            0b00100100, //4 led
            0b00100100, //5 led
            0b00100100, //6 led
          },
          { //02 - phase - yellow up
            0b00101000, //1 led
            0b00101000, //2 led
            0b00101000, //3 led
            0b00101000, //4 led
            0b00101000, //5 led
            0b00101000, //6 led
          },
          { //03 - phase - green up
            0b00001000, //1 led
            0b00001000, //2 led
            0b00001000, //3 led
            0b00001000, //4 led
            0b00001000, //5 led
            0b00001000, //6 led
          },
          { //04 - phase - blue up
            0b00000010, //1 led
            0b00000010, //2 led
            0b00000010, //3 led
            0b00000010, //4 led
            0b00000010, //5 led
            0b00000010, //6 led
          },
          { //05 - phase - cyan up
            0b00001001, //1 led
            0b00001001, //2 led
            0b00001001, //3 led
            0b00001001, //4 led
            0b00001001, //5 led
            0b00001001, //6 led
          },
          { //06 - phase - violet up
            0b00100010, //1 led
            0b00100010, //2 led
            0b00100010, //3 led
            0b00100010, //4 led
            0b00100010, //5 led
            0b00100010, //6 led
          },
};

const uint8_t dataLedOff_04[][LED_COUNT] PROGMEM = {
          { //01 - phase - all off
            0b00000000, //1 led
            0b00000000, //2 led
            0b00000000, //3 led
            0b00000000, //4 led
            0b00000000, //5 led
            0b00000000, //6 led
          },
};

//additional effects
const uint8_t dataRedAlert_06[][LED_COUNT] PROGMEM = {
          { //00 - phase - all off
            0b00000000, //1
            0b00000000, //2
            0b00000000, //3 
            0b00000000, //4
            0b00000000, //5
            0b00000000, //6
          },
          { //01 - phase - red min
            0b00000000, //1
            0b00000000, //2
            0b00100000, //3 
            0b00100000, //4
            0b00000000, //5
            0b00000000, //6
          },
          { //02 - phase - red maximum
            0b00100000, //1
            0b00100000, //2
            0b00100000, //3 
            0b00100000, //4
            0b00100000, //5
            0b00100000, //6
          },
          { //03 - phase - red min
            0b00000000, //1
            0b00000000, //2
            0b00100000, //3 
            0b00100000, //4
            0b00000000, //5
            0b00000000, //6
          },
};

const uint8_t dataRunningFire_07[][LED_COUNT] PROGMEM = {
          { //00 - phase - all off
            0b00000000, //1
            0b00000000, //2
            0b00000000, //3 
            0b00000000, //4
            0b00000000, //5
            0b00000000, //6
          },
          { //01 - phase - red 1,2-led on
            0b00100000, //1
            0b00100000, //2
            0b00000000, //3 
            0b00000000, //4
            0b00000000, //5
            0b00000000, //6
          },
          { //03 - phase - all off
            0b00000000, //1
            0b00000000, //2
            0b00000000, //3 
            0b00000000, //4
            0b00000000, //5
            0b00000000, //6
          },
          { //04 - phase - red 3,4-led on
            0b00000000, //1
            0b00000000, //2
            0b00100000, //3 
            0b00100000, //4
            0b00000000, //5
            0b00000000, //6
          },
          { //05 - phase - all off
            0b00000000, //1
            0b00000000, //2
            0b00000000, //3 
            0b00000000, //4
            0b00000000, //5
            0b00000000, //6
          },
          { //06 - phase - red 5,6-led on
            0b00000000, //1
            0b00000000, //2
            0b00000000, //3 
            0b00000000, //4
            0b00100000, //5
            0b00100000, //6
          },
};

const uint8_t dataPoliceSiren_08[][LED_COUNT] PROGMEM = {
          { //00 - phase - 3 red on
            0b00100000, //1
            0b00100000, //2
            0b00100000, //3 
            0b00000000, //4
            0b00000000, //5
            0b00000000, //6
          },
          { //01 - phase - 3 blue on
            0b00000000, //1
            0b00000000, //2
            0b00000000, //3 
            0b00000010, //4
            0b00000010, //5
            0b00000010, //6
          },
          { //02 - phase - 3 red on
            0b00100000, //1
            0b00100000, //2
            0b00100000, //3 
            0b00000000, //4
            0b00000000, //5
            0b00000000, //6
          },
          { //03 - phase - 3 blue on
            0b00000000, //1
            0b00000000, //2
            0b00000000, //3 
            0b00000010, //4
            0b00000010, //5
            0b00000010, //6
          },
          { //04 - phase - 3 red on
            0b00100000, //1
            0b00100000, //2
            0b00100000, //3 
            0b00000000, //4
            0b00000000, //5
            0b00000000, //6
          },
          { //05 - phase - 3 blue on
            0b00000000, //1
            0b00000000, //2
            0b00000000, //3 
            0b00000010, //4
            0b00000010, //5
            0b00000010, //6
          },
};

const uint8_t dataCandle_09[][LED_COUNT] PROGMEM = {
          { //00 - phase - red on
            0b00000000, //1
            0b00000000, //2
            0b00000000, //3
            0b00000000, //4
            0b00000000, //5
            0b00100000, //6
          },
          { //01 - phase - red on
            0b00000000, //1
            0b00000000, //2
            0b00000000, //3
            0b00000000, //4
            0b00100000, //5
            0b00100000, //6
          },
          { //02 - phase - red on
            0b00000000, //1
            0b00000000, //2
            0b00000000, //3
            0b00100000, //4
            0b00100000, //5
            0b00100000, //6
          },

          { //03 - phase - orange-red on
            0b00000000, //1
            0b00000000, //2
            0b00100100, //3
            0b00100100, //4
            0b00100000, //5
            0b00100000, //6
          },
          { //04 - phase - yellow-orange-red on
            0b00101000, //1
            0b00101000, //2
            0b00100100, //3
            0b00100100, //4
            0b00100000, //5
            0b00100000, //6
          },
          { //05 - phase - orange-red-red on
            0b00100100, //1
            0b00100100, //2
            0b00100000, //3
            0b00100000, //4
            0b00100000, //5
            0b00100000, //6
          },
          { //06 - phase - yellow-orange-red on
            0b00101000, //1
            0b00101000, //2
            0b00100100, //3
            0b00100100, //4
            0b00100000, //5
            0b00100000, //6
          },
          { //07 - phase - yellow-yellow-orange-red on
            0b00101000, //1
            0b00101000, //2
            0b00101000, //3
            0b00101000, //4
            0b00100100, //5
            0b00100000, //6
          },
          { //08 - phase - yellow-orange-red on
            0b00000000, //1
            0b00101000, //2
            0b00101000, //3
            0b00100100, //4
            0b00100100, //5
            0b00100000, //6
          },
          { //09 - phase - orange-red on
            0b00000000, //1
            0b00000000, //2
            0b00101000, //3
            0b00100100, //4
            0b00100100, //5
            0b00100000, //6
          },
          { //10 - phase - orange-red on
            0b00000000, //1
            0b00000000, //2
            0b00000000, //3
            0b00000000, //4
            0b00100100, //5
            0b00100000, //6
          },
          { //11 - phase - orange-red on
            0b00000000, //1
            0b00000000, //2
            0b00000000, //3
            0b00000000, //4
            0b00000000, //5
            0b00100000, //6
          },
};

//####################################################

bool runLedEffect(const uint8_t theme, const uint8_t intensityEff);
void initEffect();
bool readSerialData();

struct LedCommandStr {
    uint8_t signature;                  // 0xfa
    uint8_t theme = DEFAULT_THEME;      // 0x01..0x05
    uint8_t intensity = DEFAULT_BRIGHT; // 0x01..0x05  level_5-level_1
    uint8_t speed = DEFAULT_SPEED;      // 0x01..0x05  level_5-level_1
    uint8_t checksum;                   // uint8_t (signature + theme + intensity + speed)
};
LedCommandStr _ledCommand;
const int _sizeLedCommand = sizeof(LedCommandStr);

struct ParamEffectStr {
  uint32_t delayEff;
  uint8_t intensityEff;
  uint8_t theme;
  uint8_t currTheme;

};
ParamEffectStr _paramEffect;


//####################################################
/**
 * @brief setup
 * 
 */
void setup() {
  pixels.begin(); // This initializes the NeoPixel library.
  pixels.show();

  #ifdef UART_SERIAL_ENABLE
    softSerial.begin(SERIAL_SPEED);
  #endif

  initEffect();
}

// the main loop function
void loop() {
  static uint8_t _lastEff = 0;

  #ifdef UART_SERIAL_ENABLE
    //read mode from UART
    if (softSerial.available() >= _sizeLedCommand) {
      if (readSerialData()) {
        initEffect();
        runLedEffect(4, _paramEffect.intensityEff); //led off
      }
    }
  #endif

  static uint32_t prevMills = 0;
  uint32_t currMills = millis();
  
  if (currMills - prevMills > _paramEffect.delayEff) {
    prevMills = millis();

    //mode selector
    switch (_paramEffect.theme) {
      case 5: //default mode - complete all themes
        if (_paramEffect.currTheme != _lastEff) {
          runLedEffect(4, _paramEffect.intensityEff);//led off
          _lastEff = _paramEffect.currTheme;
        }
        else if (runLedEffect(_paramEffect.currTheme, _paramEffect.intensityEff)) {
          _paramEffect.currTheme++;
          if (_paramEffect.currTheme > LAST_THEME) {
            _paramEffect.currTheme = FIRST_DEFAULT_THEME;
          }
        }
        break;
    default:
      //selected theme
      runLedEffect(_paramEffect.theme, _paramEffect.intensityEff);
      break;
    }
  }
}

/**
 * @brief The main routine for showing effect on the LED strip
 * 
 * @param theme 
 * @param intensityEff 
 * @return true | false  - returns true if the effect phase is passed.
 */
bool runLedEffect(const uint8_t theme, const uint8_t intensityEff) {
  static uint8_t _currTheme = 0;
  static const uint8_t (*_dataEff)[LED_COUNT];
  static uint8_t _dataSize;
  static uint8_t _phaseCnt;
  static uint8_t _phaseCntPrev;
  static uint32_t _isLedPhaseEnd; //value of end phase for all leds and colors. 6led * 3rgb 18bit phase_end in 32bit variable - xxxxxxxx xxxxxx|rg b|rgb|rgb|r gb|rgb|rgb
  static bool _isPhaseEnd;
  bool isEndEffect = false;

  struct RGB {
    uint8_t R;
    uint8_t G;
    uint8_t B;
  };
  struct ledPhaseRGB {
    RGB currRGB;
    RGB destRGB;
    int8_t stepR;
    int8_t stepG;
    int8_t stepB;
  };
  static ledPhaseRGB _dataLedPhase[LED_COUNT];

  //new mode
  if ( _currTheme != theme ) {    
    _currTheme = theme;
    _phaseCnt = 0;
    _isPhaseEnd = true;

    //init data effect
    switch (theme) {
      case 1: //Rainbow
        _dataEff = dataRainbow_01;
        _dataSize = sizeof(dataRainbow_01) / sizeof(dataRainbow_01[0]);
        break;
      case 2: //Breathing
        _dataEff = dataBreathing_02;
        _dataSize = sizeof(dataBreathing_02) / sizeof(dataBreathing_02[0]);
        break;
      case 3: //ColorCycle
        _dataEff = dataColorCycle_03;
        _dataSize = sizeof(dataColorCycle_03) / sizeof(dataColorCycle_03[0]);
        break;
      case 4: //LedOff
        _dataEff = dataLedOff_04;
        _dataSize = sizeof(dataLedOff_04) / sizeof(dataLedOff_04[0]);
        break;
      case 6: //RedAlert
        _dataEff = dataRedAlert_06;
        _dataSize = sizeof(dataRedAlert_06) / sizeof(dataRedAlert_06[0]);
        break;
      case 7: //RunningFire
        _dataEff = dataRunningFire_07;
        _dataSize = sizeof(dataRunningFire_07) / sizeof(dataRunningFire_07[0]);
        break;
      case 8: //dataPoliceSiren
        _dataEff = dataPoliceSiren_08;
        _dataSize = sizeof(dataPoliceSiren_08) / sizeof(dataPoliceSiren_08[0]);
        break;
      case 9: //Candle
        _dataEff = dataCandle_09;
        _dataSize = sizeof(dataCandle_09) / sizeof(dataCandle_09[0]);
        break;
      /*
      //additional themes can be added here
      case N
        _dataEff = dataXXXX_07;
        _dataSize = sizeof(dataXXXX_07) / sizeof(dataXXXX_07[0]);
        break
      */
      default:
        return true;
        break;
    }
  }

  //when phase change
  if (_isPhaseEnd) {
    _isPhaseEnd = false;
    _isLedPhaseEnd = 0L;
    _phaseCnt++;

    if (_phaseCnt == _dataSize) {
      _phaseCnt = 0;
      isEndEffect = true;
      _phaseCntPrev = _dataSize - 1;
    }
    else {
      _phaseCntPrev = _phaseCnt - 1;
    }

    for (uint8_t i = 0; i < LED_COUNT; i++) {
      //set current RGB
      uint8_t ledColor = pgm_read_byte(&_dataEff[_phaseCntPrev][i]);

      //check for reserved 0b11 value for current data RGB
      if (!((ledColor & (ledColor >> 1)) & 0b00010101) == 0) {
        ledColor = 0; //if an error value is detected, set the value to zero.
      }

      _dataLedPhase[i].currRGB.R = ((ledColor & 0b00110000) >> 4) * (intensityEff >> 1);
      _dataLedPhase[i].currRGB.G = ((ledColor & 0b00001100) >> 2) * (intensityEff >> 1);
      _dataLedPhase[i].currRGB.B = (ledColor & 0b00000011) * (intensityEff >> 1);

      //set destination RGB
      ledColor = pgm_read_byte(&_dataEff[_phaseCnt][i]);

      //check for reserved 0b11 value for destination data RGB
      if (!((ledColor & (ledColor >> 1)) & 0b00010101) == 0) {
        ledColor = 0; //if an error value is detected, set the value to zero.
      }
      _dataLedPhase[i].destRGB.R = ((ledColor & 0b00110000) >> 4) * (intensityEff >> 1);
      _dataLedPhase[i].destRGB.G = ((ledColor & 0b00001100) >> 2) * (intensityEff >> 1);
      _dataLedPhase[i].destRGB.B = (ledColor & 0b00000011) * (intensityEff >> 1);

      //set step value for every RGB led
      _dataLedPhase[i].stepR = ((_dataLedPhase[i].destRGB.R - _dataLedPhase[i].currRGB.R) << 1) / intensityEff;
      _dataLedPhase[i].stepG = ((_dataLedPhase[i].destRGB.G - _dataLedPhase[i].currRGB.G) << 1) / intensityEff;
      _dataLedPhase[i].stepB = ((_dataLedPhase[i].destRGB.B - _dataLedPhase[i].currRGB.B) << 1) / intensityEff;
    }
  }

  //set led strip colors
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    uint32_t resColor;

    resColor = pixels.Color(_dataLedPhase[i].currRGB.R,
                            _dataLedPhase[i].currRGB.G,
                            _dataLedPhase[i].currRGB.B
    );
    pixels.fill(resColor, i, 1);
    pixels.show();

    //increment|decrement color bright for effect
    if (_dataLedPhase[i].currRGB.R != _dataLedPhase[i].destRGB.R) {
      _dataLedPhase[i].currRGB.R += _dataLedPhase[i].stepR;
    }
    else {
      _isLedPhaseEnd |= (1UL << (i * 3 + 2));
    }

    if (_dataLedPhase[i].currRGB.G != _dataLedPhase[i].destRGB.G) {
      _dataLedPhase[i].currRGB.G += _dataLedPhase[i].stepG;
    }
    else {
      _isLedPhaseEnd |= (1UL << (i * 3 + 1));
    }

    if (_dataLedPhase[i].currRGB.B != _dataLedPhase[i].destRGB.B) {
      _dataLedPhase[i].currRGB.B += _dataLedPhase[i].stepB;
    }
    else {
      _isLedPhaseEnd |= ((1UL << i * 3));
    }

    /*
    //debug info
    softSerial.print(F("_isLedPhaseEnd=0x"));
    softSerial.println(_isLedPhaseEnd, HEX);
    */
  }

  //check phase end
  if ( _isLedPhaseEnd == 0x3FFFF) {
      _isPhaseEnd = true;
  }

  return isEndEffect;
}

/**
 * @brief set values for effect
 * 
 */
void initEffect() {
  _paramEffect.delayEff = DELAY_INTERV + _ledCommand.speed * SPEED_K;
  _paramEffect.intensityEff = MAX_BRIGHT + (MAX_INTENSITY + 1 -_ledCommand.intensity) * BRIGHT_K;
  _paramEffect.theme = _ledCommand.theme;
  _paramEffect.currTheme = FIRST_DEFAULT_THEME;
}

/**
 * @brief read serial commands
 * 
 * @return true|false
  */
bool readSerialData() {
  #ifdef UART_SERIAL_ENABLE
    LedCommandStr ledCommandRead;
    uint8_t* pDataRead = (uint8_t*)&ledCommandRead;
    size_t byteCnt = 0;
    bool res = false;

    while (softSerial.available() > 0) {
      uint8_t readByte = softSerial.read();
      if (byteCnt < _sizeLedCommand) {
        pDataRead[byteCnt++] = readByte;
      }
    }

    if ( (byteCnt == _sizeLedCommand) && 
        (ledCommandRead.signature == 0xFA) &&
        (ledCommandRead.theme >= 1 && ledCommandRead.theme <= LAST_THEME) &&
        (ledCommandRead.intensity >= 1 && ledCommandRead.intensity <= MAX_INTENSITY) &&
        (ledCommandRead.speed >= 1 && ledCommandRead.speed <= MAX_SPEED)
      ) {
      //calculate CRC
      uint8_t dataCRC = 0;
      for (uint8_t i = 0; i < (_sizeLedCommand - 1); i++) {
        dataCRC += pDataRead[i];
      }

      if (dataCRC == ledCommandRead.checksum) {
        //copy data to main struct
        uint8_t* pDataMain = (uint8_t*)&_ledCommand;

        for (uint8_t i = 0; i < _sizeLedCommand; i++) {
          pDataMain[i] = pDataRead[i];
        }

        softSerial.println(F("OK"));
        res = true;
      }
    }

    return res;
  #endif
}