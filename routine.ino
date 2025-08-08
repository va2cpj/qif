
// ~/Arduino/QIF/switch/routine.ino

#include "qif.h"

// CRC64_ECMA-182 computation
// Optimized for slice-by-8 algorithm (processes 8 bytes per iteration)
//

// Function prototypes
void crc64_stream_init(crc64_stream* ctx, uint64_t initial_crc);
void crc64_stream_update(crc64_stream* ctx, const uint8_t* data, size_t length);
uint64_t crc64_stream_finalize(crc64_stream* ctx); 


// Initialize streaming CRC
void crc64_stream_init(crc64_stream* ctx, uint64_t initial_crc)
  {
    ctx->crc = ~initial_crc;
    ctx->buffer_count = 0;
  }

// Process a chunk of data (can be called multiple times)
void crc64_stream_update(crc64_stream* ctx, const uint8_t* data, size_t length)
  {
// Process any bytes left in buffer + new data to make 8-byte chunks
    while(length > 0)
      {
        size_t copy_len = 8 - ctx->buffer_count;
        if(copy_len > length) copy_len = length;       
        memcpy(ctx->buffer + ctx->buffer_count, data, copy_len);
        ctx->buffer_count += copy_len;
        data += copy_len;
        length -= copy_len;
        
// Process full 8-byte chunks immediately
        if(ctx->buffer_count == 8)
          {
            uint64_t word;
            memcpy(&word, ctx->buffer, 8);
            ctx->crc ^= word;
            ctx->crc = crc64_table[(ctx->crc >>  0) & 0xFF] ^
                      crc64_table[(ctx->crc >>  8) & 0xFF] ^
                      crc64_table[(ctx->crc >> 16) & 0xFF] ^
                      crc64_table[(ctx->crc >> 24) & 0xFF] ^
                      crc64_table[(ctx->crc >> 32) & 0xFF] ^
                      crc64_table[(ctx->crc >> 40) & 0xFF] ^
                      crc64_table[(ctx->crc >> 48) & 0xFF] ^
                      crc64_table[(ctx->crc >> 56) & 0xFF];
            ctx->buffer_count = 0;
          }
      }
  }

// Finalize CRC (process remaining bytes and return result)
uint64_t crc64_stream_finalize(crc64_stream* ctx)
  {
// Process remaining bytes
    for(size_t i = 0; i < ctx->buffer_count; i++)
      {
        ctx->crc = (ctx->crc >> 8) ^ crc64_table[(ctx->crc ^ ctx->buffer[i]) & 0xFF];
      }
    return ~ctx->crc;
  }

//----------------------------------------------------------------------------------------
// CRC64-ECMA computation (bitwise implementation)
void crc64_reset(CRC64 *ctx) { ctx->crc = 0xFFFFFFFFFFFFFFFFULL; }                                // CRC64-ECMA computation (bitwise implementation)

//----------------------------------------------------------------------------------------
void crc64_update(CRC64 *ctx, const uint8_t *data, size_t len)
  {
    for (size_t i = 0; i < len; i++)
      {
        ctx->crc ^= (uint64_t)data[i];
        for(int j = 0; j < 8; j++)
          {
            if(ctx->crc & 1) ctx->crc = (ctx->crc >> 1) ^ 0xC96C5795D7870F42ULL;       
            else ctx->crc >>= 1;       
          }
      }
  }

uint64_t crc64_finalize(const CRC64 *ctx) { return ctx->crc; }

//----------------------------------------------------------------------------------------
void Help()
  {
    if(IDE)
      {
        Serial.println();
        Serial.print("LABEL: ");
        Serial.print(LABEL);
        Serial.print("  CAN: 0x");
        Serial.print(CAN_BASE, HEX);
        Serial.print("  TYPE: ");
        Serial.println(typeNames[TYPE]);
        Serial.println(F("HELP          COMMAND:"));
        Serial.println(F("H             PRINT COMMAND LIST"));
        Serial.println(F("I             I2C LOCAL BUS SCAN"));
        Serial.println(F("T             TIME BROADCAST"));
        Serial.println(F("F             FILTER ACTIVE DUMP"));
        Serial.println(F("B (D  D)      BME688 ASK VALUE (LABEL TYPE)"));
        Serial.println(F("A (D  D)      ANALOG ASK VALUE (LABEL CHANNEL)"));
        Serial.println(F("U (D)         UPDATE SEND (LABEL)"));
        Serial.println(F("R (D)         REBOOT BOARD (LABEL)"));
        Serial.println(F("Q (XXX)       QSPI MEMORY DUMP (BLOCK)"));
        Serial.println(F("D (XXX)       FLASH MEMORY DUMP (BLOCK)"));
        Serial.println(F("M TOGGLED     START/STOP MONITOR MODE"));
        Serial.println(F("P (D D D D)   PWM MSG (LBL CHANNEL VALUE DIRECTION)"));
        Serial.println(F("S (D D D)     SEND MSG (LBL SUB VALUE)"));
        Serial.println();
      }
    DELAY(5000);                                                       
  }

//----------------------------------------------------------------------------------------
/*
The I2C scanner uses the return value of
the Write.endTransmission to see if
a device did acknowledge to the address.
*/

void ScanI2C()
  {
    uint8_t found = 0;
    BME688 = 0;                                                         // clear BME flag before scan

    if(IDE)Serial.print(F("SCANNING I2C: "));
    for(uint8_t address = BME688_ADDR_LOW; address <= BME688_ADDR_HIGH; address++)
      {
        if(IDE)Serial.print(".");                                       // Show scan progress
        uint8_t success = 0;
        for(uint8_t attempt = 0; attempt < 3; attempt++)
          {
            Wire.beginTransmission(address);
            if(Wire.endTransmission(true) == 0)
              {
                success++;
                if(success >= 2) break;
              }
          }

        if(success >= 2)
          {
            Wire.requestFrom((int)address, 1);
            if(Wire.available())
              {
                if(address == BME688_ADDR_LOW || address == BME688_ADDR_HIGH)
                  {
                    BME688 = address;
                  }
                if(IDE)Serial.print(F("0x"));
                if(address < 16) if(IDE)Serial.print("0");
                if(IDE)Serial.print(address, HEX);
                found++;
              }
            }
          delay(1);
        }
      if(IDE)Serial.println();
      if(found == 0)
        {
          if(IDE)Serial.println(F("NO I2C DEVICE FOUND"));
        }
  }
  
//----------------------------------------------------------------------------------------
// TimeSend()
// Broadcasts the current time over the CAN FD bus using the following 8-byte format:
//   Byte 0 : Device label (uint8_t)
//   Byte 1 : Year high byte (most significant byte of the 16-bit year)
//   Byte 2 : Year low byte (least significant byte of the 16-bit year)
//   Byte 3 : Month    (1–12)
//   Byte 4 : Day      (1–31)
//   Byte 5 : Hour     (0–23)
//   Byte 6 : Minute   (0–59)
//   Byte 7 : Second   (0–59)
//
// The frame is sent to CAN ID = (0 + Time).
// If IDE mode is enabled, the full decoded timestamp is printed to the serial monitor.
void TimeSend()
  {
    CANFDMessage msg;                                                                            // CAN FD payload buffer
    
    DateTime now = rtc.now();                                                                    // Get current time from RTC
    uint16_t year = now.year();                                                                  // Extract 4-digit year from RTC object

// Fill the CAN FD data payload
    msg.id = SVR + Time;                                                                         // Time channel
    msg.len = 8;
    msg.data[0] = LABEL;                                                                         // Byte 0: devic unique label
    msg.data[1] = (year >> 8) & 0xFF;                                                            // Byte 1: high byte of the year (MSB)
    msg.data[2] = year & 0xFF;                                                                   // Byte 2: low byte of the year (LSB)
    msg.data[3] = now.month();                                                                   // Byte 3: month (1–12)
    msg.data[4] = now.day();                                                                     // Byte 4: day (1–31)
    msg.data[5] = now.hour();                                                                    // Byte 5: hour (0–23)
    msg.data[6] = now.minute();                                                                  // Byte 6: minute (0–59)
    msg.data[7] = now.second();                                                                  // Byte 7: second (0–59)

// Transmit the 8-byte payload on CAN FD to the service ID for time broadcast
    sendCANFDFrame(msg.data, msg.len, msg.id);                                                   // Send frame to CAN ID (SVR + Time)

// Optional debug output if connected to PC/IDE
  if(IDE)
    {
// Reconstruct year from two bytes for printing
      uint16_t y = ((uint16_t)msg.data[1] << 8) | msg.data[2];

      Serial.print(F("TIME BROADCASTED: "));
      Serial.print(y); Serial.print("-");
      Serial.print(msg.data[3] < 10 ? "0" : ""); Serial.print(msg.data[3]); Serial.print("-");
      Serial.print(msg.data[4] < 10 ? "0" : ""); Serial.print(msg.data[4]); Serial.print(" ");
      Serial.print(msg.data[5] < 10 ? "0" : ""); Serial.print(msg.data[5]); Serial.print(":");
      Serial.print(msg.data[6] < 10 ? "0" : ""); Serial.print(msg.data[6]); Serial.print(":");
      Serial.print(msg.data[7] < 10 ? "0" : ""); Serial.print(msg.data[7]);
      Serial.println();
    }
  }

//----------------------------------------------------------------------------------------
void ReadBME(uint8_t label, uint8_t info)                                                                 // Read BME
  {
    CANFDMessage frame;
    
    if(label == 0) 
      {
        if(!BME_FLAG) { if(IDE) Serial.println(F("NO BME ON BOARD")); return; };
        Serial.println(F("READ LOCAL BME")); readBME(); return;                                          // Read local BME
      }                            
    else                                                                                                 // Ask remote BME
      {
        if(label > 127) { if(IDE) Serial.println(F("INVALID LABEL RANGE")); return; };
        frame.id = Lbl2Can(label) + Bme;                                                                 // Construct the CAN FD message
        frame.len = 2;
        frame.data[0] = info;
        frame.data[1] = LABEL;
      }    
    sendCANFDFrame(frame.data, frame.len, frame.id);                                                                        
  }

//----------------------------------------------------------------------------------------
void Reboot(uint8_t label)
  {
    if(IDE)
      {
        Serial.print(F("Rebooting board: "));
        Serial.println(label);
        DELAY(1000);                                                                              // Delay to allow serial flush
      }
    return;                                                                                       // Disable now
    NVIC_SystemReset();                                                                           // Perform software reset
  }

