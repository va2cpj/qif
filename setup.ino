
// ~/Arduino/QIF/switch/setup.ino


#include "qif.h"

typedef void (*FilterCallback)(const CANFDMessage &);

typedef struct {                                                                                     // Filter entry structure
    uint16_t From;                                                                                   // starting CAN ID
    uint16_t To;                                                                                     // ending CAN ID
    FilterCallback callback;                                                                         // pointer to function like Process_Led, Process_Dummy
} FilterEntry;

void setup()
  {
  	Serial.begin(115200);
    unsigned long start = millis();

// Wait up to 5 seconds for Serial to be ready
    while(!Serial && (millis() - start < 5000)) { DELAY(10); }

// Detection of IDE connection
    if(Serial && Serial.dtr()) { IDE = true; } else { IDE = false; }
  
    randomSeed(micros());

    pinMode(PIN_CAN_STANDBY, OUTPUT);
    digitalWrite(PIN_CAN_STANDBY, false);                                                        // turn off STANDBY
    pinMode(PIN_CAN_BOOSTEN, OUTPUT);
    digitalWrite(PIN_CAN_BOOSTEN, true);                                                         // turn on BOOSTER
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    SERCOM0->USART.CTRLA.bit.ENABLE = 0;

    UID       = uuid2uid();
    CAN_BASE  = getCAN(UID);
    LABEL     = getLBL(UID);
    TYPE      = getTYPE(UID);

// Set control frame
    stx = STX >> 8;                                                                              // Remove label from original STX marker
    etx = ETX >> 8;
    dle = DLE >> 8;
    ack = ACK >> 8;
    nak = NAK >> 8;

#define VERSION   01             
#define REVISION  00  

const char  copyright[] PROGMEM = { "©2026 QIF / karel@qif.ch http://qif.ch" };

    if(IDE)
      {
        Serial.println();                                                                         // Connected to IDE, print some debugging messages
        Serial.println ((const __FlashStringHelper *) copyright);
        Serial.println(F("CONNECTED TO THE QIF/BATOTIK"));
        Serial.print(F("VERSION:      "));
        Serial.print(VERSION);
        Serial.print('.');
        Serial.print(REVISION);
        Serial.print("  ");
        now = DateTime(F(__DATE__), F(__TIME__));                                                 // if IDE connected to a PC
        Serial.print(now.timestamp(DateTime::TIMESTAMP_DATE));
        Serial.print(" ");
        Serial.println(now.timestamp(DateTime::TIMESTAMP_TIME)); 
        Serial.print(F("GENERATED ON: "));
        Serial.println(LABEL);
        Serial.print(F("BOARD:        "));
        Serial.println(BOARD_NAME);
        Serial.print(F("UUID:         0x"));
        for(uint8_t i = 0; i < (8 > 8 ? 8 : 8); i++) { UUID[i] = UniqueID[i]; Serial.print(UniqueID[i], HEX); }
        Serial.println(); 
        Serial.print(F("UID:          0x"));
        Serial.println(UID, HEX);
        Serial.print(F("LABEL:        "));
        Serial.println(LABEL);
        Serial.print(F("CAN:          0x"));
        Serial.println(CAN_BASE,HEX);
        Serial.print(F("TYPE:         "));
        Serial.println(typeToString(DB[LABEL].TYPE));
        Serial.print(F("DB SIZE       "));
        Serial.println(sizeof(DB));
        uint32_t flash_used = (uint32_t)&__etext;
        Serial.print("FLASH USED:   ");
        Serial.print((float)flash_used / 1024.0, 2);
        Serial.println(" KB");
      } 

    if(!IDE) now = 946728000;                                                                     // Not connected set at 1/1/2000 12:00:00

    strip.begin();
    strip.setBrightness(10);

    rtc.begin();
    rtc.adjust(now);             
    DateTime alarm = DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute() + 1, now.second());
    rtc.setAlarm(0, alarm);
    rtc.enableAlarm(0, rtc.MATCH_SS);
    rtc.attachInterrupt(alarmMatch);                                                              // RTC interrupt evey minute


    Wire.begin();
    Wire.setClock(250000);                                                                        // Start the I2C bus at 250 kbps
    ScanI2C();
    if(BME688 != 0)
      {
        BME_FLAG = true;
        Wire.beginTransmission(BME688);
      }
    else Wire.endTransmission();                                                                  // No module present, disable I2C

    if(BME_FLAG) { if(IDE) Serial.println(F("BME688        MODULE FOUND")); }
    else { if(IDE) Serial.println(F("BME688        MODULE NOT FOUND")); }

    if(BME_FLAG)
        {
          iaqSensor.begin(BME688, Wire);                                                          // I2C connect to BME688

        if(IDE)
          {
            Serial.print(F("BSEC LIBRARY VERSION: ")) ;
            Serial.print(iaqSensor.version.major);
            Serial.print("." );
            Serial.println(iaqSensor.version.minor);
          }
        DELAY(1000);
        iaqSensor.updateSubscription(sensorList, 13, BSEC_SAMPLE_RATE_LP);
        checkIaqSensorStatus();
        readBME();
        }
      
    if(!flash.begin())                                                                            // Initialize QSPI flash
      {
        if(IDE) Serial.println(F("Failed to initialize QSPI flash"));
        while (true)
          {
            BLINK(LILAC); DELAY(1000);                                                            // Error
          }
      }
    if(IDE) Serial.println(F("QSPI MEMORY   INITIALIZED"));

