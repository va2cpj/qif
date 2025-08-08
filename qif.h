
// ~/Arduino/QIF/qif.h Located in parent directory and linked in subdirectory


#ifndef   QIF_H
#define   QIF_H

#undef  BOARD_NAME
#define BOARD_NAME QIF    

#include <stdint.h>
#include <stddef.h>

#include <RTC_SAMD51.h>
#include <ArduinoUniqueID.h>
#include <Wire.h>
#include <bsec.h>
#include <SPI.h>
#include <Adafruit_SPIFlash.h>
#include <Adafruit_FlashTransport.h>
#include <Adafruit_NeoPixel.h>
#include "SAMDTimerInterrupt.h"

#define CAN0_MESSAGE_RAM_SIZE 0                                                                    // RAM sizes must be defined for CAN *before* the CAN library
#define CAN1_MESSAGE_RAM_SIZE 4096                                                                 // CAN1 used
#include <ACANFD_FeatherM4CAN.h>

#include "function.h"
#include "db.h"
#include "crc64.h"

extern "C" uint32_t __etext;                                                                       // End of code in flash (from linker script)

#define QIF "QIF BATOTIK/ATSAME51J19A"

const uint8_t copyright[] PROGMEM = { "©2025 QIF / karel@qif.ch http://qif.ch" };

#define CAN_SPEED 250*1000

#define PWM_CHANNELS      8                                                                         // Switch & low power board
#define PWM_RESOLUTION    63                                                                        // 6-bits resolution (0–63)
#define TIMER_INTERVAL_US 40                                                                        // 40 µs = 25 kHz base frequency

// Control frames, first 2 hexgit replace by board label
#define STX  0x2A3F9E2D4C7B0A8EULL                                                                  // Start of data marker
#define ETX  0x8E3A6F5D42B9C0E7ULL                                                                  // End of data marker
#define DLE  0x7B4E09F3A2D8C156ULL                                                                  // Escape
#define HBT  0x2FC5A1D8E93B4670ULL                                                                  // Heart beat marker
#define ACK  0x7F4C3A9D0E2B6F81ULL                                                                  // Acknowledge message
#define NAK  0x5A9E3D7F1C2B8E64ULL                                                                  // Non acknowledge message

#define MARKER_MASK  0xA5A5A5A5A5A5A5ULL                                                            // Arbitrary 56-bit mask for obfuscation of control frame
#define TIMEOUT_MS 120000UL                                                                         // 120 seconds timeout

#define SVR     0x00                                                                                // Service channel CAN address, 0x00-0x0F

#define FLASH_BASE_ADDR  0x000004000                                                                // Flash programming start address
#define BOOT2_START_ADDR 0x00079000UL                                                               // Start of reserved bootloader area
#define RAM_START        0x20000000UL                                                               // Start of RAM
#define RAM_END          0x2002EE00UL                                                               // 192 KB
#define QSPI_BASE_ADDR   0x00000000                                                                 // QSPI programming start address
#define MQSPI_BASE_ADDR  0x04000000  																																// Memory-mapped QSPI base address
#define QSPI_BLOCK_SIZE  4096                                                                       // QSPI block size
#define QSPI_PAGE_SIZE   256                                                                        // QSPI page size
#define BYTES_PER_LINE   16                                                                         // 16 bytes per line output
#define MAX_FILTERS      128                                                                        // Maximum number of filter entries the manager can hold
#define BME688_ADDR_LOW   0x76                                                                      // Default I2C address
#define BME688_ADDR_HIGH  0x77                                                                      // Alternate I2C address 

#define BME1  "Temperature(°C): "                                                                   // BME68X extracted values
#define BME2  "Pressure(hPa):   "
#define BME3  "Humidity(%):     "
#define BME4  "Gas(%):          "
#define BME5  "Iaq:             "
#define BME6  "Voc:             "
#define BME7  "Co2(ppm):        "

#define MAX_TIMERS 4       

#define N 8                                                                                         // Number of switch
#define SHORT 20                                                                                    // ~200 ms
#define VERY_LONG 100                                                                               // ~1 sec
#define WAIT_RESET 200                                                                              // ~2 sec idle resets click count
#define SHORT_DELAY_LIMIT 20                                                                        // ~200 ms wait for possible second short

#define CAN_NULL  0x0400                                                                            // Voluntarily outside the address range of CAN     

#define ON      1
#define OFF     0
#define FORWARD 0
#define REVERSE 1