//----------------------------------------------------------------------------------------
// State-based function handlers
//----------------------------------------------------------------------------------------

void processHLP() { Help(); }
void processI2C() { ScanI2C(); }                                                                   // Scan I2C bus
void processTIM() { TimeSend(); }                                                                  // Broadcast time
void processFLT() { filterManager_dump(&filterManager); }                                          // Dump active filter
void processRST(const uint8_t label) { Reboot(label); }                                            // Reboot selected board
void processUPD(const uint8_t label) { QSPI2CAN(label); }                                          // Send update to board label                                 
void processBME(const uint8_t label, uint8_t info) { requestBME(info, label); }                    // Ask for BME688 value from label
void processANA(const uint8_t label, uint8_t channel) { requestANA(label, channel); }              // Ask for analog value from label
void processQSP(const uint8_t block) { DumpQSPI(block); }                                          // Dump QSPI block
void processDMP(const uint8_t block) { dumpInternalFlash(block); }                                 // Dump FLASH block
void processSND(const uint8_t label, uint8_t subaddr, uint8_t value) { SendCan(label, subaddr, value); }
void processPWM(const uint8_t label, uint8_t subaddr, uint8_t value, uint8_t direction) { SendPwm(label, subaddr, value, direction != 0); }

//----------------------------------------------------------------------------------------

void SendCan(uint8_t label, uint8_t subaddr, uint8_t value)
  {
    CANFDMessage msg;
    if(label > 127 || subaddr > 15 || value > 255)                                                  // Argument validation
      {
        if(IDE)Serial.println(F("Invalid Argument"));
        return;                                          
      }
    uint16_t can_id = 0xFFFF;                                                                       // Search DB for matching label
    for(uint8_t i = 0; i < DB_count; i++)
      {
        if (DB[i].LBL == label)
        {
          can_id = DB[i].CAN + subaddr;                                                             // Use base CAN + subaddress offset
          break;
        }
      }
    if(can_id == 0xFFFF)
      {
        if(IDE) Serial.println(F("Label not found in DB"));
        return;
      }
    msg.id = can_id;                                                                                // Prepare message
    msg.len = 1;                                                                                    // Set length to 8 bytes
    msg.data[0] = value;                                                                            // Set the last byte to value
    sendCANFDFrame(msg.data, msg.len, msg.id);  
  }

void SendPwm(uint8_t label, uint8_t channel, uint8_t value, bool direction)
  {
    CANFDMessage msg;

    if(label >= DB_count || channel > 15 || value > 100)  // Basic argument validation
      {
    if(IDE) Serial.println(F("Invalid Argument"));
    return;
  }

    if((DB[label].TYPE == SWITCH && channel >= 8) || (DB[label].TYPE != SWITCH && channel >= 6))       
      {
        if(IDE)
          {
            Serial.print(F("Invalid PWM channel for label "));
            Serial.print(label);
            Serial.print(F(", TYPE: "));
            Serial.println(DB[label].TYPE);
          }
        return;
      }

    msg.id = Lbl2Can(label) + channel;                                                              // Prepare message
    msg.len = 2;                                                                                    // Set length to 2 bytes
    msg.data[0] = value;                                                                            // Set the last byte to value
    msg.data[1] = direction; 
    sendCANFDFrame(msg.data, msg.len, msg.id);  
  }

//----------------------------------------------------------------------------------------
// Monitor
// This function toggles the "monitor mode" used for CAN traffic observation.
// When activated, it saves the current filter configuration, installs a wide-range
// monitor filter, and sets MONITOR_FLAG to true.
// When deactivated, it restores the previously saved filter set.
//
// Dependencies:
// - MONITOR_FLAG (global boolean flag indicating monitor state)
// - SaveFilter(CANFilterManager* mgr)
// - RestoreFilter(CANFilterManager* mgr)
// - MonitorFilter(bool on)
// - filterManager : the global CAN filter manager instance
//
void processMNT() { Monitor(); }                                                                    // Set monitor mode

//----------------------------------------------------------------------------------------
// Execute the current state with its argument
void handlestate()
  {
    switch (State)
      {
        case HLP: { processHLP();                       break; }
        case I2C: { processI2C();                       break; }
        case TIM: { processTIM();                       break; }
        case FLT: { processFLT();                       break; }
        case MNT: { processMNT();                       break; }
        case UPD: { processUPD(Value);                  break; }
        case RST: { processRST(Value);                  break; }
        case BMX: { processBME(Value,Value1);           break; }
        case ANA: { processANA(Value,Value1);           break; }
        case QSP: { processQSP(Value);                  break; }
        case DMP: { processDMP(Value);                  break; }
        case SND: { processSND(Value, Value1, Value2);  break; }
        case MLI: { processPWM(Value, Value1, Value2, Value3);  break; }
        default:
        break;
      } 
    Value = 0; Value1 = 0; Value2 = 0; Value3 = 0; argIndex = 0; State = NONE;                                // Reset input state
  }

//----------------------------------------------------------------------------------------
// Handle incoming characters from serial

void processchar(const uint8_t c)
{
  static char currentState = 0;
  static bool argStarted = false;

  if (isdigit(c))
  {
    uint8_t digit = c - '0';
    argStarted = true;

    switch (argIndex)
    {
      case 0: Value  = Value  * 10 + digit; break;
      case 1: Value1 = Value1 * 10 + digit; break;
      case 2: Value2 = Value2 * 10 + digit; break;
      case 3: Value3 = Value3 * 10 + digit; break;
    }
  }
  else if (c == ' ')
  {
    if (argStarted && argIndex < 3)
    {
      argIndex++;
      argStarted = false;
    }
  }
  else if (c == '\r' || c == '\n')
  {
    // Interpret state and execute
    switch (toupper(currentState))
    {
      case 'H': State = HLP;  break;
      case 'R': State = RST;  break;
      case 'B': State = BMX;  break;
      case 'A': State = ANA;  break;
      case 'I': State = I2C;  break;
      case 'T': State = TIM;  break;
      case 'U': State = UPD;  break;
      case 'F': State = FLT;  break;
      case 'Q': State = QSP;  break;
      case 'D': State = DMP;  break;
      case 'M': State = MNT;  break;
      case 'S': State = SND;  break;
      case 'P': State = MLI;  break;
      default:  State = NONE; break;
    }

    handlestate();

    // Reset state for next command
    currentState = 0;
    Value = Value1 = Value2 = Value3 = 0;
    Value = Value1 = Value2 = 0;
    argIndex = 0;
    argStarted = false;
  }
  else if (isalpha(c))
  {
    // Start new state command
    currentState = c;
    Value = Value1 = Value2 = Value3 = 0;
    argIndex = 0;
    argStarted = false;
  }
}


//----------------------------------------------------------------------------------------
/*
Check the status of the BME688 sensor and handle errors or warnings.
This function checks the status codes of the IAQ sensor (both BSEC and BME68X).
If there are any errors or warnings, it updates the output string with the
corresponding message, prints it to the Serial Monitor if the IDE flag is set,
and sets the LED strip to orange to indicate a failure or warning.
The function distinguishes between errors and warnings based on the status code
values: Errors have negative status codes, while warnings have positive status codes.
*/
void checkIaqSensorStatus(void)
  {
    if(iaqSensor.bsecStatus != BSEC_OK)
      {
        if(iaqSensor.bsecStatus < BSEC_OK)
          {
            output = "BSEC error code : " + String(iaqSensor.bsecStatus);
            if(IDE) Serial.println(output);
            BLINK(ORANGE);                                                                 // Failure                  
          }
        else
          {
            output = "BSEC warning code : " + String(iaqSensor.bsecStatus);
            if(IDE) Serial.println(output);
            BLINK(ORANGE);                  
          }
      }

    if(iaqSensor.bme68xStatus != BME68X_OK)
      {
        if(iaqSensor.bme68xStatus < BME68X_OK)
          {
            output = "BME68X error code : " + String(iaqSensor.bme68xStatus);
            if(IDE) Serial.println(output);
            BLINK(ORANGE);                                                                 // Failure                  
          }
        else
          {
            output = "BME68X warning code : " + String(iaqSensor.bme68xStatus);
            if(IDE)Serial.println(output);
            BLINK(ORANGE);                  
          }
      }
  } 

//---------------------------------------------------------------------------------------- 
/*
Read and process data from the BME sensor.
This function reads data from the BME sensor using the iaqSensor.run() method.
If new data is available, it updates the `BME` structure with the latest sensor
readings, sets an LED strip to yellow, and prints the data to the Serial Monitor
if the IDE flag is set. The function uses atomic operations to safely update
the BME structure.
The function also categorizes the IAQ (Indoor Air Quality) index and prints a
qualitative description of the air quality based on the index value.
return true if new data is available and processed, false otherwise.
*/
boolean readBME()
  {
    if(iaqSensor.run())
      {
        BLINK(YELLOW);                                                                          // New data is available                  

        ATOMIC() LOCK = true;

        BME.co2 = iaqSensor.co2Equivalent;
        BME.voc = iaqSensor.breathVocEquivalent;
        BME.pressure = iaqSensor.pressure / 100;
        BME.temperature = iaqSensor.temperature;
        BME.humidity = iaqSensor.rawHumidity;
        BME.gas = iaqSensor.gasPercentage;
        BME.iaq = iaqSensor.iaq;

        ATOMIC() LOCK = false;

        if(IDE)
          {
            Serial.println("LOCAL BME");
            Serial.print(BME1); Serial.println(BME.temperature);            
            Serial.print(BME2); Serial.println(BME.pressure);
            Serial.print(BME3); Serial.println(BME.humidity);
            Serial.print(BME4); Serial.println(BME.gas);
            Serial.print(BME5); Serial.print(BME.iaq); Serial.print(" ");
            if((int)BME.iaq <= 50) Serial.println(F("Excellent"));
            if((int)BME.iaq > 50 && (int)BME.iaq <= 100) Serial.println(F("Good"));
            if((int)BME.iaq > 100 && (int)BME.iaq <= 150) Serial.println(F("Lightly polluted"));
            if((int)BME.iaq > 150 && (int)BME.iaq <= 200) Serial.println(F("Moderately polluted")); 
            if((int)BME.iaq > 200 && (int)BME.iaq <= 250) Serial.println(F("Heavely polluted")); 
            if((int)BME.iaq > 250 && (int)BME.iaq <= 300) Serial.println(F("Severely polluted")); 
            if((int)BME.iaq > 350) Serial.println(F("Extremely polluted"));
            Serial.print(BME6); Serial.println(BME.voc);
            Serial.print(BME7); Serial.println(BME.co2);
            Serial.println();
          }
        return true;
      }
    else
      {
        checkIaqSensorStatus();
        return false;
      }
  }