/*
  if(!eraseQSPI()) if(IDE) Serial.println(F("QSPI ERASE FAILED"));                                // Erase all QSPI memory (all 0xff)
  if(!Mirror2QSPI()) if(IDE) Serial.println(F("COPY FLASH TO QSPI FAILED"));                      // Mirror flash from 0x4000 to QSPI memory starting at 0x0000
  verifyQSPI();
*/

  DWT_Init();                                                                                     // Initialize the cycle counter
  TCC2_Setup();                                                                                   // Setup timer counter interrupt handler

  if(!CAN_Setup())                                                                                // Initializes CAN
    {
      if(IDE) Serial.println(F("Filter apply failed"));
      while (true)
        {
          BLINK(LILAC); DELAY(1000);
        }
    } 
  if(IDE) Serial.println();  

  startDelay(0, 1000);                                                                             // Init with 1000    * 1ms  -> 1000ms   (1 second)
  startDelay(1, 2000);                                                                             // Init with 2000    * 1ms  -> 2000ms   (2 second)
  startDelay(2, 60000);                                                                            // Init with 60000   * 1 ms -> 6000ms   (1 minute)
  startDelay(3, 3600000);                                                                          // Init with 3600000 * 1 ms -> 360000ms (1 hour)

  InitPins(LABEL);

  if(TYPE == LPOWER)
    {
      analogReadResolution(12);                                                                     // Set ADC resolution to 12 bits (0–4096)     
      analogReference(AR_DEFAULT);                                                                  // Use internal 1.0 V reference
      DELAY(1);
      Serial.println();
      float measuredvbat = analogRead(A6);                                                          // Read raw ADC value (0–4095)
      measuredvbat = (measuredvbat * 2.0 * 3.3) / 4096.0;                                           // Measure was divided by 2, multiply back, multiply by 1.0V, 1.0V internal
      if(IDE) { Serial.print("BATTERY: " ); Serial.print(measuredvbat); Serial.println(F(" Volt")); }
    }

  if(!ITimer.attachInterruptInterval(TIMER_INTERVAL_US, TimerHandler))                              // Start PWM timer interrupt
    {                              
      if(IDE)Serial.println(F("Failed to start ITimer!"));
    }
  else
    {
      if(IDE)Serial.println(F("8-CHANNEL PWM STARTED."));                                           // Set all PWM channels duty cycle


    for (uint8_t i = 0; i < PWM_CHANNELS; i++)
      {
        saved_PWM[i] = 0;
        pwmDir[i] = 0;
        pwmPendingResume[i] = false;
        pwmNextDuty[i] = 0;
        pwmNextDir[i] = 0;
        pwmStopTime[i] = 0;

        if(TYPE == SWITCH) pwmDuty[i] = map(100, 0, 100, 0, PWM_RESOLUTION);                         // Inverted logic — 0% → max duty to turn off LED (active-low)
        else pwmDuty[i] = 0;                                                                         // Motors or non-inverted outputs
      }
    }

  Help();

//  WDT_Setup();                                                                                      // Watchdog setup (not used at this time of development)                                                                       
  }

