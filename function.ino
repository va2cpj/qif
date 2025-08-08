
// ~/Arduino/QIF/switch/function.ino

/*
┌───────────────────────────────────────────────────────────────┐
│                 Functions sequence (A.I. generated)           │
└───────────────────────────────────────────────────────────────┘

1. ───── Function pointer dispatcher ───────────────────────────
    - `CallFunction(can, idx, param)` is the main entry point.
    - Looks up the `DB[]` array for a matching `CAN` base address.
    - Finds the label index in the DB to identify the board.

2. ───── Validation ────────────────────────────────────────────
    - If CAN address not found:
      * Prints "CAN address not found" if `IDE` debug mode is active.
      * Returns immediately (no function call).
    - If `idx` >= 16:
      * Prints "Invalid function index" if `IDE` is active.
      * Returns immediately.

3. ───── Function pointer resolution ───────────────────────────
    - Retrieves function pointer from `DB[label].fct[idx]`.
    - If function pointer is `nullptr`:
      * Prints "Null function pointer" if `IDE` is active.
      * Returns immediately.

4. ───── Function invocation ───────────────────────────────────
    - Calls `fptr(param)` directly, passing the `uint8_t` parameter.

5. ───── FCT00 to FCT07 implementations ────────────────────────
    - Each `FCTxx(uint8_t value)` is a specific CAN-based function.
    - Calculates a unique CAN channel as:
      * `channel = CAN + offset` (offset from 0x00 to 0x07).
    - Builds an 8-byte CAN FD frame:
      * All bytes zero except:
        `data[7] = value`.

6. ───── Serial debug output ──────────────────────────────────
    - If `IDE` debug mode is active:
      * Prints function name (e.g., "FCT03 → CAN ID: 0x501").
      * Shows parameter value.
      * Prints full CAN frame contents in hex.

7. ───── CAN FD frame transmission ────────────────────────────
    - Calls:
      * `sendCANFDFrame(data, 8, channel);`
    - Sends the 8-byte frame over CAN FD to the calculated ID.

8. ───── FCT08 to FCT15 placeholders ──────────────────────────
    - Defined but empty.
    - Reserved for future expansion of function table.

9. ───── DUMMY handler ────────────────────────────────────────
    - Called if a DB entry points to `DUMMY` instead of a real function.
    - Prints "Calling an undefined function" if `IDE` is active.

*/

#include "qif.h"

// Function call wrapper using CAN address
// Only calls if pointer is valid, prints call info after validation
void CallFunction(uint16_t can, uint8_t idx, uint8_t param) {
  int label = -1;

  for (uint8_t i = 0; i < DB_count; i++) {
    if (DB[i].CAN == can) {
      label = i;
      break;
    }
  }

  if (label == -1) {
    if (IDE) {
      Serial.print(F("CAN address not found: "));
      Serial.println(can, HEX);
    }
    return;
  }

  if (idx >= 16) {
    if (IDE) {
      Serial.print(F("Invalid function index: "));
      Serial.println(idx);
    }
    return;
  }

  FilterCallback fptr = DB[label].fct[idx];                                             // Do not redeclare if already declared

  if (fptr != nullptr) {
    CANFDMessage msg;
    msg.len = 8;
    memset(msg.data, 0, 8);
    msg.data[7] = param;                                                                // Pass param in data[7] like your function expects

    fptr(msg);
  } else {
    if (IDE) {
      Serial.println(F("Null function pointer"));
    }
  }
}

//  Implementations for all function pointers taking uint8_t parameter                                             
void DUMMY(const CANFDMessage &)
  {
    if(IDE) Serial.println("Calling an undefined function");
  }                                              

// Switch function send CAN frame (TX)

