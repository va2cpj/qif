
// ~/Arduino/QIF/test/switch.ino

#include "qif.h"

// Structure representing a CAN single filter entry
typedef struct {
  uint16_t idStart;                                                                   // Start of CAN ID range
  uint16_t idEnd;                                                                     // End of CAN ID range
  ACANFD_FeatherM4CAN_FilterAction action;                                            // Filter action
  ACANFDCallBackRoutine callback;                                                     // Callback function
  bool valid;                                                                         // Whether this entry is currently active
} CANFilterEntry;

// The filter manager — holds an array of active filters
typedef struct {
  CANFilterEntry entries[MAX_FILTERS];                                                // Fixed array of filters
  uint8_t count;                                                                      // Current number of active filters
} CANFilterManager;

CANFilterManager filterManager;

typedef struct {
  uint64_t crc;
} CRC64;

extern ACANFD_FeatherM4CAN can1;                                                      // Reference the CAN object declared by the library

uint64_t  UID;
uint16_t  CAN_BASE;                                                                   // CAN address of the board
uint8_t   TYPE;
uint8_t   LABEL;
char      UUID[8];  

const char* typeNames[] = {
  "UNDEF",
  "SWITCH",
  "LPOWER",
  "MPOWER",
  "HPOWER"
};

volatile bool  IDE          = false;                                                   // True when a PC is connected
volatile bool  BME_FLAG     = false;                                                   // BME present in module
volatile bool  MONITOR_FLAG = false;
volatile bool  LOCK         = false; 
volatile bool  BLINK        = false;
volatile bool  STX_FLAG     = false;
volatile bool  ETX_FLAG     = false;                                                                         

volatile uint64_t stx;                                                                 // Obfuscated control frame
volatile uint64_t etx;
volatile uint64_t dle;
volatile uint64_t ack;
volatile uint64_t nak;

Switch switchState[N];
uint8_t switchPins[N] = { BTNP0, BTNP1, BTNP2, BTNP3, BTNP4, BTNP5, BTNP6, BTNP7 };
uint8_t ledPins[N]    = { LED00, LED01, LED02, LED03, LED04, LED05, LED06, LED07 };

DelayTask delays[MAX_TIMERS];                                                          // Timer pool

uint64_t previousMillis = 0;
unsigned long startMicros = 0;

bool      Blink_Mode[8]  { false };                                                    // All elements initialized to false

uint8_t gCAN1MessageRAM[CAN1_MESSAGE_RAM_SIZE] __attribute__((aligned(4)));            // Required buffer for CAN1

// Configure CAN speed
ACANFD_FeatherM4CAN_Settings settings (ACANFD_FeatherM4CAN_Settings::CLOCK_48MHz, CAN_SPEED, DataBitRateFactor::x8);

CANFilterManager savedFilterManager;                                                   // Global backup for full filter manager structure

// Define the 8 PWM output pins
uint8_t           pwmPins[PWM_CHANNELS * 2] = {0};                                         
volatile uint8_t  pwmDuty[PWM_CHANNELS] = {0};                                         // Per-channel 8-bit duty values (0–127)
volatile uint8_t  pwmTick = 0;                                                         // PWM tick counter (0–PWM_RESOLUTION)
uint8_t           saved_PWM[PWM_CHANNELS];                                             // Saved PWM duty-cycle
bool              pwmDir[PWM_CHANNELS] = { FORWARD };                                  // true = forward, false = reverse
bool              blink_flag[PWM_CHANNELS] = { false };                                // Blink flag for 8 leds
bool              pwmPendingResume[PWM_CHANNELS];                                      // Flag to indicate motor restart needed
uint32_t          pwmStopTime[PWM_CHANNELS];                                           // Timestamp when stop was issued
uint8_t           pwmNextDuty[PWM_CHANNELS];                                           // Next duty after delay
bool              pwmNextDir[PWM_CHANNELS];                                             // Next direction after delay
SAMDTimer         ITimer(TIMER_TC3);     

// Define 4 ANALOG input pins                                             
uint8_t           analogPins[4] = {0};

volatile struct BSEC BME;

DateTime alarm;

String output;

Adafruit_FlashTransport_QSPI flashTransport = Adafruit_FlashTransport_QSPI();
Adafruit_SPIFlash flash(&flashTransport);

CANFDMessage message;

volatile uint32_t idleCounter = 0;
uint32_t lastIdleCount = 0;
uint32_t lastMillis = 0;
uint32_t lastSampleTime = 0;
const uint32_t maxIdle = 975000;                                                         // Adjust after calibration
float cpuLoadAccum = 0.0f;
uint8_t sampleCount = 0;

extern uint32_t _etext;

void loop()
  {
    uint32_t deltaIdle;
    float idleRatio;
    float loadPercent;

    while(Serial.available()) processchar(Serial.read());                                // Serial command processing if using IDE
// --- Delay slot 0: Toggle Neopixel every 1 second (1000 x 1 ms) ---
    if(delays[0].flag)
      {
        delays[0].flag = false;
        BLINK(RED);

        for(uint8_t led = 0; led < PWM_CHANNELS; led++)                                  // Blink any led if set 
          {
            if(Blink_Mode[led])
              {
                if(blink_flag[led])
                  {
                    Set_PWM(led, 100, 0);                                                // LED on
                    blink_flag[led] = false;
                  }
            else
              {
                Set_PWM(led, 0, 0);                                                       // LED off
                blink_flag[led] = true;
              }
            }
          }
        deltaIdle = idleCounter - lastIdleCount;                                          // Idle loops in last second
        lastIdleCount = idleCounter;                                                      // Save new baseline
        idleRatio = min(1.0f, (float)deltaIdle / maxIdle);                                // Normalize against maxIdle
        loadPercent = (1.0f - idleRatio) * 100.0f;                                        // Invert to get CPU usage
        cpuLoadAccum += loadPercent;                                                      // Accumulate for averaging
        sampleCount++;      
        startDelay(0, 1000);                                                              // Delay before next toggle (per slot 0)
      }

// --- Delay slot 1: Clear NeoPixel every 2 seconds (2000 x 1 ms) ---
    if(delays[1].flag)
      {
        delays[1].flag = false;
        BLINK(BLACK);
        startDelay(1, 2000);
      }

// --- Delay slot 2: Heartbeat and BME every 1 minute (60000 x 1 ms) ---
    if(delays[2].flag)
      {
        delays[2].flag = false;
        sendHeartbeatFrame();
        if(BME_FLAG) readBME();
        float avgLoad = cpuLoadAccum / sampleCount;                                         // Compute average
        if(IDE) Serial.printf("AVERAGE CPU LOAD (60s): %.1f%%\n", avgLoad);                 // Debug output
        cpuLoadAccum = 0;                                                                   // Reset accumulator
        sampleCount = 0;
        startDelay(2, 60000);
      }

// --- Delay slot 3: Send time every hour if IDE is connected (3600000 x 1 ms) ---
    if(delays[3].flag)
      {
        delays[3].flag = false;
        if(IDE) TimeSend();
        startDelay(3, 3600000);
      }

    if(MONITOR_FLAG && IDE)
      {
        while(can1.receiveFD0(message))
          {              
            Serial.print(F("CAN RX FIFO0: "));
            Serial.printf("0x%03X ", message.id); 
            Serial.printf(" %02X ", message.len); 
            Serial.print(F(" "));
            for(uint8_t i = 0; i < message.len; i++) Serial.printf("%02X ", message.data[i]);
            Serial.println();    
          }
      }
    idleCounter++;
  }