//----------------------------------------------------------------------------------------
/*
TCC2Setup: Configures and enables Timer/Counter Control 2 (TCC2).
This function sets up the TCC2 peripheral on the microcontroller.
It enables the peripheral clock, sets the prescaler, configures the period register,
enables the overflow interrupt, and finally enables the timer.
The function also configures the interrupt controller to handle TCC2 interrupts
with the lowest priority.
Steps performed by this function:
1. Enable the peripheral channel for TCC2 and connect to generic clock 4 (12MHz).
2. Set the prescaler to divide by 1024.
3. Set the period register to its maximum value.
4. Enable the overflow interrupt.
5. Enable the TCC2 timer.
6. Wait for synchronization.
7. Enable the NVIC interrupt for TCC2 channel 0.
8. Set the NVIC priority for the TCC2 interrupt to the lowest level.
*/
void TCC2_Setup(void)                                                                                  
{
  GCLK->PCHCTRL[TCC2_GCLK_ID].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN_GCLK4;  // Use GCLK4 @12 MHz

  TCC2->CTRLA.bit.ENABLE = 0;                  // Disable before config
  while (TCC2->SYNCBUSY.bit.ENABLE);           // Wait for sync

  TCC2->CTRLA.bit.PRESCALER = TCC_CTRLA_PRESCALER_DIV1024_Val;  // Divide 12 MHz by 1024 → ~11.7 kHz
  TCC2->PER.reg = 11;                          // 1 ms = ~11.72 ticks → round to 11 (12 counts total)
  
  TCC2->INTENSET.bit.OVF = 1;                  // Enable overflow interrupt
  TCC2->CTRLA.bit.ENABLE = 1;                  // Enable TCC2
  while (TCC2->SYNCBUSY.bit.ENABLE);           // Wait for sync

  NVIC_EnableIRQ(TCC2_0_IRQn);                 // Enable interrupt
  NVIC_SetPriority(TCC2_0_IRQn, 2);            // Set priority (adjust as needed)
}

// ----------------------------------------------------------------------------
// Initializes the Data Watchpoint and Trace (DWT) unit for cycle counting.
// ----------------------------------------------------------------------------

void DWT_Init(void)
  {    
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;             // Enable TRC    
    DWT->CYCCNT = 0;                                            // Reset the cycle counter   
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;                        // Enable the cycle counter
  }

//----------------------------------------------------------------------------------------
/*
WDT_Setup: Configures and enables the Watchdog Timer (WDT).
This function sets up the Watchdog Timer (WDT) on the microcontroller.
It enables the necessary clock for the WDT, configures the WDT timeout period,
sets the NVIC priority for the WDT interrupt, connects the WDT to the NVIC,
and finally enables the WDT.
The function also ensures that the WDT is synchronized before returning.
Steps performed by this function:
1. Enable the APB clock for the WDT.
2. Set the WDT timeout period to approximately 16 seconds.
3. Set the NVIC priority for the WDT interrupt to the highest level (0).
4. Enable the NVIC interrupt for the WDT.
5. Enable the WDT.
6. Wait for the WDT to synchronize before returning.
*/
void WDT_Setup(void)
  {
    MCLK->APBAMASK.reg |= MCLK_APBAMASK_WDT;                                                          // Activate the CLK_WDT_APB clock
    WDT->CONFIG.bit.PER = WDT_CONFIG_PER_CYC16384;                                                    // Set WDT timeout to 16 seconds  
    NVIC_SetPriority(WDT_IRQn, 0);                                                                    // Set NVIC priority for WDT to 0 
    NVIC_EnableIRQ(WDT_IRQn);                                                                         // Connect WDT to NVIC
    WDT->CTRLA.reg = WDT_CTRLA_ENABLE;                                                                // Enable the WDT
    while((WDT->SYNCBUSY.reg & WDT_SYNCBUSY_ENABLE) == WDT_SYNCBUSY_ENABLE);
  }