void requestBME(uint8_t type, uint8_t label)
  {
    CANFDMessage msg;
    if(label > 127 || type > 36 || type < 30)                                                 // Argument validation
      {
        if(IDE)Serial.println(F("Invalid Argument"));
        return;                                          
      }

    msg.id = Lbl2Can(label);                                                                    
    msg.id += Bme;
    msg.len = 2;                                                                              // info + target
    msg.data[0] = type;
    msg.data[1] = LABEL;                               
    sendCANFDFrame(msg.data, msg.len, msg.id); 
  }

void requestANA(uint8_t label, uint8_t channel)
  {
    CANFDMessage msg;
    if(label > 127 || channel > 3)                                                            // Argument validation
      {
        if(IDE)Serial.println(F("Invalid Argument"));
        return;                                          
      }
    msg.id = Lbl2Can(label);                                                                    
    msg.id += Anl;
    msg.id += channel;
    msg.len = 1;                                                                              // Channel
    msg.data[0] = channel;                           
    sendCANFDFrame(msg.data, msg.len, msg.id); 
  }

//----------------------------------------------------------------------------------------
// sendBME: Sends a single BME measurement over CAN FD to a target board.
//
// Parameters:
//   type   : Measurement type, expected to be one of the enum values:
//            BMETEMP (30), BMEPRESS (31), BMEHUMID (32), BMEGAZ (33),
//            BMEIAQ (34), BMEVOC (35), BMECO2 (36)
//   target : Target device label to receive the measurement
//
// Frame layout (6 bytes total):
//   Byte 0 : Measurement type (BMETEMP–BMECO2)
//   Byte 1-4 : Float value as 4-byte IEEE 754
//   Byte 5 : Sender board label
//
// Returns:
//   true  on success
//   false on error (invalid type, BME not available, or CAN send failure)
//----------------------------------------------------------------------------------------
bool sendBME(uint8_t type, uint8_t target)
{
  CANFDMessage msg;
  BMEData hex;                                                                              // Union to convert float to byte array

  const uint8_t info[]  = { BMETEMP, BMEPRESS, BMEHUMID, BMEGAZ, BMEIAQ, BMEVOC, BMECO2 };
  const float   values[] = { BME.temperature, BME.pressure, BME.humidity,BME.gas, BME.iaq, BME.voc, BME.co2 };

  if (!BME_FLAG) return false;                                                              // Abort if no BME onboard

  uint8_t index = type - BMETEMP;                                                           // Map type (30–36) to array index (0–6)
  const uint8_t maxIndex = sizeof(values) / sizeof(values[0]);                              // Invalid access if type is not recognized
  if (index >= maxIndex) return false;
  // Compose CAN FD message
  msg.id = Lbl2Can(target) + Bme;                                                           // Address + BME offset
  msg.len = 6;                                                                              // 1 type + 4 float + 1 label
  hex.value = values[index];                                                                // Convert float to byte array
  msg.data[0] = type;                                                                       // Measurement type (enum)
  memcpy(&msg.data[1], hex.bytes, 4);                                                       // Float bytes
  msg.data[5] = LABEL;                                                                      // Sender label

  if (!sendCANFDFrame(msg.data, msg.len, msg.id)) return false;
  DELAY(1);                                                                                 // Pacing delay
  return true;
}


//----------------------------------------------------------------------------------------
// Function to send a CAN FD frame (up to 64 bytes)

#define kTryToSendReturnStatusFD_OK                     0                                   // tryToSendReturnStatusFD return code
#define kTryToSendReturnStatusFD_TooLong                1
#define kTryToSendReturnStatusFD_InvalidBitRateSwitch   2
#define kTryToSendReturnStatusFD_InvalidFormat          3
#define kTryToSendReturnStatusFD_InvalidLength          4
#define kTryToSendReturnStatusFD_TxFifoFull             5

//========================================================================================
// sendCANFDFrame: Sends a CAN FD frame with retry logic.
//
// This function constructs a CAN FD frame and attempts to transmit it using the
// ACANFD_FeatherM4CAN driver. It retries up to 10 times on any transmission failure,
// including cases such as FIFO full, invalid format, or length issues. Each error is
// reported after the final failed attempt if debugging is enabled (IDE = true).
//
// Parameters:
//   - data: Pointer to the data buffer (max 64 bytes, actual length defined by `len`)
//   - len : Payload length (must be <= 64, valid CAN FD payload size)
//   - id  : 11-bit standard CAN ID (0x000 to 0x7FF)
//
// Note:
//   - Requires the CAN controller to be initialized and started (CAN_Setup()).
//   - The frame type is set to CAN FD with bit rate switching.
//
//========================================================================================

