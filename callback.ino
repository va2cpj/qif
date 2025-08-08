
// ~/Arduino/QIF/switch/callback.ino

#include "qif.h"


// Callback are invoked when a frame matches the filter

//----------------------------------------------------------------------------------------
// Process_Time: Handles and decodes an incoming CAN FD frame containing time data.
//
// Expected frame layout (8 bytes total):
//   Byte 0 : Device label (uint8_t)
//   Byte 1 : Year high byte (MSB)
//   Byte 2 : Year low byte  (LSB)
//   Byte 3 : Month (1–12)
//   Byte 4 : Day   (1–31)
//   Byte 5 : Hour  (0–23)
//   Byte 6 : Minute (0–59)
//   Byte 7 : Second (0–59)
//
// This function compares the received time with the local RTC time. If the difference
// is greater than 1 hour (3600 seconds), the local RTC is adjusted to the received value.
void Process_Time(const CANFDMessage & message)
  {
// Visual indication of CAN activity using onboard LED strip
    BLINK(BLUE);

// Ensure the incoming CAN frame has exactly 8 bytes as required by the time format
    if(message.len != 8)
      {
        if(IDE) Serial.println(F("Invalid TIME frame length."));
        return;
      }

// Byte 0 is the label of the sending device
    uint8_t label = message.data[0];

// Bytes 1–7 represent the timestamp
    uint16_t year   = ((uint16_t)message.data[1] << 8) | message.data[2];                         // Combine two bytes for year
    uint8_t  month  = message.data[3];
    uint8_t  day    = message.data[4];
    uint8_t  hour   = message.data[5];
    uint8_t  minute = message.data[6];
    uint8_t  second = message.data[7];

// Construct DateTime object from received components
    DateTime received(year, month, day, hour, minute, second);

// Retrieve current RTC time
    DateTime local = rtc.now();

// If debugging and enabled over serial
    if(IDE)
      {
        Serial.print(F("RX  TIME FROM: "));
        Serial.print(label); Serial.print(F(" "));
        Serial.print(year); Serial.print("-");
        Serial.print(month < 10 ? "0" : ""); Serial.print(month); Serial.print("-");
        Serial.print(day < 10 ? "0" : ""); Serial.print(day); Serial.print(" ");
        Serial.print(hour < 10 ? "0" : ""); Serial.print(hour); Serial.print(":");
        Serial.print(minute < 10 ? "0" : ""); Serial.print(minute); Serial.print(":");
        Serial.print(second < 10 ? "0" : ""); Serial.println(second);
        Serial.print(F("LOCAL TIME:       "));
        Serial.println(local.timestamp(DateTime::TIMESTAMP_FULL));
      }

// Compare UNIX timestamps (seconds since 1/1/1970)
    uint32_t delta = abs(received.unixtime() - local.unixtime());

// Update local RTC only if difference exceeds 3600 seconds (1 hour)
    if(delta > 3600)
      {
        rtc.adjust(received);                                                                     // Set RTC to received time
        if(IDE)
          {
            Serial.print(F("RTC TIME UPDATED  : "));
            Serial.println(received.timestamp(DateTime::TIMESTAMP_FULL));
          }
      }
    else
      {
      if(IDE) Serial.println(F("No RTC adjustment needed (within 1 hour)."));
      }
  }

//----------------------------------------------------------------------------------------
void Process_Reboot(const CANFDMessage & message)
  {
    BLINK(ORANGE);
    if(IDE)
      {
        Serial.print(F("REBOOT PROCESSING 🔂"));
        Serial.print(F("CAN ID: "));
        PrintHex16(message.id);
        Serial.print(F(" "));
        Serial.print(F("Length: "));
        PrintHex8(message.len);
        Serial.print(F(" "));
        Serial.print(F("Data: "));
      }
    for(uint8_t i = 0; i < message.len; i++)
      {
        if(IDE)
          {
            PrintHex8(message.data[i]);
            Serial.print(" ");
          }
      }
    if(IDE) Serial.println();
  }