//----------------------------------------------------------------------------------------
/*
Init_CAN: Initializes the CAN (Controller Area Network) interface with specific settings and filters.
This function sets up the CAN interface with a baud rate of 250 kbps and configures it for Normal FD mode.
It adds several filters to manage incoming CAN frames, including range filters.
It also adds an additional range filter based on the device ID.
Frames that do not match any filter are rejected.
The function sets the sizes of hardware and driver receive FIFOs and then attempts
to start the CAN interface with these settings.
The function returns `true` if the initialization is successful,and `false` otherwise.
*/
bool CAN_Setup(void)                                                                                  // Configure CAN
  {
  // Configure hardware FIFO sizes (in the CAN peripheral)
  settings.mHardwareRxFIFO0Size         = 16;                                                         // Hardware FIFO0 size
  settings.mHardwareTransmitTxFIFOSize  = 16;                                                         // TX FIFO in hardware

  // Configure software driver buffers (used between hardware and app)
  settings.mDriverReceiveFIFO0Size      = 256;                                                        // Software RX FIFO0 size
  settings.mDriverTransmitFIFOSize      = 128;                                                        // Software TX buffer size                                             

  const uint32_t errorCode = can1.beginFD(settings);
    if(errorCode != 0)
      {
        if(IDE) Serial.println(F("CAN INIT       FAILED"));
        while(1);
      }
    if(IDE) Serial.println(F("CAN           STARTED"));


// Set up a filter that triggers the callback when receiving matching frame
    ACANFD_FeatherM4CAN::StandardFilters standardFilters;

//----------------------------------------------------------------------------------------
// Service static filters
//----------------------------------------------------------------------------------------
FilterEntry ServiceFilters[] = {
  { (uint16_t)(SVR + Time),         (uint16_t)(SVR + Time),         Process_Time        },
  { (uint16_t)(SVR + Reset),        (uint16_t)(SVR + Reset),        Process_Reboot      },
  { (uint16_t)(SVR + Update),       (uint16_t)(SVR + Update),       Process_Update      },
  { (uint16_t)(SVR + Abme),         (uint16_t)(SVR + Abme),         Process_Alarm_BME   },
  { (uint16_t)(SVR + Hbt),          (uint16_t)(SVR + Hbt),          Process_Heart_Beat  },
  { (uint16_t)(SVR + Ack),          (uint16_t)(SVR + Ack),          Process_ACK         },
  { (uint16_t)(SVR + Nack),         (uint16_t)(SVR + Nack),         Process_NACK        },
  { (uint16_t)(SVR + Gps),          (uint16_t)(SVR + Gps),          Process_GPS         },
  { (uint16_t)(SVR + Gyro),         (uint16_t)(SVR + Gyro),         Process_Gyro        },
  { (uint16_t)(SVR + Anl),          (uint16_t)(SVR + Anl),          Process_Analog_RX   },
  { (uint16_t)(SVR + Pir),          (uint16_t)(SVR + Pir),          Process_Pir         }
};

const size_t ServiceFilterCount = sizeof(ServiceFilters) / sizeof(FilterEntry);

//----------------------------------------------------------------------------------------
// Switch static filters
//----------------------------------------------------------------------------------------
FilterEntry SwitchFilters[] = {
  { (uint16_t)(CAN_BASE + 0x00),    (uint16_t)(CAN_BASE + 0x00),    Process_Led         },                
  { (uint16_t)(CAN_BASE + 0x01),    (uint16_t)(CAN_BASE + 0x01),    Process_Led         },
  { (uint16_t)(CAN_BASE + 0x02),    (uint16_t)(CAN_BASE + 0x02),    Process_Led         },
  { (uint16_t)(CAN_BASE + 0x03),    (uint16_t)(CAN_BASE + 0x03),    Process_Led         },
  { (uint16_t)(CAN_BASE + 0x04),    (uint16_t)(CAN_BASE + 0x04),    Process_Led         },
  { (uint16_t)(CAN_BASE + 0x05),    (uint16_t)(CAN_BASE + 0x05),    Process_Led         },
  { (uint16_t)(CAN_BASE + 0x06),    (uint16_t)(CAN_BASE + 0x06),    Process_Led         },
  { (uint16_t)(CAN_BASE + 0x07),    (uint16_t)(CAN_BASE + 0x07),    Process_Led         },
  { (uint16_t)(CAN_BASE + 0x0F),    (uint16_t)(CAN_BASE + 0x0F),    Process_BME         }
};
const size_t SwitchFilterCount = sizeof(SwitchFilters) / sizeof(FilterEntry);

//----------------------------------------------------------------------------------------
// Low power static filters
//----------------------------------------------------------------------------------------
FilterEntry LpowerFilters[] = {
  { (uint16_t)(CAN_BASE + 0x00),    (uint16_t)(CAN_BASE + 0x00),    Process_Lpwm        },                
  { (uint16_t)(CAN_BASE + 0x01),    (uint16_t)(CAN_BASE + 0x01),    Process_Lpwm        },
  { (uint16_t)(CAN_BASE + 0x02),    (uint16_t)(CAN_BASE + 0x02),    Process_Lpwm        },
  { (uint16_t)(CAN_BASE + 0x03),    (uint16_t)(CAN_BASE + 0x03),    Process_Lpwm        },
  { (uint16_t)(CAN_BASE + 0x04),    (uint16_t)(CAN_BASE + 0x04),    Process_Lpwm        },
  { (uint16_t)(CAN_BASE + 0x05),    (uint16_t)(CAN_BASE + 0x05),    Process_Lpwm        },
  { (uint16_t)(CAN_BASE + 0x06),    (uint16_t)(CAN_BASE + 0x06),    Process_Lpwm        },
  { (uint16_t)(CAN_BASE + 0x07),    (uint16_t)(CAN_BASE + 0x07),    Process_Lpwm        },
  { (uint16_t)(CAN_BASE + 0x08),    (uint16_t)(CAN_BASE + 0x08),    Process_PwrCtrl     },
  { (uint16_t)(CAN_BASE + 0x09),    (uint16_t)(CAN_BASE + 0x09),    Process_Analog      },
  { (uint16_t)(CAN_BASE + 0x0A),    (uint16_t)(CAN_BASE + 0x0A),    Process_Analog      },
  { (uint16_t)(CAN_BASE + 0x0B),    (uint16_t)(CAN_BASE + 0x0B),    Process_Analog      },
  { (uint16_t)(CAN_BASE + 0x0C),    (uint16_t)(CAN_BASE + 0x0C),    Process_Analog      },
  { (uint16_t)(CAN_BASE + 0x0D),    (uint16_t)(CAN_BASE + 0x0D),    Process_Isense      },
  { (uint16_t)(CAN_BASE + 0x0F),    (uint16_t)(CAN_BASE + 0x0F),    Process_BME         }
};
const size_t LpowerFilterCount = sizeof(LpowerFilters) / sizeof(FilterEntry);

//----------------------------------------------------------------------------------------
// Medium power static filters
//----------------------------------------------------------------------------------------
FilterEntry MpowerFilters[] = {
  { (uint16_t)(CAN_BASE + 0x00),    (uint16_t)(CAN_BASE + 0x00),    Process_Hpwm        },
  { (uint16_t)(CAN_BASE + 0x01),    (uint16_t)(CAN_BASE + 0x01),    Process_Hpwm        },
  { (uint16_t)(CAN_BASE + 0x02),    (uint16_t)(CAN_BASE + 0x02),    Process_Hpwm        },
  { (uint16_t)(CAN_BASE + 0x03),    (uint16_t)(CAN_BASE + 0x03),    Process_Hpwm        },
  { (uint16_t)(CAN_BASE + 0x04),    (uint16_t)(CAN_BASE + 0x04),    Process_Hpwm        },
  { (uint16_t)(CAN_BASE + 0x05),    (uint16_t)(CAN_BASE + 0x05),    Process_Hpwm        },
  { (uint16_t)(CAN_BASE + 0x08),    (uint16_t)(CAN_BASE + 0x08),    Process_PwrCtrl     },
  { (uint16_t)(CAN_BASE + 0x09),    (uint16_t)(CAN_BASE + 0x09),    Process_Analog      },
  { (uint16_t)(CAN_BASE + 0x0A),    (uint16_t)(CAN_BASE + 0x0A),    Process_Analog      },
  { (uint16_t)(CAN_BASE + 0x0B),    (uint16_t)(CAN_BASE + 0x0B),    Process_Analog      },
  { (uint16_t)(CAN_BASE + 0x0C),    (uint16_t)(CAN_BASE + 0x0C),    Process_Analog      },
  { (uint16_t)(CAN_BASE + 0x0D),    (uint16_t)(CAN_BASE + 0x0D),    Process_Isense      },
  { (uint16_t)(CAN_BASE + 0x0F),    (uint16_t)(CAN_BASE + 0x0F),    Process_BME }
};
const size_t MpowerFilterCount = sizeof(MpowerFilters) / sizeof(FilterEntry);

//----------------------------------------------------------------------------------------
// High power static filters
//----------------------------------------------------------------------------------------
FilterEntry HpowerFilters[] = {
  { (uint16_t)(CAN_BASE + 0x00),    (uint16_t)(CAN_BASE + 0x00),    Process_Hpwm        },
  { (uint16_t)(CAN_BASE + 0x01),    (uint16_t)(CAN_BASE + 0x01),    Process_Hpwm        },
  { (uint16_t)(CAN_BASE + 0x02),    (uint16_t)(CAN_BASE + 0x02),    Process_Hpwm        },
  { (uint16_t)(CAN_BASE + 0x03),    (uint16_t)(CAN_BASE + 0x03),    Process_Hpwm        },
  { (uint16_t)(CAN_BASE + 0x04),    (uint16_t)(CAN_BASE + 0x04),    Process_Hpwm        },
  { (uint16_t)(CAN_BASE + 0x05),    (uint16_t)(CAN_BASE + 0x05),    Process_Hpwm        },
  { (uint16_t)(CAN_BASE + 0x08),    (uint16_t)(CAN_BASE + 0x08),    Process_PwrCtrl      },
  { (uint16_t)(CAN_BASE + 0x09),    (uint16_t)(CAN_BASE + 0x09),    Process_Analog      },
  { (uint16_t)(CAN_BASE + 0x0A),    (uint16_t)(CAN_BASE + 0x0A),    Process_Analog      },
  { (uint16_t)(CAN_BASE + 0x0B),    (uint16_t)(CAN_BASE + 0x0B),    Process_Analog      },
  { (uint16_t)(CAN_BASE + 0x0C),    (uint16_t)(CAN_BASE + 0x0C),    Process_Analog      },
  { (uint16_t)(CAN_BASE + 0x0D),    (uint16_t)(CAN_BASE + 0x0D),    Process_Isense      },
  { (uint16_t)(CAN_BASE + 0x0F),    (uint16_t)(CAN_BASE + 0x0F),    Process_BME         }
};
const size_t HpowerFilterCount = sizeof(HpowerFilters) / sizeof(FilterEntry);

// Filter groups:
// group 0 = TYPE-based filters
// group 1 = SERVICE filters

FilterEntry* ActiveFilters[2];
size_t       ActiveFilterCount[2];

switch (TYPE) {
  case SWITCH:
    ActiveFilters[0]      = SwitchFilters;
    ActiveFilterCount[0]  = sizeof(SwitchFilters) / sizeof(FilterEntry);
    break;
  case LPOWER:
    ActiveFilters[0]      = LpowerFilters;
    ActiveFilterCount[0]  = sizeof(LpowerFilters) / sizeof(FilterEntry);
    break;
  case MPOWER:
    ActiveFilters[0]      = MpowerFilters;
    ActiveFilterCount[0]  = sizeof(MpowerFilters) / sizeof(FilterEntry);
    break;
  case HPOWER:
    ActiveFilters[0]      = HpowerFilters;
    ActiveFilterCount[0]  = sizeof(HpowerFilters) / sizeof(FilterEntry);
    break;
  default:
    ActiveFilters[0]      = nullptr;
    ActiveFilterCount[0]  = 0;
    break;
}

// Group 1: always apply service filters
ActiveFilters[1]      = ServiceFilters;
ActiveFilterCount[1]  = sizeof(ServiceFilters) / sizeof(FilterEntry);


for (uint8_t group = 0; group < 2; group++) {
  for (size_t i = 0; i < ActiveFilterCount[group]; i++) {
    if (ActiveFilters[group][i].callback != nullptr) {
      filterManager_add(&filterManager,
                        ActiveFilters[group][i].From,
                        ActiveFilters[group][i].To,
                        ACANFD_FeatherM4CAN_FilterAction::FIFO0,
                        ActiveFilters[group][i].callback);
    }
  }
}

// Initialize CAN with settings and filter
    bool status = filterManager_apply(&filterManager, &can1, &settings);                               // Apply filter
    if(!status && IDE) { Serial.println(F("Filter not applied!")); }
    if(status) return true;
    else return false;
  
  }