void FCT00(const CANFDMessage & msg)
{
  uint16_t channel = CAN_BASE + 0x00;
  uint8_t data[8] = {0}; data[7] = msg.data[7];
  if (IDE) {
    Serial.print("SEND CAN ID: 0x"); Serial.print(channel, HEX);
    Serial.print(" FRAME: ");
    for(uint8_t i = 0; i < 8; i++) {
      if(data[i] < 0x10) Serial.print('0');
      Serial.print(data[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
  }
  sendCANFDFrame(data, 8, channel);                                                       // Transmit 8 bytes to CAN                      
}

void FCT01(const CANFDMessage & msg)
{
  uint16_t channel = CAN_BASE + 0x01;
  uint8_t data[8] = {0}; data[7] = msg.data[7];
  if (IDE) {
    Serial.print("SEND CAN ID:  0x"); Serial.print(channel, HEX);
    Serial.print(" FRAME: ");
    for (uint8_t i = 0; i < 8; i++) {
      if (data[i] < 0x10) Serial.print('0');
      Serial.print(data[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
  }
  sendCANFDFrame(data, 8, channel);                                                         // Transmit 8 bytes to CAN   
}

void FCT02(const CANFDMessage & msg) {
  uint16_t channel = CAN_BASE + 0x02;
  uint8_t data[8] = {0}; data[7] = msg.data[7];

  if (IDE) {
    Serial.print("SEND CAN ID: 0x"); Serial.print(channel, HEX);
    Serial.print(" FRAME: ");
    for (uint8_t i = 0; i < 8; i++) {
      if (data[i] < 0x10) Serial.print('0');
      Serial.print(data[i], HEX); Serial.print(' ');
    }
    Serial.println();
  }

  sendCANFDFrame(data, 8, channel);
}

void FCT03(const CANFDMessage & msg) {
  uint16_t channel = CAN_BASE + 0x03;
  uint8_t data[8] = {0}; data[7] = msg.data[7];

  if (IDE) {
    Serial.print("SEND CAN ID: 0x"); Serial.print(channel, HEX);
    Serial.print(" FRAME: ");
    for (uint8_t i = 0; i < 8; i++) {
      if (data[i] < 0x10) Serial.print('0');
      Serial.print(data[i], HEX); Serial.print(' ');
    }
    Serial.println();
  }

  sendCANFDFrame(data, 8, channel);
}

void FCT04(const CANFDMessage & msg) {
  uint16_t channel = CAN_BASE + 0x04;
  uint8_t data[8] = {0}; data[7] = msg.data[7];

  if (IDE) {
    Serial.print("SEND CAN ID: 0x"); Serial.print(channel, HEX);
    Serial.print(" FRAME: ");
    for (uint8_t i = 0; i < 8; i++) {
      if (data[i] < 0x10) Serial.print('0');
      Serial.print(data[i], HEX); Serial.print(' ');
    }
    Serial.println();
  }

  sendCANFDFrame(data, 8, channel);
}

void FCT05(const CANFDMessage & msg) {
  uint16_t channel = CAN_BASE + 0x05;
  uint8_t data[8] = {0}; data[7] = msg.data[7];

  if (IDE) {
    Serial.print("SEND CAN ID: 0x"); Serial.print(channel, HEX);
    Serial.print(" FRAME: ");
    for (uint8_t i = 0; i < 8; i++) {
      if (data[i] < 0x10) Serial.print('0');
      Serial.print(data[i], HEX); Serial.print(' ');
    }
    Serial.println();
  }

  sendCANFDFrame(data, 8, channel);
}

void FCT06(const CANFDMessage & msg) {
  uint16_t channel = CAN_BASE + 0x06;
  uint8_t data[8] = {0}; data[7] = msg.data[7];

  if (IDE) {
    Serial.print("SEND CAN ID: 0x"); Serial.print(channel, HEX);
    Serial.print(" FRAME: ");
    for (uint8_t i = 0; i < 8; i++) {
      if (data[i] < 0x10) Serial.print('0');
      Serial.print(data[i], HEX); Serial.print(' ');
    }
    Serial.println();
  }

  sendCANFDFrame(data, 8, channel);
}

void FCT07(const CANFDMessage & msg) {
  uint16_t channel = CAN_BASE + 0x07;
  uint8_t data[8] = {0}; data[7] = msg.data[7];

  if (IDE) {
    Serial.print("SEND CAN ID: 0x"); Serial.print(channel, HEX);
    Serial.print(" FRAME: ");
    for (uint8_t i = 0; i < 8; i++) {
      if (data[i] < 0x10) Serial.print('0');
      Serial.print(data[i], HEX); Serial.print(' ');
    }
    Serial.println();
  }

  sendCANFDFrame(data, 8, channel);
}

// Leds function receive CAN frame (RX) called by filter

void FCT08(const CANFDMessage & msg)
{
  uint16_t channel = CAN_BASE + 0x08;
  uint8_t data[8] = {0}; data[7] = msg.data[7];
  if (IDE) {
    Serial.print("RECEIVE CAN ID: 0x"); Serial.print(channel, HEX);
    Serial.print(" FRAME: ");
    for(uint8_t i = 0; i < 8; i++) {
      if(data[i] < 0x10) Serial.print('0');
      Serial.print(data[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
  }            
}

void FCT09(const CANFDMessage & msg)
{
  uint16_t channel = CAN_BASE + 0x09;
  uint8_t data[8] = {0}; data[7] = msg.data[7];
  if (IDE) {
    Serial.print("RECEIVE CAN ID: 0x"); Serial.print(channel, HEX);
    Serial.print(" FRAME: ");
    for(uint8_t i = 0; i < 8; i++) {
      if(data[i] < 0x10) Serial.print('0');
      Serial.print(data[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
  }            
}

void FCT10(const CANFDMessage & msg)
{
  uint16_t channel = CAN_BASE + 0x0a;
  uint8_t data[8] = {0}; data[7] = msg.data[7];
  if (IDE) {
    Serial.print("RECEIVE CAN ID: 0x"); Serial.print(channel, HEX);
    Serial.print(" FRAME: ");
    for(uint8_t i = 0; i < 8; i++) {
      if(data[i] < 0x10) Serial.print('0');
      Serial.print(data[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
  }            
}

void FCT11(const CANFDMessage & msg)
{
  uint16_t channel = CAN_BASE + 0x0b;
  uint8_t data[8] = {0}; data[7] = msg.data[7];
  if (IDE) {
    Serial.print("RECEIVE CAN ID: 0x"); Serial.print(channel, HEX);
    Serial.print(" FRAME: ");
    for(uint8_t i = 0; i < 8; i++) {
      if(data[i] < 0x10) Serial.print('0');
      Serial.print(data[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
  }            
}

void FCT12(const CANFDMessage & msg)
{
  uint16_t channel = CAN_BASE + 0x0c;
  uint8_t data[8] = {0}; data[7] = msg.data[7];
  if (IDE) {
    Serial.print("RECEIVE CAN ID: 0x"); Serial.print(channel, HEX);
    Serial.print(" FRAME: ");
    for(uint8_t i = 0; i < 8; i++) {
      if(data[i] < 0x10) Serial.print('0');
      Serial.print(data[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
  }            
}

void FCT13(const CANFDMessage & msg)
{
  uint16_t channel = CAN_BASE + 0x0d;
  uint8_t data[8] = {0}; data[7] = msg.data[7];
  if (IDE) {
    Serial.print("RECEIVE CAN ID: 0x"); Serial.print(channel, HEX);
    Serial.print(" FRAME: ");
    for(uint8_t i = 0; i < 8; i++) {
      if(data[i] < 0x10) Serial.print('0');
      Serial.print(data[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
  }            
}

void FCT14(const CANFDMessage & msg)
{
  uint16_t channel = CAN_BASE + 0x0e;
  uint8_t data[8] = {0}; data[7] = msg.data[7];
  if (IDE) {
    Serial.print("RECEIVE CAN ID: 0x"); Serial.print(channel, HEX);
    Serial.print(" FRAME: ");
    for(uint8_t i = 0; i < 8; i++) {
      if(data[i] < 0x10) Serial.print('0');
      Serial.print(data[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
  }            
}


void FCT15(const CANFDMessage & msg)
{
  uint16_t channel = CAN_BASE + 0x0f;
  uint8_t data[8] = {0}; data[7] = msg.data[7];
  if (IDE) {
    Serial.print("RECEIVE CAN ID: 0x"); Serial.print(channel, HEX);
    Serial.print(" FRAME: ");
    for(uint8_t i = 0; i < 8; i++) {
      if(data[i] < 0x10) Serial.print('0');
      Serial.print(data[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
  }            
}




// void FCT15(const CANFDMessage & msg) { (void)msg; /* Stub */ }