//----------------------------------------------------------------------------------------
// Process_Update: Receive firmware stream over CAN FD and store into QSPI
//
// This function:
//   - Detects STX and ETX control markers
//   - Writes 8-byte data frames into QSPI flash page-by-page
//   - Verifies CRC64 at the end against the received 8-byte CRC frame
//
// Assumptions:
//   - QSPI was erased beforehand up to bootloader region
//   - Each CAN FD frame contains exactly 8 bytes
//   - The firmware ends with a CRC64 checksum
//----------------------------------------------------------------------------------------

void Process_Update(const CANFDMessage &message)
{
  static uint32_t qspiOffset = 0;
  static uint8_t pageBuffer[QSPI_PAGE_SIZE];
  static uint16_t pageIndex = 0;
  static crc64_stream crc;
  static uint8_t crc_candidate[8];
  static uint8_t frame_buffer[8];
  static bool has_prev_frame = false;
  static uint32_t byteCount = 0;
  static uint64_t lastCRCValue = 0;
  static uint32_t frameCount = 0;

  // Only process valid 8-byte CAN FD frames
  if (message.len != 8) return;

  // Control marker check (first byte must match local label)
  bool isControlFrame = (message.data[0] == LABEL);

  // --- STX received: Initialize ---
  if (isControlFrame && isControlMarkerMatch(stx, message))
  {
    BLINK(LILAC);
    STX_FLAG = true;
    ETX_FLAG = false;
    qspiOffset = 0;
    pageIndex = 0;
    byteCount = 0;                       // Reset byte counter for update
    has_prev_frame = false;
    crc64_stream_init(&crc, 0);         // Restart CRC
    eraseQSPI_Safe();                   // Erase QSPI up to Boot2 area

    if (IDE) Serial.println(F("STX received ✅"));
    return;
  }

  // --- ETX received: Finalize update and verify CRC ---
  if (isControlFrame && isControlMarkerMatch(etx, message))
  {
    STX_FLAG = false;
    ETX_FLAG = true;

    if (IDE)
    {
      Serial.println(F("ETX received ✅"));
      Serial.print(F("Total bytes received: "));
      Serial.println(byteCount);
    }

    // Flush any remaining data in the current page buffer
    if (pageIndex > 0)
    {
      memset(&pageBuffer[pageIndex], 0xFF, QSPI_PAGE_SIZE - pageIndex);
      flash.writeBuffer(qspiOffset, pageBuffer, QSPI_PAGE_SIZE);
      qspiOffset += QSPI_PAGE_SIZE;
    }

    // Convert saved CRC candidate into uint64_t
    lastCRCValue = 0;
    for (uint8_t i = 0; i < 8; i++)
      lastCRCValue |= ((uint64_t)crc_candidate[i]) << (8 * i);

    uint64_t computed_crc = crc64_stream_finalize(&crc);
    bool match = (computed_crc == lastCRCValue);

    if (IDE)
    {
      Serial.print(F("Computed CRC64: ")); PrintHex64(computed_crc);
      Serial.print(F(" | Received CRC64: ")); PrintHex64(lastCRCValue);
    }

    if (!match)
    {
      if (IDE) Serial.println(F("\nCRC MISMATCH ❌"));
      Send_Nack();
      return;
    }
    else
    {
      if (IDE)
      {
        Serial.println(F("\nCRC MATCH ✅"));
        Serial.println(F("SYSTEM WILL REBOOT NOW"));
      }
      Send_Ack();                                                                     
 
      void (*boot2_ptr)(void) = (void (*)(void))(MQSPI_BASE_ADDR + BOOT2_START_ADDR);
      boot2_ptr();                                                                                            // Jump to QSPI Boot2 mapped to 0x04000000 + 0x79000
      return;
    }
  }

  // Skip if update not yet started
  if (!STX_FLAG) return;

  // --- Normal data frame processing ---
  byteCount += 8;

  // CRC update with *previous* frame
  if (has_prev_frame)
  {
    crc64_stream_update(&crc, frame_buffer, 8);

    for (uint8_t i = 0; i < 8; i++)
    {
      pageBuffer[pageIndex++] = frame_buffer[i];
      if (pageIndex >= QSPI_PAGE_SIZE)
      {
        if (!flash.writeBuffer(qspiOffset, pageBuffer, QSPI_PAGE_SIZE))
        {
          if (IDE)
          {
            Serial.print(F("QSPI write failed at offset 0x"));
            Serial.println(qspiOffset, HEX);
          }
          STX_FLAG = false;
          Send_Nack();
          return;
        }
        qspiOffset += QSPI_PAGE_SIZE;
        pageIndex = 0;
      }
    }
  }

  // Save current frame for CRC64 and final CRC frame match
  memcpy(frame_buffer, message.data, 8);
  memcpy(crc_candidate, message.data, 8);
  has_prev_frame = true;

  // Visual blinking
  frameCount++;
  if ((frameCount % 126) == 0) BLINK(BLUE);
  if ((frameCount % 256) == 0) BLINK(YELLOW);
}