//----------------------------------------------------------------------------------------
/*
  Function: Init Feather Pins From DB
  Purpose: Initialize the GPIO pins of the Adafruit Feather M4 CAN Express
           based on the DB configuration and a given board label.
  Description:
    - Looks up the DB[] table for the entry matching the provided label.
    - Applies the pin configurations (UNSET, OUTPUT, INPUT, PWM) for that board.
  Parameters:
    - label (uint8_t): The label identifying the board configuration.
*/
void InitPins(uint8_t label)
  {
    if(label >= DB_count)
      {
        Serial.print(F("Invalid label: ")); Serial.println(label);
        return;
      }

    const IO &entry = DB[label];

    if(IDE)
      {
        if(IDE)Serial.print(F("PINS INIT:    "));
        if(entry.TYPE < sizeof(mode) / sizeof(mode[0])) if (IDE) Serial.println(mode[entry.TYPE]);
        else if(IDE)Serial.println(F("UNKNOWN"));
      }

    const Pins* pins;                                                             // Get the appropriate pin configuration based on board type
    switch(entry.TYPE)
      {
        case SWITCH: pins = &SWITCH_PIN; break;
        case LPOWER: pins = &LPOWER_PIN; break;
        case MPOWER: pins = &MPOWER_PIN; break;
        case HPOWER: pins = &HPOWER_PIN; break;
        default:     pins = &UNSET_PIN;  break;
      }

    char buf[8];

    for(uint8_t i = 0; i < NB_PINS; i++)
      {
        if(IDE) if(i % 7 == 0) Serial.println();

        const Pin& p = pins->pin[i];

        switch (p.FUNCTION)
          {
            case OUT:
              pinMode(p.NUMBER, OUTPUT);
              if(entry.TYPE == SWITCH) digitalWrite(p.NUMBER, HIGH) ; else digitalWrite(p.NUMBER, LOW); // Preset output value
              if(IDE) { sprintf(buf, "%02u", p.NUMBER); Serial.print(buf); Serial.print(F(":OUT, ")); }
            break;

            case INP:
              pinMode(p.NUMBER, INPUT_PULLUP);
              if(IDE) { sprintf(buf, "%02u", p.NUMBER); Serial.print(buf); Serial.print(F(":INP, ")); }
            break;

            case PWM:
              pinMode(p.NUMBER, OUTPUT);
              if(IDE) { sprintf(buf, "%02u", p.NUMBER); Serial.print(buf); Serial.print(F(":PWM, ")); }
            break;  

            case UNSET:                                                                                   // Do nothing                                   
              if(IDE) { sprintf(buf, "%02u", p.NUMBER); Serial.print(buf); Serial.print(F(":UNS, ")); }
            break;
            default:                                             
            break;
          }
      }

    if(TYPE == SWITCH)
      {
        pwmPins[0] = 1;   pwmPins[1] = 4;  pwmPins[2] = 13; pwmPins[3] = 12;                            // Translate Pin/Port
        pwmPins[4] = 11;  pwmPins[5] = 10; pwmPins[6] = 9;  pwmPins[7] = 6;
      }

    if(TYPE == LPOWER)
      {
        pwmPins[0]    = 12;  pwmPins[1]   = 11;   pwmPins[2]    = 10; pwmPins[3]    = 9;                  // Translate Pin/Port
        pwmPins[4]    = 6;   pwmPins[5]   = 5;    pwmPins[6]    = 0;  pwmPins[7]    = 1;
        analogPins[0] = 16;  analogPins[1] = 17;  analogPins[2] = 18; analogPins[3] = 19;
      }

    if(TYPE == MPOWER)
      {
        pwmPins[0]    = 12;  pwmPins[1] = 11;     pwmPins[2]    = 10; pwmPins[3]    =  9;    
        pwmPins[4]    =  6;  pwmPins[5] =  5;     pwmPins[6]    =  0; pwmPins[7]    =  1;
        pwmPins[8]    =  2;  pwmPins[9] =  3;     pwmPins[10]   =  4; pwmPins[11]   =  7;
      }

    if(TYPE == HPOWER)
      {

      }
    if(IDE) { Serial.println(); Serial.println(F("PINS INIT:    DONE")); };
  }

void startDelay(uint8_t slot, uint64_t ticks) 
{
  if (slot >= MAX_TIMERS) return;
  delays[slot].counter = ticks;
  delays[slot].active = true;
  delays[slot].flag = false;
}

const char* typeToString(uint8_t type) {
  switch (type) {
    case UNDEF:  return "UNDEF";
    case SWITCH: return "SWITCH";
    case LPOWER: return "LOW POWER";
    case MPOWER: return "MEDIUM POWER";
    case HPOWER: return "HIGH POWER";
    default:     return "UNKNOWN";
  }
}