enum INFO : uint8_t { BMETEMP = 30,BMEPRESS,BMEHUMID,BMEGAZ,BMEIAQ,BMEVOC,BMECO2 };                 // Message info, first byte of can message data 
              
typedef void (*FilterCallback)(const CANFDMessage &);

// ----------------------------------------------------------------------------
// Click enum values
// ----------------------------------------------------------------------------
enum ClickValue : uint8_t {                                                                      
  CLICK_NONE = 0,   // Uninitialized
  CLICK_S    = 1,   // Single short
  CLICK_SS   = 2,   // Double short
  CLICK_L    = 3,   // Single long
  CLICK_SL   = 4,   // Short + Long
  CLICK_VL   = 5    // Very long hold
};

typedef struct {                                                                                    // Switch board structure
  uint8_t tim;
  uint8_t wait;
  uint8_t cnt;
  uint8_t longCnt;
  uint8_t state;
  uint8_t shortDetected;
  uint8_t shortDelay;
} Switch;

typedef struct {
  volatile uint64_t counter;                                                                        // Countdown in ticks
  volatile bool     active;                                                                         // Active state
  volatile bool     flag;                                                                           // Expiry flag (set to true when expired)
} DelayTask;

  struct BSEC                                                                                       // BME readings
    {
      float iaq;
      float co2;
      float voc;
      float pressure;
      float temperature;
      float humidity;
      float gas;
    };

typedef union {                                                                                     // Define a named union to hold the value of BME688
  float value;
  uint8_t bytes[4];
} BMEData;

RTC_SAMD51 rtc;                                                                                     // Set to get one interrupt evey 60 seconds
                                                                            
Adafruit_NeoPixel strip(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

uint32_t RED =      strip.Color(255, 0, 0);                                                         // Any errors
uint32_t GREEN =    strip.Color(0, 255, 0);                                                         // Loop executing (blinking)
uint32_t LILAC =    strip.Color(142, 68, 173);                                                      // Filter manager fail (blinking)
uint32_t BLUE =     strip.Color(0, 0, 255);                                                         // Receiving CAN
uint32_t MAGENTA =  strip.Color(255, 0, 255);                                                       // Sending CAN
uint32_t YELLOW =   strip.Color(255,255,0);                                                         // Read BME value
uint32_t ORANGE =   strip.Color(255, 165, 0);                                                       // BME error reading
uint32_t PINK =     strip.Color(255, 0, 127); 
uint32_t BLACK =    strip.Color(0, 0, 0);                                                           // Black
uint32_t WHITE =    strip.Color(255, 255, 255);                                                     // White

// Useful macros
#define BLINK(color)  do { strip.setPixelColor(0, color); strip.show(); } while(0)
#define ATOMIC() for (bool _once = (noInterrupts(), true); _once; _once = (interrupts(), false))
#define NOP __asm__("nop\n\t")                                                                      // No blocking delay

int year, month, day, hour, minute, second;                                                         // hour, minute, second, day, month, year

Bsec iaqSensor; 

/*
States state = NONE;                                                                                // State-machine current state
uint16_t State;                                                                                     
uint16_t Value; 
*/

enum STATE_t                                                                                        // state machine reading serial input
  {                                                                                                 // The possible states of the state-machine
    NONE,
    UPD,                                                                                            // Send update
    RST,                                                                                            // Reset (reboot) specific board
    BMX,                                                                                            // Ask BME value
    ANA,                                                                                            // Ask analog value
    I2C,                                                                                            // Scan I2C bus
    TIM,                                                                                            // Send time
    HLP,                                                                                            // Print command list
    QSP,                                                                                            // Dump QSPI memory block
    DMP,                                                                                            // Dump FLASH memory block
    MNT,                                                                                            // Set monitor mode
    FLT,                                                                                            // Print active filter
    SND,                                                                                            // Send message to: label, sub-address, value
    MLI                                                                                             // Send message to: label, pwm channel, value
  };

STATE_t State     = NONE;
uint16_t Value    = 0;                                                                              // Argument 1 (label)
uint8_t  Value1   = 0;                                                                              // Argument 2
uint8_t  Value2   = 0;                                                                              // Argument 3
uint8_t  Value3   = 0;                                                                              // Argument 4
uint8_t  argIndex = 0;                                                                              // Argument selector

uint8_t BME688    = 0;                                                                              // BME68X module I2C address

DateTime now;

bsec_virtual_sensor_t sensorList[13] =
  { 
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_STABILIZATION_STATUS,
    BSEC_OUTPUT_RUN_IN_STATUS,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    BSEC_OUTPUT_GAS_PERCENTAGE
  };

#endif