//----------------------------------------------------------------------------------------
// Process_Receive_BME: Handles and decodes an incoming CAN FD frame containing BME688 sensor data.
// Each frame is 6 bytes long and contains:
//   Byte   0:    Type of measure (Temperature, Pressure, Humidity..........)
//   Bytes  1-4:  A 4-byte float value (sensor reading)
//   Byte   5:    Label of sender

void Process_BME(const CANFDMessage & message)
  {
    BLINK(BLUE);                                                                                  // Turn on BLUE LED to indicate processing activity

    if(message.len < 3)                                                                           // If len < 3 it's a request else a it's a response
      {                                                                           
        sendBME(message.data[0],message.data[1]);                                                 // data[0] type, data[1] target
        return;
      }

    uint8_t label = message.data[5];                                                              // Extract label last byte (device identifier)         
    char    type   = message.data[0];                                                             // First byte: type of measure enum info  BMETEMP,BMEPRESS,BMEHUMID......
    float   value;                                                                                // Value 4-byte IEEE float value
  
    union {                                                                                       // Convert bytes 1-4 into a float using a union
      uint8_t b[4];
      float f;
    } u;

    u.b[0] = message.data[1];
    u.b[1] = message.data[2];
    u.b[2] = message.data[3];
    u.b[3] = message.data[4];
    value = u.f;

    if(IDE)                                                                                       // Print the received value if connected to IDE (serial debug)
      {
        Serial.print(F("RX BME   FROM: "));
        Serial.print(label, DEC);                                                                 // Print sender label
        Serial.print(F("  "));

// Interpret and print the value based on the type
      switch (type)
        {
          case BMETEMP: { Serial.print(BME1); Serial.println(value); break; }                     // Temperature
          case BMEPRESS: { Serial.print(BME2); Serial.println(value); break; }                    // Pressure
          case BMEHUMID: { Serial.print(BME3); Serial.println(value); break; }                    // Humidity
          case BMEGAZ: { Serial.print(BME4); Serial.println(value); break; }                      // Gas
          case BMEIAQ: {                                                                          // IAQ (Indoor Air Quality)
                      Serial.print(BME5); Serial.print(value); Serial.print("  ");
                      if((int)value <= 50)         Serial.println(F("Excellent"));
                      else if ((int)value <= 100)  Serial.println(F("Good"));
                      else if ((int)value <= 150)  Serial.println(F("Lightly polluted"));
                      else if ((int)value <= 200)  Serial.println(F("Moderately polluted"));
                      else if ((int)value <= 250)  Serial.println(F("Heavily polluted"));
                      else if ((int)value <= 300)  Serial.println(F("Severely polluted"));
                      else                         Serial.println(F("Extremely polluted"));
                      break;
                    }
          case BMEVOC: { Serial.print(BME6); Serial.println(value); break; }                      // VOC
          case BMECO2: { Serial.print(BME7); Serial.println(value); break; }                      // CO2
          default: {
            Serial.println(F("Unknown BME info "));                                               // Unexpected type
          break;
                    }
        }
      }
  }