bool sendCANFDFrame(const uint8_t* data, uint8_t len, uint16_t id)
{
  if(MONITOR_FLAG) return false;                                                          // Do not send anything while in monitor mode
  CANFDMessage frame;                                                                     // Frame to be transmitted
  uint8_t status;                                                                         // Status returned by tryToSendReturnStatusFD
  uint8_t attempt = 0;                                                                    // Retry counter

  //-------------------------------
  // Prepare CAN FD frame
  //-------------------------------
  frame.id = id;                                                                          // Standard 11-bit CAN ID
  frame.ext = false;                                                                      // Use standard frame format (not extended)
  frame.len = len;                                                                        // Set payload length
  frame.type = CANFDMessage::CANFD_WITH_BIT_RATE_SWITCH;                                  // Enable CAN FD with bit rate switch
  frame.idx = 0;                                                                          // Use default TX FIFO index

  // Copy data into frame payload
  for (uint8_t i = 0; i < len; i++) {
    frame.data[i] = data[i];
  }

  //-------------------------------
  // Debug: Print frame content
  //-------------------------------
  if (IDE) {
    Serial.print(F("SENDING CANFD FRAME - ID: 0x"));
    Serial.print(id, HEX);
    Serial.print(F(" LEN: "));
    Serial.print(len);
    Serial.print(F(" DATA: "));
    for (uint8_t i = 0; i < len; i++) {
      if (data[i] < 0x10) Serial.print('0');
      Serial.print(data[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
  }

  //-------------------------------
  // Attempt to send (up to 10 times)
  //-------------------------------
  do {
    status = can1.tryToSendReturnStatusFD(frame);                         // Attempt transmission
    if (status == kTryToSendReturnStatusFD_OK) {
      BLINK(GREEN);
      return true;                                                        // Success: exit early
    }
    attempt++;                                                            // Count failed attempt
    DELAY(random(1, 11));                                                 // Random delay between 1 and 10 ms before retry
  } while (attempt < 10);                                                 // Limit to 10 tries
  BLINK(RED);
  //-------------------------------
  // If still failed after retries, print error
  //-------------------------------
  if (IDE) {
    Serial.print(F("CAN FD send failed after "));
    Serial.print(attempt);
    Serial.print(F(" attempt(s)! Error code: "));
    Serial.print(status);
    Serial.print(F(" → "));

    switch (status) {
      case kTryToSendReturnStatusFD_TooLong:
        Serial.println(F("Frame too long"));
        break;
      case kTryToSendReturnStatusFD_InvalidBitRateSwitch:
        Serial.println(F("Invalid bit rate switch usage"));
        break;
      case kTryToSendReturnStatusFD_InvalidFormat:
        Serial.println(F("Invalid frame format"));
        break;
      case kTryToSendReturnStatusFD_InvalidLength:
        Serial.println(F("Invalid payload length"));
        break;
      case kTryToSendReturnStatusFD_TxFifoFull:
        Serial.println(F("Transmit FIFO full"));
        break;
      default:
        Serial.println(F("Unknown error"));
        break;
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------
uint64_t uuid2uid()
  {
    uint64_t id = 0;
 
    id |= ((uint64_t)UniqueID[0] << 40);
    id |= ((uint64_t)UniqueID[1] << 32);
    id |= ((uint64_t)UniqueID[2] << 24);
    id |= ((uint64_t)UniqueID[3] << 16);
    id |= ((uint64_t)UniqueID[4] << 8);
    id |= UniqueID[5];
    return id;
  }

//----------------------------------------------------------------------------------------
void PrintHex8(uint8_t data)                                                                      // print 8-bit data in hex with leading zeroes
  {
    Serial.print("0x");
    if(data < 0x10) { Serial.print('0'); }                                                        // Add a leading zero for single-digit values
    Serial.print(data, HEX);                                                                      // Print the 8-bit value in hexadecimal format
  }

//----------------------------------------------------------------------------------------
 void PrintHex16(uint16_t data)
  {
    uint8_t MSB = (data >> 8) & 0xFF;                                                             // Extract the most significant byte
    uint8_t LSB = data & 0xFF;                                                                    // Extract the least significant byte

    Serial.print("0x");
    if(MSB < 0x10) { Serial.print('0'); }                                                         // Add a leading zero for single-digit MSB
    Serial.print(MSB, HEX);
    if(LSB < 0x10) { Serial.print('0'); }                                                         // Add a leading zero for single-digit LSB
    Serial.print(LSB, HEX);                                                                       // Print LSB
  }

//----------------------------------------------------------------------------------------
void PrintHex32(uint32_t data)
  {
    uint8_t MSB1 = (data >> 24) & 0xFF;                                                           // Extract the most significant byte
    uint8_t MSB2 = (data >> 16) & 0xFF;                                                           // Extract the next most significant byte
    uint8_t LSB1 = (data >> 8) & 0xFF;                                                            // Extract the second least significant byte
    uint8_t LSB2 = data & 0xFF;                                                                   // Extract the least significant byte

    Serial.print("0x");
    if(MSB1 < 0x10) { Serial.print('0'); }
    Serial.print(MSB1, HEX);
    if(MSB2 < 0x10) { Serial.print('0'); }
    Serial.print(MSB2, HEX);
    Serial.print(" ");
    if(LSB1 < 0x10) { Serial.print('0'); }
    Serial.print(LSB1, HEX);
    if(LSB2 < 0x10) { Serial.print('0'); }
    Serial.print(LSB2, HEX);                                                                      // Print LSB2
  }

//----------------------------------------------------------------------------------------
void PrintHex64(uint64_t data)
  {
    int8_t shift;
    uint8_t nibble;

    Serial.print("0x");
    for(shift = 60; shift >= 0; shift -= 4)                                                       // Process each nibble (4 bits at a time)
      { 
        nibble = (data >> shift) & 0xF;
        if(nibble < 10) { Serial.print((char)('0' + nibble)); } else { Serial.print((char)('A' + (nibble - 10))); }
      }
  }

//----------------------------------------------------------------------------------------
uint16_t getCAN(uint64_t uid)                                                                     // Return can base address from the board UID
  {
    const IO *ptr = DB;                                                                           // Define a pointer to the DB array

    for(uint16_t i = 0; i < DB_count; i++)                                                        // Search for the structure containing the board UID
      { 
        if (ptr->UID == uid) { return ptr->CAN; }                 
        ptr++;                                                                                    // Next element in the array                                                                
      }
    return false;
  }

//----------------------------------------------------------------------------------------
uint8_t getLBL(uint64_t uid)                                                                      // Return label from the board UID                                                            
  {
    for(uint8_t i = 0; i < DB_count; i++)
      {
        if(DB[i].UID == uid) return DB[i].LBL;
      }
    return false;
  }

//----------------------------------------------------------------------------------------
uint8_t getTYPE(uint64_t uid)                                                                     // Return type from the board UID
  {
    const IO *ptr = DB;                                                                           // Define a pointer to the DB array

    for(uint16_t i = 0; i < DB_count; i++)                                                        // Search for the structure containing the board UID
      { 
        if (ptr->UID == uid) { return ptr->TYPE; }                 
        ptr++;                                                                                    // Next element in the array                                                                
      }
    return false;
  }

uint16_t Lbl2Can(uint8_t lbl)                                                                     // Extract can address from label
  {
    for(uint8_t i = 0; i < DB_count; i++)
      {
        if(DB[i].LBL == lbl)
        return DB[i].CAN;                                                                         // Found CAN address
      }
    return 0xFFFF;                                                                                // Not found
  }

// ----------------------------------------------------------------------------------
// Print a CAN FD frame (like the message.data[] array) in hexadecimal format
void PrintCANFrameHex(const uint8_t *frame, uint8_t len = 8)
{
  for (uint8_t i = 0; i < len; i++) {
    Serial.print("0x");
    if (frame[i] < 0x10) Serial.print("0");
    Serial.print(frame[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

//----------------------------------------------------------------------------------------
/**
 * @brief Transmits a firmware image stored in QSPI to a remote device via CAN FD.
 *
 * This function reads a firmware binary previously copied to external QSPI flash memory
 * (starting at offset 0x00000000), and transmits it using 8-byte CAN FD frames.
 * It handles the full STX → DATA → CRC64 → ETX sequence, and verifies the CRC on the receiver side.
 *
 * Highlights:
 *   - Scans QSPI to determine actual firmware size (based on end marker: 0xFF-filled blocks)
 *   - Sends data in 8-byte CAN FD frames
 *   - Computes CRC64 on-the-fly and appends it
 *   - Ends with an obfuscated ETX marker
 *
 * Assumptions:
 *   - QSPI firmware starts at offset 0x00000000
 *   - Final 4KB of QSPI is padded with 0x00 (used to detect end)
 *   - Each QSPI block is 4096 bytes
 *
 * @param label Target device label (must match device database and not self)
 * @return true if transmission succeeded, false if aborted or failed
 */
 bool QSPI2CAN(uint8_t label)
{
  const uint32_t block_size   = QSPI_BLOCK_SIZE;  // 4 KB per block
  const uint32_t chunk_size   = 8;                // CAN FD frame size
  const uint32_t flash_offset = 0x00000000;
  uint8_t buffer[block_size];
  uint32_t offset = 0;
  crc64_stream ctx;

  if(IDE) Serial.println(F("NOT YET IMPLEMENTED"));
  return false;

  if (label == 0 || label == LABEL)
  {
    if(IDE) Serial.println(F("QSPI2CAN aborted: invalid or self-addressed label ❌"));
    return false;
  }

  bool found = false;
  for (uint8_t i = 0; i < DB_count; i++)
  {
    if (DB[i].LBL == label) { found = true; break; }
  }
  if (!found)
  {
    if (IDE)
    {
      Serial.print(F("QSPI2CAN aborted: label ")); Serial.print(label);
      Serial.println(F(" not found in DB ❌"));
    }
    return false;
  }

  // Detect actual firmware size in QSPI (stop on 0x00-filled block)
  uint32_t program_size = 0;
  while (true)
  {
    if (!flash.readBuffer(flash_offset + program_size, buffer, block_size)) break;
    bool all_00 = true;
    for (uint32_t i = 0; i < block_size; i++)
    {
      if (buffer[i] != 0x00) { all_00 = false; break; }
    }
    if (all_00) break;
    program_size += block_size;
  }

  uint32_t aligned_limit = ((program_size + block_size - 1) / block_size) * block_size;

  if (IDE)
  {
    Serial.println(F("QSPI to CAN FD transfer started ℹ️"));
    Serial.print(F("Program size        : ")); Serial.println(program_size);
    Serial.print(F("Aligned block limit : ")); Serial.println(aligned_limit);
  }

  sendControlMarker(obfuscate(stx), label);
  crc64_stream_init(&ctx, 0);

  while (offset < aligned_limit)
  {
    memset(buffer, 0xFF, block_size);
    uint32_t valid_bytes = (offset + block_size <= program_size) ? block_size : (program_size - offset);
    if (!flash.readBuffer(flash_offset + offset, buffer, valid_bytes)) return false;

    for (uint32_t i = 0; i < block_size; i += chunk_size)
    {
      uint8_t frame[chunk_size];
      memcpy(frame, &buffer[i], chunk_size);

      sendCANFDFrame(frame, chunk_size, SVR + Update);
      crc64_stream_update(&ctx, frame, chunk_size);
      DELAY(5);

      if (((offset + i) / chunk_size) % 128 == 0) { BLINK(MAGENTA); if (IDE) Serial.print("."); }
      if (((offset + i) / chunk_size) % 256 == 0) BLINK(BLACK);
    }

    offset += block_size;
  }

  // Final CRC64 transmission
  uint64_t crc64_val = crc64_stream_finalize(&ctx);
  uint8_t crc_frame[8];
  for (uint8_t i = 0; i < 8; i++) crc_frame[i] = (crc64_val >> (8 * i)) & 0xFF;

  sendCANFDFrame(crc_frame, 8, SVR + Update);

  if (IDE)
  {
    Serial.println(); Serial.print(F("CRC64: "));
    PrintHex64(crc64_val); Serial.println();
  }

  sendControlMarker(obfuscate(etx), label);
  BLINK(BLACK);
  return true;
}

//----------------------------------------------------------------------------------------
/*
AlarmMatch: Sets an alarm for the next minute.
This function sets a flag (SEC60_FLAG) to indicate an alarm match condition
and then sets an alarm for exactly one minute from the current time.
It retrieves the current date and time using the rtc.now() function,
calculates the new alarm time, and sets the alarm.
Finally, it enables the alarm with a match on seconds and minutes.
flag: A flag parameter, which is not used within the function but may be used for 
other purposes outside the scope of this function.
*/
void alarmMatch(uint32_t flag)
  {                                                                            
    now = rtc.now();
    alarm = DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute() + 1, now.second());               
    rtc.setAlarm(0,alarm);                                                                     // Set alarm for next minute                                                            
    rtc.enableAlarm(0, rtc.MATCH_SS);
  }

//----------------------------------------------------------------------------------------
// CAN Filter Manager (static-size array)
void filterManager_init(CANFilterManager* mgr) {                                               // Initializes the filter manager (clears all entries)
  mgr->count = 0;
  for (int i = 0; i < MAX_FILTERS; i++) {
    mgr->entries[i].valid = false;
  }
}

//----------------------------------------------------------------------------------------
// Filter manager
bool filterManager_add(CANFilterManager* mgr, uint16_t idStart, uint16_t idEnd,
                       ACANFD_FeatherM4CAN_FilterAction action, ACANFDCallBackRoutine cb) 
{
  if(mgr->count >= MAX_FILTERS) return false;

  for(int i = 0; i < MAX_FILTERS; i++)
  {
    if (!mgr->entries[i].valid)
    {
      mgr->entries[i].idStart = idStart;
      mgr->entries[i].idEnd = idEnd;
      mgr->entries[i].action = action;
      mgr->entries[i].callback = cb;
      mgr->entries[i].valid = true;
      mgr->count++;
      return true;
    }
  }

// Add to first empty slot
  if(mgr->count >= MAX_FILTERS) return false;                                                // No space left
  for(int i = 0; i < MAX_FILTERS; i++) {
    if(!mgr->entries[i].valid) {
      mgr->entries[i].idStart = idStart;
      mgr->entries[i].idEnd = idEnd;
      mgr->entries[i].action = action;
      mgr->entries[i].callback = cb;
      mgr->entries[i].valid = true;
      mgr->count++;
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------
// Removes a filter by ID range
bool filterManager_remove(CANFilterManager* mgr, uint16_t idStart, uint16_t idEnd)
  {
    for(int i = 0; i < MAX_FILTERS; i++)
      {
        if(mgr->entries[i].valid && mgr->entries[i].idStart == idStart && mgr->entries[i].idEnd == idEnd)
          {
            mgr->entries[i].valid = false;
            mgr->count--;
            return true;
          }
      }
    return false;
  }

//----------------------------------------------------------------------------------------
// Remove a filter entry by its index number (0 to MAX_FILTERS-1)
// USAGE: filterManager_removeByIndex(&myFilterManager, 3);
bool filterManager_removeByIndex(CANFilterManager* mgr, uint8_t index)
  {
    if(index >= MAX_FILTERS)                                                                // Bounds check
      {
        if(IDE)
          {
            Serial.print(F("Invalid filter index: "));
            Serial.println(index);
          }
        return false;
      }

// Check if the entry at the given index is valid
    if(mgr->entries[index].valid)
      {
        mgr->entries[index].valid = false;
        if(mgr->count > 0) mgr->count--;
    
        if(IDE)
          {
            Serial.print(F("Filter #"));
            Serial.print(index);
            Serial.println(F("removed successfully ✅"));
          }
        return true;
      }
    else
      {
        if(IDE)
          {
            Serial.print(F("No valid filter at index "));
            Serial.print(index);
            Serial.println(F(" to remove 🚫"));
          }
        return false;
      }
  }

//----------------------------------------------------------------------------------------
// Removes all filters from the manager
void filterManager_clear(CANFilterManager* mgr)
  {
    for(int i = 0; i < MAX_FILTERS; i++) { mgr->entries[i].valid = false; }
    mgr->count = 0;
  }

//----------------------------------------------------------------------------------------
// Applies all valid filters to the CAN controller

bool filterManager_apply(CANFilterManager* manager, ACANFD_FeatherM4CAN* can, ACANFD_FeatherM4CAN_Settings* settings)
{
  if (!manager || !can || !settings) {
    if (IDE) Serial.println(F("ERROR: Null pointer in filterManager_apply()"));
    return false;
  }

  ACANFD_FeatherM4CAN::StandardFilters stdFilters;

  uint8_t count = 0;
  for (uint8_t i = 0; i < MAX_FILTERS; i++) {
    if (manager->entries[i].valid) {
      stdFilters.addRange(manager->entries[i].idStart,
                          manager->entries[i].idEnd,
                          manager->entries[i].action,
                          manager->entries[i].callback);
      count++;
    }
  }

  if (count == 0) {
    if (IDE) Serial.println(F("WARNING: No filters defined to apply"));
  }

  uint32_t errorCode = can->beginFD(*settings, stdFilters);
  if (errorCode == 0) {
    if (IDE) {
      Serial.println(F("CAN           INITIALIZED"));
      Serial.print(F("FILTER COUNT  "));
      Serial.println(count);
    }
    return true;
  } else {
    if (IDE) {
      Serial.print(F("CAN INIT FAILED ERROR: 0x"));
      Serial.println(errorCode, HEX);
    }
    return false;
  }
}

//----------------------------------------------------------------------------------------
// Monitor()
// Toggles monitor mode ON/OFF
//
// When enabled:
//   - Saves the current filter configuration.
//   - Clears all filters and installs a wide-open filter (0x000–0x7FF).
//   - Routes traffic to FIFO0 handled manually in loop().
//
// When disabled:
//   - Restores the previously saved filter configuration.
//   - Clears the monitor mode flag.
//
// Outputs status over Serial if IDE is defined.
//----------------------------------------------------------------------------------------

void Monitor() 
  {
    if(!MONITOR_FLAG)                                                               // Enable monitor mode
      {   
        memcpy(&savedFilterManager, &filterManager, sizeof(CANFilterManager));      // Backup active filters
        filterManager_clear(&filterManager);                                        // Clear all filters

        filterManager_add(&filterManager,
                      0x000, 0x7FF,                                                 // Accept all 11-bit CAN IDs
                      ACANFD_FeatherM4CAN_FilterAction::FIFO0,
                      nullptr);                                                     // No callback; handled manually in loop()

        bool ok = filterManager_apply(&filterManager, &can1, &settings);            // Apply new filter
        if(!ok && IDE) Serial.println(F("Monitor filter not applied!"));

        MONITOR_FLAG = true;

        if(IDE)
          {
            Serial.println(F("MONITOR MODE  ENABLED"));
            Serial.println(F("DISPATCHING   FIFO0"));
          }
      }
    else
      {                                                                             // Disable monitor mode
        memcpy(&filterManager, &savedFilterManager, sizeof(CANFilterManager));      // Restore previous filters
        filterManager_apply(&filterManager, &can1, &settings);

        MONITOR_FLAG = false;

        if(IDE)
          {
            Serial.println(F("MONITOR MODE  DISABLED"));
            Serial.println(F("FILTER        RESTORED"));
          }
      }
  }

struct NamedCallback                                                                // Define a struct for function pointer + name association
  {
    FilterCallback fn;
    const __FlashStringHelper *name;
  };

// Declare the table of known callbacks
const NamedCallback namedCallbacks[] = {
  { Process_Time,         F("Process_Time") },
  { Process_Reboot,       F("Process_Reboot") },
  { Process_Update,       F("Process_Update") },
  { Process_BME,          F("Process_BME") },
  { Process_Alarm_BME,    F("Process_Alarm_BME") },
  { Process_Heart_Beat,   F("Process_Heart_Beat") },
  { Process_ACK,          F("Process_ACK") },
  { Process_NACK,         F("Process_NACK") },
  { Process_Led,          F("Process_Led") },
  { Process_GPS,          F("Process_GPS") },
  { Process_Gyro,         F("Process_Gyro") },
  { Process_Level,        F("Process_Level") },
  { Process_Pir,          F("Process_Pir") },
  { Process_Lpwm,         F("Process_Lpwm") },
  { Process_Hpwm,         F("Process_Hpwm") },
  { Process_PwrCtrl,      F("Process_PwrCtrl") },
  { Process_Analog,       F("Process_Analog") },
  { Process_Isense,       F("Process_Isense") },
  { Process_Analog_RX,    F("Process_Analog_RX") },                                       
  { NULL,                 F("Unknown") }                                              // Fallback
};

const __FlashStringHelper* callbackName(FilterCallback cb) {
  for (uint8_t i = 0; namedCallbacks[i].fn != NULL; i++) {
    if (namedCallbacks[i].fn == cb) {
      return namedCallbacks[i].name;
    }
  }
  return F("Unknown");
}

//----------------------------------------------------------------------------------------
// Print all currently active CAN filters stored in the filter manager
void filterManager_dump(const CANFilterManager* mgr)
  {
    if(mgr == NULL || mgr->count == 0)
      {
        if(IDE)Serial.println(F("No active filters"));
        return;
      }

    if(IDE)
      {
        Serial.print(F("FILTER COUNT: "));
        Serial.println(mgr->count);
        Serial.println(F("ACTIVE CAN FILTERS:"));
      }
    for(uint8_t i = 0; i < MAX_FILTERS; i++)
      {
        if(mgr->entries[i].valid)
          {
            if(IDE)
              {
                Serial.print(F("Filter #"));
                if(i < 10) Serial.print(0);
                Serial.print(i);
                Serial.print(F(": ID range "));
                PrintHex16(mgr->entries[i].idStart);
                Serial.print(F(" - "));
                PrintHex16(mgr->entries[i].idEnd);
                Serial.print(F(", Action: "));
              }
// Only use valid known enum values
          switch(mgr->entries[i].action)
            {
              case ACANFD_FeatherM4CAN_FilterAction::REJECT:
                if(IDE)Serial.print(F("REJECT "));
                break;
              case ACANFD_FeatherM4CAN_FilterAction::FIFO0:
                if(IDE)Serial.print(F("FIFO0 "));
                break;
              case ACANFD_FeatherM4CAN_FilterAction::FIFO1:
                if(IDE)Serial.print(F("FIFO1 "));
                break;
              default:
                if(IDE)Serial.print(F("UNKNOWN"));
                break;
            }
      if(mgr->entries[i].callback != NULL)
        {
          if(IDE) Serial.print(F("Yes -> "));
          if(IDE) Serial.println(callbackName(mgr->entries[i].callback));
        } else
        {
          if(IDE) Serial.println(F("No"));
        }
      }
    }
  if(IDE) Serial.println();
}

//----------------------------------------------------------------------------------------
/*
 * Dumps a 4096-byte block (4 KB) of QSPI external flash memory in hexadecimal and ASCII format.
 *
 * Each block is assumed to be 4 KB (4096 bytes, 0x1000).
 * This function reads the block line-by-line, 16 bytes per line, and prints:
 *   - The memory address
 *   - The hexadecimal representation (16 bytes)
 *   - The ASCII printable characters ('.' shown for non-printables)
 *
 * Parameters:
 *   block - The block number to dump (0 = address 0x0000, 1 = address 0x1000, etc.)
 *
 * Notes:
 * - If reading fails at any point, a failure message is printed and the function exits early.
 * - Only runs if IDE mode (serial output) is enabled.
 * - Each line represents 16 bytes.
 * - Useful for visual inspection of QSPI flash content.
 */
void DumpQSPI(uint8_t block)
  {
    uint32_t address = (uint32_t)block * 4096;                                                // Each block is 4096 bytes (0x1000), calculate starting address
    uint8_t buffer[16];                                                                       // 16-byte temporary buffer for each line

    if(!IDE) return;
    Serial.print(F("QSPI DUMP — 4KB Block "));
    Serial.print(block);
    Serial.print(F(" (start address 0x"));
    Serial.print(address, HEX);
    Serial.println(F("):"));
 
  for(uint16_t offset = 0; offset < QSPI_BLOCK_SIZE; offset += 16)                            // Loop through 4096 bytes in 16-byte steps
    {
      if(!flash.readBuffer(address + offset, buffer, sizeof(buffer)))                         // Read 16 bytes into buffer
        {
          Serial.print(F("Failed to read at offset 0x"));
          Serial.println(address + offset, HEX);
          return;
        }
      Serial.print("0x");
      if(address + offset < 0x10000) Serial.print("0");                                   // Padding
      Serial.print(address + offset, HEX);
      Serial.print(": ");
      for(uint8_t i = 0; i < 16; i++)                                                         // Print 16 hex bytes
        {
          if(buffer[i] < 0x10) Serial.print('0');
          Serial.print(buffer[i], HEX);
          Serial.print(' ');
        }
      Serial.print(" |");                                                                     // ASCII separator
      for(uint8_t i = 0; i < 16; i++)                                                         // Print ASCII representation
        {
          char c = (buffer[i] >= 32 && buffer[i] <= 126) ? buffer[i] : '.';
          Serial.print(c);
        }
      Serial.println('|');
    }
    Serial.println();
  }

//----------------------------------------------------------------------------------------
// Dump 4 KB Block of Internal Flash Memory (block 0 = address 0x00004000)
//----------------------------------------------------------------------------------------
/*
  This function reads and dumps a single 4 KB block from internal flash memory,
  starting at a calculated address based on the block number provided.

  Flash memory layout assumptions:
    - Absolute user program base address: 0x00004000
    - Each block = 4 KB (4096 bytes)
    - Block 0 = 0x00004000
    - Block 1 = 0x00005000
    - Block 2 = 0x00006000
    ...

  Parameters:
    - block : uint8_t
        0-based index of the block to dump relative to 0x4000.

  Behavior:
    - Prints each line of memory:
      * 16 bytes per line (hex values)
      * Followed by ASCII printable characters.

  Notes:
    - Serial output enabled only if IDE = true.
*/

#define BLOCK_SIZE     4096

void dumpInternalFlash(uint8_t block)
  {
    if(!IDE) return;
    uint32_t base_address = FLASH_BASE_ADDR + (uint32_t)(block) * BLOCK_SIZE;
    const uint8_t* ptr = (const uint8_t*)base_address;

    if(block > 125) return;                                                                     // Prevent overflow
    Serial.print(F("Dumping 4 KB internal flash block "));
    Serial.print(block);
    Serial.print(F(" (start address 0x"));
    Serial.print(base_address, HEX);
    Serial.println(F("):"));
    for(uint32_t offset = 0; offset < BLOCK_SIZE; offset += BYTES_PER_LINE)                     // Iterate over 4096 bytes
      {
        Serial.print(F("0x"));
        Serial.print(base_address + offset, HEX);
        Serial.print(F(": "));
        for(uint8_t i = 0; i < BYTES_PER_LINE; i++)
          {
            uint8_t val = ptr[offset + i];
            if(val < 0x10) Serial.print('0');
            Serial.print(val, HEX);
            Serial.print(' ');
          }
        Serial.print("|");
        for(uint8_t i = 0; i < BYTES_PER_LINE; i++)
          {
            char c = ptr[offset + i];
            Serial.print(isPrintable(c) ? c : '.');
          }
        Serial.println("|");
      }
  }

//----------------------------------------------------------------------------------------
// Send a Heartbeat Frame over CAN FD
// The frame is sent to CAN ID SVR + Hbt.
void sendHeartbeatFrame()
  {
    CANFDMessage msg;
    msg.id = SVR + Hbt;
    msg.len = 1; 
    msg.data[0] = LABEL;                                                              
    sendCANFDFrame(msg.data, msg.len, msg.id);                                             // Send heartbeat on CAN SVR + Hbt
  }

//----------------------------------------------------------------------------------------
// Send a acknowledge Frame over CAN FD using the predefined ACK pattern
// with the LSB byte replaced by the current device label.
// The frame is sent to CAN ID SVR + Ack.
void Send_Ack()
  {
    uint64_t ack = ACK;                                                                     // Start with the predefined ACK marker
    uint8_t data[8];
    data[0] = LABEL;                                                                        // Put label directly in first byte
    for(uint8_t i = 1; i < 8; i++)
      {
        data[i] = (ack >> (8 * (i - 1))) & 0xFF;                                            // Store remaining 56 bits (low to high)
      }
    sendCANFDFrame(data, 8, SVR + Ack);                                                     // Send to ACK ID
  }

//----------------------------------------------------------------------------------------
// Send a non acknowledge Frame over CAN FD using the predefined NACK pattern
// with the LSB byte replaced by the current device label.
// The frame is sent to CAN ID SVR + Nack.
void Send_Nack()
  {
    uint64_t nack = NAK;                                                                    // Start with the predefined ACK marker
    uint8_t data[8];
    data[0] = LABEL;                                                                        // Put label directly in first byte
    for(uint8_t i = 1; i < 8; i++)
      {
        data[i] = (ack >> (8 * (i - 1))) & 0xFF;                                            // Store remaining 56 bits (low to high)
      }
    sendCANFDFrame(data, 8, SVR + Nack);                                                    // Send to NACK ID
  }

//----------------------------------------------------------------------------------------  
// Erase the entire QSPI flash chip.
// This will wipe all data stored in external flash!
bool eraseQSPI()
  { 
    if(IDE) Serial.println(F("ERASING entire QSPI flash 🔄"));                              // Ensure flash is initialized
    if(!flash.begin())
      {
        Serial.println(F("QSPI flash not initialized 🚫"));
        return false;
      }
    if(flash.eraseChip())
      {
        if(IDE) Serial.println(F("QSPI flash erase complete ✅"));                          // Call chip erase
        return true;
      }
    else
      {
        if(IDE) Serial.println(F("Failed to erase QSPI flash ❌"));
        return false;
      }       
  }

//----------------------------------------------------------------------------------------
// Interrupt handler for Timer/Counter Control 2 (TCC2) overflow. 
// This interrupt handler is triggered when TCC2 overflows.
// Used by the timer pool
//----------------------------------------------------------------------------------------

void TCC2_0_Handler()                                                                        // Interrupt set every 1 ms
  {
    if(TCC2->INTFLAG.bit.OVF)                                                             
      {
        TCC2->INTFLAG.bit.OVF = TCC_INTFLAG_OVF;                                             // Clear the overflow flag
        static uint8_t tickDivider = 0;
        tickDivider++;
    
        for(uint8_t i = 0; i < MAX_TIMERS; i++)
          {
            if(delays[i].active && delays[i].counter > 0)
              {
                delays[i].counter--;
                if(delays[i].counter == 0)
                  {
                    delays[i].flag = true;                                                   // Mark task as completed
                    delays[i].active = false;                                                // Auto-disable after expiry
                  }
              }
          }

        if(tickDivider >= 10)
          {
            tickDivider = 0;                                                                 // Only call every 10 ms
            if(TYPE == SWITCH) Switch_Handler();                                             // Only call switch handler if this board is a SWITCH type
            UpdatePWMResume();                                                               // Check for motors that need to restart after direction change
          }
        can1.dispatchReceivedMessage();
      }
  }

//----------------------------------------------------------------------------------------
// Switch_Handler: Polls each switch, debounces, and detects click types (short, long, double, etc.)
// Designed for N switches connected to GPIO pins defined in switchPins[]
// Uses state machine per switch to detect:
//   - Short clicks
//   - Long presses
//   - Very long presses
//   - Single & double short clicks
//----------------------------------------------------------------------------------------

void Switch_Handler() {
  for (int n = 0; n < N; n++) {                       // Iterate over all N switches
    int val = digitalRead(switchPins[n]);             // Read the current level on this switch pin    
    switchState[n].state = (val == LOW) ? 1 : 0;      // Update switch state: 1 = pressed (LOW), 0 = released (HIGH)

    if (switchState[n].state == 1) {                  // If switch is currently pressed
      switchState[n].tim++;                           // Increment press duration counter
      switchState[n].wait = 0;                        // Reset wait counter since button is held down
    } else {                                          // If switch is released
      switchState[n].wait++;                          // Increment wait counter (time since release)

      if (switchState[n].wait > WAIT_RESET) {         // If released long enough, reset all state tracking
        switchState[n].cnt = 0;
        switchState[n].longCnt = 0;
        switchState[n].shortDetected = 0;
        switchState[n].shortDelay = 0;
      }


      if (switchState[n].tim > VERY_LONG) {           // Check if it was a very long press
        Send_Click(n, CLICK_VL);                      // Trigger very long click event
        switchState[n].tim = 0;                       // Reset all tracking for this switch
        switchState[n].cnt = 0;
        switchState[n].longCnt = 0;
        switchState[n].shortDetected = 0;
        switchState[n].shortDelay = 0;
      }
      // Check if it was a long press (not very long)
      else if (switchState[n].tim > SHORT) {
        switchState[n].longCnt++;                    // Count long press events

        if (switchState[n].longCnt == 1 && switchState[n].cnt == 0) {
          Send_Click(n, CLICK_L);                    // Send long press event
        } else if (switchState[n].cnt == 1) {
          Send_Click(n, CLICK_SL);                   // If we had a short click before, interpret as short + long
        }

        // Reset short click tracking after long press
        switchState[n].cnt = 0;
        switchState[n].shortDetected = 0;
        switchState[n].shortDelay = 0;
        switchState[n].tim = 0;
        switchState[n].wait = 0;
      }
      // Handle quick short clicks
      else if (switchState[n].tim <= SHORT && switchState[n].tim > 0) {
        if (switchState[n].wait == 2) {              // Small debounce: check two ticks after release
          switchState[n].shortDetected = 1;          // Mark that a short click is pending
          switchState[n].shortDelay = 0;
          switchState[n].cnt++;                      // Increment count of short clicks
          switchState[n].tim = 0;
          switchState[n].longCnt = 0;
        }
      }

      // Handle timing window for short & double short clicks
      if (switchState[n].shortDetected) {
        switchState[n].shortDelay++;                 // Increment short click delay timer

        if (switchState[n].shortDelay >= SHORT_DELAY_LIMIT * 2) {
          if (switchState[n].cnt == 1) {
            Send_Click(n, CLICK_S);                  // Single short click
          } else if (switchState[n].cnt == 2) {
            Send_Click(n, CLICK_SS);                 // Double short click
          }
          // Reset after handling
          switchState[n].shortDetected = 0;
          switchState[n].shortDelay = 0;
          switchState[n].cnt = 0;
        }
      }
    }
  }
}

void Send_Click(uint8_t n, ClickValue value)
{
  uint8_t param = 0;

  switch (value) {
    case CLICK_S:  param = CLICK_S;  break;
    case CLICK_SS: param = CLICK_SS; break;
    case CLICK_L:  param = CLICK_L;  break;
    case CLICK_SL: param = CLICK_SL; break;
    case CLICK_VL: param = CLICK_VL; break;
    default: return;
  }

  // Retrieve CAN base address from DB[].lnk[n]
  uint16_t can_base = 0xFFFF;
  for (uint8_t i = 0; i < DB_count; i++) {
    if (DB[i].LBL == LABEL && n < 16) {
      can_base = DB[i].lnk[n];
      break;
    }
  }

  if (can_base == 0xFFFF) {
    if (IDE) {
      Serial.print(F("Invalid .lnk target for port "));
      Serial.println(n);
    }
    return;
  }

  // Construct the CAN FD message
  CANFDMessage frame;
  frame.id  = can_base;
  frame.ext = false;
  frame.len = 1;
  frame.type = CANFDMessage::CANFD_WITH_BIT_RATE_SWITCH;     // Enable CAN FD with bit rate switch
  frame.idx = 0;                                             // Use default TX FIFO index
  frame.data[0] = param;

  // Send using tryToSendReturnStatusFD pattern
  sendCANFDFrame(frame.data, frame.len, can_base);
}

// ----------------------------------------------------------------------------
// Milliseconds delay
// ----------------------------------------------------------------------------
void DELAY(uint32_t ms)
  {
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = (SystemCoreClock / 1000) * ms;                                        // Number of cycles for ms
    while ((DWT->CYCCNT - start) < cycles) {}                                               // Spin
  }
 
//----------------------------------------------------------------------------------------
/*
Reset: Performs a system reset.
This function disables all interrupts except for non-maskable interrupts (NMI) 
by setting the FAULTMASK register. It then triggers a system reset using 
the NVIC (Nested Vectored Interrupt Controller) system reset function.
Note: This function uses the __set_FAULTMASK intrinsic to disable interrupts
and the NVIC_SystemReset function to perform the reset.
These operations is specific to ARM Cortex-M microcontrollers.
 */
void Reset_board(void)
  {
    __set_FAULTMASK(1);                                                                       // Disable all interrupts except non maskable (NMI)       
    ATOMIC() NVIC_SystemReset();
  }

// ----------------------------------------------------------------------------------
// Send CAN FD control frame: label + 56-bit marker (total 64 bits)
void sendControlMarker(uint64_t marker, uint8_t label)
{
  uint8_t frame[8];
  frame[0] = label;  // First byte is always the label

  // Send least significant 7 bytes of marker (56 bits)
  for (uint8_t i = 0; i < 7; i++)
    frame[i + 1] = (marker >> (8 * i)) & 0xFF;

  sendCANFDFrame(frame, 8, SVR + Update);
  PrintCANFrameHex(frame);
}
// ----------------------------------------------------------------------------------
// Obfuscate control frames before sending
uint64_t obfuscate(uint64_t val) {
  return val ^ MARKER_MASK;
}

//----------------------------------------------------------------------------------------
/**
 * @brief Erases external QSPI flash from 0x00000000 to 0x0078FFFF, skipping protected region.
 *
 * This function:
 *   - Iterates through QSPI sectors (4 KB each)
 *   - Stops completely at address 0x00790000
 *   - Skips erasing any sector in the `.qspi_protected` range (defined below)
 *
 * Typical Use:
 *   - Used to safely erase QSPI while preserving the bootloader (e.g., at 0x00790000).
 *
 * @return true on success, false on any erase failure
 */
//----------------------------------------------------------------------------------------
bool eraseQSPI_Safe()
{
  const uint32_t sector_size      = 4096;                // Sector size = 4 KB
  const uint32_t protect_start    = 0x0079000;            // Start of .qspi_protected region
  const uint32_t protect_end      = protect_start + 0x4000; // End of protected region (inclusive)
  const uint32_t erase_limit      = 0x0079000;            // Stop erasing at this address

  bool success     = true;

  if (IDE)
  {
    Serial.println(F("Erasing QSPI (safe mode) 🔄"));
    Serial.print(F("QSPI total size : "));
    Serial.println(flash.size());
    Serial.print(F("Erase limit     : 0x00000000 to 0x"));
    Serial.println(erase_limit - 1, HEX);
    Serial.print(F("Protect region  : 0x"));
    Serial.print(protect_start, HEX);
    Serial.print(F(" - 0x"));
    Serial.println(protect_end - 1, HEX);
  }

  for (uint32_t addr = 0; addr < erase_limit; addr += sector_size)
  {

    if (!flash.eraseSector(addr))
      {
        if (IDE)
        {
          Serial.print(F("Erase failed at 0x"));
          Serial.println(addr, HEX);
        }
      success = false;
    }

  }

  if (IDE)
  {
    Serial.println();
    if (success)
      Serial.println(F("QSPI erase complete ✅"));
    else
      Serial.println(F("QSPI erase encountered errors ❌"));
  }

  return success;
}

//----------------------------------------------------------------------------------------
/**
 * @brief Mirror entire internal flash (from 0x4000 to 0x80000) into QSPI starting at 0x0000.
 *
 * This function copies all application-relevant flash data (from 0x4000) up to end-of-flash
 * into QSPI flash at offset 0x0000. The QSPI flash must be erased beforehand.
 *
 * Assumptions:
 * - Internal flash starts at 0x00004000
 * - Ends at 0x00080000 (512 KB flash)
 * - QSPI flash is large enough
 * - 256-byte write granularity
 *
 * @return true on success, false on write failure
 */

bool Mirror2QSPI() {
  const uint32_t FLASH_START   = 0x00004000;   // Start of application in internal flash
  const uint32_t FLASH_END     = 0x00080000;   // End of flash region (exclusive)
  const uint32_t SECTOR_SIZE   = 4096;         // Write in 4KB sectors
  const uint32_t QSPI_OFFSET   = QSPI_BASE_ADDR;

  uint32_t total_size = FLASH_END - FLASH_START;
  uint32_t offset = 0;

  if (IDE) {
    Serial.print(F("QSPI Mirror (sector 4KB): 0x"));
    Serial.print(FLASH_START, HEX);
    Serial.print(F(" → 0x"));
    Serial.print(FLASH_END - 1, HEX);
    Serial.println(F(" → QSPI @ 0x000000"));
  }

  while (offset < total_size) {
    uint32_t remaining = total_size - offset;
    uint32_t chunk = (remaining >= SECTOR_SIZE) ? SECTOR_SIZE : remaining;

    const uint8_t* src = (const uint8_t*)(FLASH_START + offset);
    uint32_t dst_addr = QSPI_OFFSET + offset;

    if (!flash.writeBuffer(dst_addr, src, chunk)) {
      if (IDE) {
        Serial.print(F("QSPI write failed at 0x"));
        Serial.println(dst_addr, HEX);
      }
      return false;
    }
    if (IDE) Serial.print(".");
    offset += chunk;
  }
  if (IDE) {
    Serial.println();
    Serial.println(F("QSPI mirror complete ✅"));
  }
  return true;
}

//----------------------------------------------------------------------------------------
// Verifies that QSPI content at 0x00000000 matches internal flash from 0x00004000
// up to 0x00080000 (end of flash). Compares in 256-byte blocks.
bool verifyQSPI() 
  {
    const uint32_t flash_start = FLASH_BASE_ADDR;                                                    // Start of application in internal flash
    const uint32_t qspi_start  = QSPI_BASE_ADDR;                                                     // Start of mirrored QSPI content
    const uint32_t flash_end   = 0x00080000;                                                         // End of flash
    const uint32_t compare_size = flash_end - flash_start;
    const uint32_t block_size   = 256;                                                               // Comparison chunk size
    uint8_t flash_buf[block_size];
    uint8_t qspi_buf[block_size];

    for(uint32_t offset = 0; offset < compare_size; offset += block_size)
      {
        uint32_t len = (compare_size - offset < block_size) ? (compare_size - offset) : block_size;
        const uint8_t* flash_ptr = (const uint8_t*)(flash_start + offset);                           // Read internal flash
        memcpy(flash_buf, flash_ptr, len);
        if(!flash.readBuffer(qspi_start + offset, qspi_buf, len))                                    // Read QSPI flash
          {
            Serial.print(F("QSPI read failed at 0x"));
            Serial.println(qspi_start + offset, HEX);
            return false;
          }
        for(uint32_t i = 0; i < len; i++)                                                            // Compare both buffers
          {
            if(flash_buf[i] != qspi_buf[i])
              {
                Serial.print(F("Mismatch at 0x"));
                Serial.print(offset + i, HEX);
                Serial.print(F(": flash=0x"));
                Serial.print(flash_buf[i], HEX);
                Serial.print(F(", qspi=0x"));
                Serial.println(qspi_buf[i], HEX);
                return false;
              }
          }
      }
    Serial.println(F("QSPI matches internal flash up to end of flash ✅"));
    return true;
  }

//----------------------------------------------------------------------------------------
// TimerHandler — Software PWM generator using TC3 interrupt
// This routine is called at a fixed interval defined by TIMER_INTERVAL_US and generates
// PWM signals in software using digitalWrite().
//
// FUNCTIONALITY:
// - For SWITCH or LPOWER boards (TYPE == SWITCH/LPOWER):
//     • Uses single output pin per channel.
//     • Generates PWM by toggling output HIGH/LOW based on pwmDuty[].
//
// - For MPOWER or HPOWER boards (TYPE == MPOWER/HPOWER):
//     • Uses two output pins per channel (A and B) for H-Bridge control.
//     • If pwmDuty == 0 → both pins set to LOW for braking.
//     • If pwmDuty > 0 → PWM signal applied to A/B with direction logic.
//
// RESOURCES:
// - Uses global:
//     • pwmDuty[]         → current PWM value for each channel (0 to PWM_RESOLUTION)
//     • pwmDir[]          → direction (0 = forward, 1 = reverse)
//     • pwmPins[]         → A/B pin mapping (A = even index, B = odd index)
//     • TYPE              → board type (SWITCH, LPOWER, MPOWER, HPOWER)
//     • PWM_CHANNELS      → total number of logical PWM channels
//     • PWM_RESOLUTION    → full-scale resolution of PWM
//----------------------------------------------------------------------------------------
void qqqqqqqqqqqqqq()
  {
    pwmTick++;
    if(pwmTick >= PWM_RESOLUTION) pwmTick = 0;  
      const bool isHBridge = (TYPE == MPOWER || TYPE == HPOWER);                      // Determine if board uses H-Bridge logic
      for(uint8_t ch = 0; ch < PWM_CHANNELS; ch++)
        {
          const uint8_t duty = pwmDuty[ch];
          if(isHBridge)
            {
              const uint8_t pinA = pwmPins[ch * 2];                                   // Even index = A
              const uint8_t pinB = pwmPins[ch * 2 + 1];                               // Odd index = B
              const bool dir     = pwmDir[ch];                                        // Direction: 0 = forward, 1 = reverse
              if(duty == 0)
                {
                  digitalWrite(pinA, LOW);                                            // Braking mode: set both A & B LOW to short motor terminals
                  digitalWrite(pinB, LOW);
                }
              else
                {
                  const bool pwmState = (pwmTick < duty);
                  if(!dir)
                    {         
                      digitalWrite(pinA,  pwmState);                                  // Forward: A = PWM, B = inverted
                      digitalWrite(pinB, !pwmState);
                    }
                  else
                    {
                      digitalWrite(pinA, !pwmState);                                  // Reverse: A = inverted, B = PWM
                      digitalWrite(pinB,  pwmState);
                    }
                }
            }
        else
          {
            const bool pwmState = (pwmTick < duty);                                   // Single output mode: PWM directly on pin
            digitalWrite(pwmPins[ch], pwmState ? HIGH : LOW);
          }
      }
  }




//----------------------------------------------------------------------------------------
// TimerHandler — Software PWM generator using TC3 interrupt
//
// This routine is called at a fixed interval defined by TIMER_INTERVAL_US and generates
// PWM signals in software using digitalWrite().
//
// FUNCTIONALITY:
// - For SWITCH or LPOWER boards (TYPE == SWITCH/LPOWER):
//     • Uses single output pin per channel.
//     • Generates PWM by toggling output HIGH/LOW based on pwmDuty[].
//
// - For MPOWER or HPOWER boards (TYPE == MPOWER/HPOWER):
//     • Uses two output pins per channel (A and B) for H-Bridge control.
//     • If pwmDuty == 0 → both pins set to LOW for braking.
//     • If pwmDuty > 0 → PWM signal applied to A/B with direction logic.
//
// - Additionally, if TYPE != SWITCH:
//     • Checks all pwmDuty[] entries.
//     • If at least one is active, sets PWCTRL to ON.
//     • If all are zero, sets PWCTRL to OFF.
//
// RESOURCES:
// - pwmDuty[], pwmDir[], pwmPins[], TYPE, PWM_CHANNELS, PWM_RESOLUTION
// - PWCTRL, ON, OFF must be defined elsewhere in your project
//----------------------------------------------------------------------------------------
void TimerHandler()
{
  pwmTick++;
  if (pwmTick >= PWM_RESOLUTION) pwmTick = 0;

  const bool isHBridge = (TYPE == MPOWER || TYPE == HPOWER);
  bool anyActive = false;  // Track if any channel is running

  for (uint8_t ch = 0; ch < PWM_CHANNELS; ch++)
  {
    const uint8_t duty = pwmDuty[ch];

    if (duty > 0)
      anyActive = true;  // At least one channel active

    if (isHBridge)
    {
      const uint8_t pinA = pwmPins[ch * 2];       // Even index = A
      const uint8_t pinB = pwmPins[ch * 2 + 1];   // Odd index = B
      const bool dir     = pwmDir[ch];            // Direction: 0 = forward, 1 = reverse

      if (duty == 0)
      {
        // Braking mode: set both A & B LOW to short motor terminals
        digitalWrite(pinA, LOW);
        digitalWrite(pinB, LOW);
      }
      else
      {
        const bool pwmState = (pwmTick < duty);

        if (!dir)
        {
          digitalWrite(pinA,  pwmState);  // Forward: A = PWM
          digitalWrite(pinB, !pwmState);  //           B = inverted
        }
        else
        {
          digitalWrite(pinA, !pwmState);  // Reverse: A = inverted
          digitalWrite(pinB,  pwmState);  //           B = PWM
        }
      }
    }
    else
    {
      // Single-pin PWM mode (SWITCH or LPOWER)
      const bool pwmState = (pwmTick < duty);
      digitalWrite(pwmPins[ch], pwmState ? HIGH : LOW);
    }
  }

  // Control PWCTRL if TYPE != SWITCH
  if (TYPE != SWITCH)
  {
    digitalWrite(PWCTRL, anyActive ? ON : OFF);
  }
}



//----------------------------------------------------------------------------------------
// Set_PWM(channel, percent, direction)
// 
// Purpose:
//   Safely updates the PWM output on a given channel, with automatic handling of 
//   direction changes. If a direction change is detected while the motor is running 
//   (i.e., non-zero PWM), the motor is first stopped for 2 seconds before resuming
//   with the new direction and duty cycle.
//
// Parameters:
//   - channel   : PWM channel index (0 to PWM_CHANNELS - 1)
//   - percent   : Desired duty cycle (0 to 100%)
//   - direction : Direction flag
//                 false → forward  (A = PWM, B = inverted)
//                 true  → reverse  (A = inverted, B = PWM)
//
// Behavior:
//   - If direction changes and motor is running, PWM is set to 0 immediately,
//     and the new parameters are scheduled to resume after 2 seconds.
//   - If no direction change or motor is already stopped, the new PWM and direction
//     are applied immediately.
//
// Notes:
//   - Active-low logic is applied automatically if TYPE == SWITCH.
//   - This function should be used in coordination with UpdatePWMResume().
//

void qqqqqqqqqqqqqqqqq(uint8_t channel, uint8_t percent, uint8_t direction)
  {

    if(channel >= PWM_CHANNELS) return;                                                        // Validate input
    if(percent > 100) percent = 100; 
    if(pwmPendingResume[channel])                                                              // Prevent override if a direction change is already pending ---
    return;

    if((pwmDir[channel] != direction) && (saved_PWM[channel] > 0))                             // Handle direction change while motor is active ---
      {
        pwmDuty[channel] = 0;                                                                  // Immediately stop motor
        saved_PWM[channel] = 0;
// 2. Schedule resume after delay (handled in UpdatePWMResume)
        pwmNextDuty[channel] = percent;                                                        // Save target PWM value
        pwmNextDir[channel]  = direction;                                                      // Save target direction
        pwmStopTime[channel] = millis();                                                       // Save stop time
        pwmPendingResume[channel] = true;                                                      // Mark resume pending
        return;                                                                                // Exit — update will happen later
      }
// --- Apply PWM immediately if no direction change or already stopped ---
    saved_PWM[channel] = percent;
    pwmDir[channel] = direction;
    if(TYPE == SWITCH) percent = 100 - percent;                                                // Apply logic inversion to control leds of SWITCH boards
    pwmDuty[channel] = map(percent, 0, 100, 0, PWM_RESOLUTION);                                // Scale 0–100% to 0–PWM_RESOLUTION
  }


//----------------------------------------------------------------------------------------
// Set_PWM(channel, percent, direction)
//
// Purpose:
//   Safely updates the PWM output on a given channel with direction control and braking.
//   If direction changes while the motor is active, a braking delay is applied before
//   resuming with the new direction and PWM value.
//
// Behavior:
//   - If direction changes at non-zero PWM:
//       → Motor is stopped (PWM = 0).
//       → New settings are scheduled and applied after a 1-second delay.
//   - If no direction change or already stopped:
//       → PWM and direction are applied immediately.
//
// Additional:
//   - TYPE == SWITCH → active-low logic (inverted PWM).
//   - If TYPE != SWITCH, checks all pwmDuty[] values and sets PWCTRL:
//       → ON if any active PWM.
//       → OFF if all are 0.
//
// Parameters:
//   - channel   : PWM channel index (0 to PWM_CHANNELS - 1)
//   - percent   : Duty cycle (0–100%)
//   - direction : 0 = forward, 1 = reverse
//----------------------------------------------------------------------------------------
void Set_PWM(uint8_t channel, uint8_t percent, uint8_t direction)
  {
    if(channel >= PWM_CHANNELS) return;
    if(percent > 100) percent = 100;
    if(pwmPendingResume[channel]) return;                                                 // Avoid interrupting a scheduled direction change 
    if((pwmDir[channel] != direction) && (saved_PWM[channel] > 0))                        // Handle direction change while motor is active
      {
        pwmDuty[channel] = 0;                                                             // Stop motor immediately
        saved_PWM[channel] = 0;
        pwmNextDuty[channel] = percent;                                                   // Schedule resume
        pwmNextDir[channel]  = direction;
        pwmStopTime[channel] = millis();
        pwmPendingResume[channel] = true;
        if(TYPE != SWITCH)                                                                // Update PWCTRL if needed
          {
            bool anyActive = false;
            for(uint8_t i = 0; i < PWM_CHANNELS; i++)
            if(pwmDuty[i] > 0) { anyActive = true; break; }
            digitalWrite(PWCTRL, anyActive ? ON : OFF);
          }
        return;
      }
    saved_PWM[channel] = percent;                                                         // Apply PWM immediately (no direction change)
    pwmDir[channel]    = direction;
    if(TYPE == SWITCH)percent = 100 - percent;                                            // Apply active-low logic   
    pwmDuty[channel] = map(percent, 0, 100, 0, PWM_RESOLUTION);
    if(TYPE != SWITCH)                                                                    // Update PWCTRL if needed
      {
        bool anyActive = false;
        for(uint8_t i = 0; i < PWM_CHANNELS; i++)
        if(pwmDuty[i] > 0) { anyActive = true; break; }
        digitalWrite(PWCTRL, anyActive ? ON : OFF);
      }
  }

//----------------------------------------------------------------------------------------
// UpdatePWMResume() — Resume motor PWM after a scheduled direction change delay
//
// This function checks whether the delay period following a direction change has expired.
// If so, it applies the new PWM duty cycle and direction that were saved earlier.
// 
// This mechanism ensures that motors resume with the correct parameters after
// a safe delay, such as after a forced stop or direction update.
//
// Variables used:
//   - pwmPendingResume[ch] → true if resume is pending for this channel
//   - pwmStopTime[ch]      → millis() timestamp when the channel was stopped
//   - pwmNextDuty[ch]      → PWM duty (0–100%) to apply after delay
//   - pwmNextDir[ch]       → Direction to apply after delay
//   - pwmDir[ch]           → Current direction
//   - saved_PWM[ch]        → Currently applied duty cycle
//   - pwmDuty[ch]          → Scaled value for PWM output (0–PWM_RESOLUTION)
//   - TYPE                 → Used to determine if PWM needs inversion (SWITCH boards)
//   - IDE                  → Enables debug logging
//
// Called every 10ms in TCC2 handler to apply pending updates.
//----------------------------------------------------------------------------------------
void UpdatePWMResume()
{
  bool anyActive = false;  // Track if any PWM output becomes active

  for (uint8_t ch = 0; ch < PWM_CHANNELS; ch++)
  {
    if (pwmPendingResume[ch] && (millis() - pwmStopTime[ch] >= 1000))
    {
      // Resume direction and duty
      pwmDir[ch] = pwmNextDir[ch];
      saved_PWM[ch] = pwmNextDuty[ch];

      uint8_t percent = pwmNextDuty[ch];
      if (TYPE == SWITCH)
        percent = 100 - percent;  // Invert logic if needed

      pwmDuty[ch] = map(percent, 0, 100, 0, PWM_RESOLUTION);
      pwmPendingResume[ch] = false;
    }

    // Check if current channel has active PWM
    if (pwmDuty[ch] > 0)
      anyActive = true;
  }

  // Apply PWCTRL logic if not a SWITCH board
  if (TYPE != SWITCH)
    digitalWrite(PWCTRL, anyActive ? ON : OFF);
}