void Process_Heart_Beat(const CANFDMessage &message)
  {
    BLINK(ORANGE);

    if(IDE)
      {
        Serial.print(F("RX BEAT  FROM: "));
        Serial.println(message.data[0], DEC);                                                      // Contains the board label
      }
  }

//----------------------------------------------------------------------------------------
/**
 * @brief Processes an incoming CAN FD ACK (acknowledgement) frame.
 *
 * This function provides visual and serial debug feedback when an ACK frame is received.
 * The expected frame format is 8 bytes:
 *   - Byte 0: Sender's device label
 *   - Bytes 1–7: User-defined or marker-specific data
 *
 * Behavior:
 *   - Turns on a blue LED as a visual indicator of ACK reception.
 *   - If IDE mode is active, prints label a detailed breakdown of the message:
 
 * Dependencies:
 *   - `IDE` (global flag for serial debug output)
 *   - `strip` (NeoPixel LED strip)
 *   - `can.dispatchReceivedMessage()` (to handle any pending CAN frames)
 *
 * @param message The received CAN FD message containing the ACK frame.
 */
void Process_ACK(const CANFDMessage & message)
{
// --- Visual feedback using onboard LED strip ---
  BLINK(BLUE);       // Set LED to blue to indicate ACK reception

// --- If serial debug (IDE) mode is enabled, print full details ---
  if (IDE)
  {
// Print the label (sender's unique identifier)
    Serial.print(F("RX ACK   FROM: "));
    Serial.println(message.data[0], DEC);          // Byte 0: device label in decimal format
  }
// --- Allow other queued CAN messages to be handled ---
}

//----------------------------------------------------------------------------------------
void Process_NACK(const CANFDMessage & message) {
  BLINK(BLUE);
  if(IDE)
    {
      Serial.print(F("NACK PROCESSING->"));
      Serial.print(F("CAN ID: "));
      PrintHex16(message.id);
      Serial.print(F(" "));
      Serial.print(F("LENGTH: "));
      PrintHex8(message.len);
      Serial.print(F(" "));
      Serial.print(F("DATA: "));
    }
  for (uint8_t i = 0; i < message.len; i++) {
    if(IDE)
      {
        PrintHex8(message.data[i]);
        Serial.print(" ");
      }
  }
  if(IDE) Serial.println();
}

//----------------------------------------------------------------------------------------
// Process_Led: Handles LED control based on incoming CAN FD message.
//
// - case 1: Increases brightness by 20% each time, up to 100%, wraps to 20%
// - case 2: Decreases brightness by 20% each time, down to 0%, wraps to 100%
// - case 3: Enables blinking mode
// - case 4: Disables blinking without changing brightness
// - case 5: Turns off LED and disables blinking
//
// Each LED keeps its own brightness state (0–5 for case 1, 0–5 for case 2).
//----------------------------------------------------------------------------------------

void Process_Led(const CANFDMessage &message)
{
  BLINK(BLUE);  // Visual feedback for CAN activity

  uint8_t led   = message.id & 0x0F;                                               // Extract LED channel (0–7)
  uint8_t value = message.data[0];                                                 // LED command

// Static state to track brightness steps for each LED (0 to 5 = 0–100% / 20% steps)
  static uint8_t level_up[8] = {0};                                                // case 1 up counter (1–5) brigthness up
  static uint8_t level_down[8] = {5};                                              // case 2 down counter (5–0) brigthness down

  if (IDE)
  {
    Serial.print(F("LED FROM:     0x"));
    Serial.print(message.id, HEX);
    Serial.print(F(" -> LED "));
    Serial.print(led);
    Serial.print(F(" VALUE "));
    Serial.println(value);
  }

  switch (value)
  {
    case 0:                                                                         // No action
      break;

    case 1:                                                                         // Increment brightness by 20% each time (lock at 100%)
      level_up[led]++;
      Set_PWM(led, level_up[led] * 20, 0);                                          // Apply 20–100%
      Blink_Mode[led] = false;
      break;

    case 2:                                                                         // Decrement brightness by 20% each time (lock at 0%)
      if(level_down[led] > 0) level_down[led]--;                                    // Min level is 0 (0%)
      Set_PWM(led, level_down[led] * 20, 0);                                        // Apply 0–100%
      Blink_Mode[led] = false;
      break;

    case 3:
      Blink_Mode[led] = true;                                                       // Enable blinking
      break;

    case 4:
      Blink_Mode[led] = false;                                                      // Disable blinking (keep brightness)
      break;

    case 5:
      Set_PWM(led, 0, 0);                                                           // Turn off
      Blink_Mode[led] = false;
      level_up[led] = 0;                                                            // Reset brightness state
      level_down[led] = 5;
      break;

    default:
      Blink_Mode[led] = false;
      break;
  }
}

void Process_PwrCtrl(const CANFDMessage & message)
  {
    BLINK(BLUE);                                                                    // Visual feedback for activity
    
    if(message.data[0] == 0)
      {
        digitalWrite(PWCTRL, OFF);                                                  // Turn off power control
        for(uint8_t i = 0; i < PWM_CHANNELS;  i++) Set_PWM(i, 0, OFF);
      }                            
    else digitalWrite(PWCTRL,1);                                                    // Turn on                                                                        
  }

void Process_Lpwm(const CANFDMessage & message)
  {
    BLINK(BLUE);                                                                    // Visual feedback for activity

    uint8_t channel =  message.id & 0x0F;                                           // Extract channel (0–7)
    uint8_t value   = message.data[0];                                              // PWM command duty cycle 0-100%

    if (IDE)
      {
        Serial.print(F("PWM -> CHANNEL "));
        Serial.print(channel);
        Serial.print(F(" VALUE "));
        Serial.println(value);
      }

    switch (channel)
      {
        case 0:                                                                     // Channel 0  
          Set_PWM(pwm0, value, 0);                         
        break;

        case 1:                                                                                                                                                        
          Set_PWM(pwm1, value, 0);
        break;

        case 2:                
          Set_PWM(pwm2, value, 0);                                                         
        break;

        case 3:
          Set_PWM(pwm3, value, 0); 
        break;

        case 4:
          Set_PWM(pwm4, value, 0); 
        break;

        case 5:
          Set_PWM(pwm5, value, 0); 
        break;

        case 6:
          Set_PWM(pwm6, value, 0); 
        break; 
    
        case 7:
          Set_PWM(pwm7, value, 0); 
        break;

      default:
        break;
    }
  }

void Process_Hpwm(const CANFDMessage & message)
  {
    BLINK(BLUE);                                                                              // Visual feedback for activity

    uint8_t channel   =  message.id & 0x0F;                                                   // Extract channel (0–6)
    uint8_t value     = message.data[0];                                                      // PWM command duty cycle 0-100%
    uint8_t direction = message.data[1];

        switch (channel)
      {
        case 0:                                                                               // Channel 0  
          Set_PWM(pwm0, value, direction);                         
        break;

        case 1:                                                                                                                                                        
          Set_PWM(pwm1, value, direction);
        break;

        case 2:                
          Set_PWM(pwm2, value, direction);                                                         
        break;

        case 3:
          Set_PWM(pwm3, value, direction); 
        break;

        case 4:
          Set_PWM(pwm4, value, direction); 
        break;

        case 5:
          Set_PWM(pwm5, value, direction); 
        break;

      default:
        break;
    }  
  }

void Process_Analog(const CANFDMessage & message)
  {
    BLINK(BLUE);                                                                              // Visual feedback for activity
    
    CANFDMessage msg;
    uint8_t   channel   = (message.id & 0x0F) - Anl;                                          // Extract analog channel (0–3) 
    uint16_t  target    = SVR + Anl;  
    uint16_t  analogValue; 

    if(IDE)
      {
        Serial.print(F("ANALOG FROM:     0x"));
        Serial.print(message.id, HEX);
        Serial.print(F(" -> CHANNEL "));
        Serial.print(channel);
        Serial.print(F(" TARGET "));
        Serial.println(target);
      }

    switch(channel)
      {
        case 0:                                                                           // Channel 0    
          analogValue = analogRead(analogPins[0]);          
        break;

        case 1:                                                                                                                                                        
          analogValue = analogRead(analogPins[1]);
        break;

        case 2:                
          analogValue = analogRead(analogPins[2]);                                  
        break;

        case 3:
          analogValue = analogRead(analogPins[3]);
        break;

        default:
        break;
      }

    msg.id = target;                                                                          // Return value on service channel with sender in first byte
    msg.len = 4;
    msg.data[0] = LABEL;                                                                      // Sender
    msg.data[1] = channel;                                                                    // Analog 0-3
    msg.data[2] = analogValue >> 8;                                                           // MSB
    msg.data[3] = analogValue & 0xFF;                                                         // LSB
    sendCANFDFrame(msg.data, msg.len, msg.id);
  }

void Process_Analog_RX(const CANFDMessage & message)                                          // Receiving analog channel value
  {
    BLINK(BLUE);                                                                              // Visual feedback for activity

    uint8_t sender  = message.data[0];
    uint8_t channel = message.data[1];
// Reconstruct 16-bit analog value from two bytes (little-endian: LSB first)
    uint16_t analogValue = (uint16_t)message.data[2] | ((uint16_t)message.data[3] << 8);
// Convert to voltage (assuming 3.3V reference and 16-bit resolution)
    float voltage = (analogValue / 65535.0f) * 3.3f;
    if(IDE)                                                                                   // Print the values
      {
        Serial.print(F("ANALOG READ FROM: "));
        Serial.print(sender);
        Serial.print(F("  CHANNEL: "));
        Serial.print(channel);
        Serial.print(F("  VOLTAGE: "));
        Serial.print(voltage, 3);                                                             // Print voltage with 3 decimal places
        Serial.println(F(" V"));  
      }                                               
  }
void Process_Isense(const CANFDMessage & message)                                             // Current sense
  {
    BLINK(BLUE);                                                                              // Visual feedback for activity
  }

void Process_Alarm_BME(const CANFDMessage & message)                                          // Alarm detection of CO,CO2 & temp
  {
    BLINK(BLUE);                                                                              // Visual feedback for activity
  }

void Process_GPS(const CANFDMessage & message)
  {
    BLINK(BLUE);                                                                              // Visual feedback for activity
  }

void Process_Gyro(const CANFDMessage & message)
  {
    BLINK(BLUE);                                                                              // Visual feedback for activity
  }

void Process_Level(const CANFDMessage & message)
  {
    BLINK(BLUE);                                                                              // Visual feedback for activity
  }

void Process_Pir(const CANFDMessage & message)
  {
    BLINK(BLUE);                                                                              // Visual feedback for activity
  }

// ---------------------------------------------------------------------
// Compares a control marker (STX, ETX, etc.) against received CAN frame
// The marker is 56 bits obsfucated with MARKER_MASK stored in message.data[1..7], label is data[0]
bool isControlMarkerMatch(uint64_t marker, const CANFDMessage &msg)
{
  if (msg.len != 8) return false;

  uint64_t candidate = 0;
  for (uint8_t i = 1; i < 8; i++) {
    candidate |= ((uint64_t)msg.data[i]) << (8 * (i - 1));
  }

  uint64_t decoded = candidate ^ MARKER_MASK;                                                 // deobfuscate

  return decoded == marker;
}
