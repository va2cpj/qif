
// ~/Arduino/QIF/switch/loader.ino

/*
┌───────────────────────────────────────────────────────────────────┐
│     Explanation of secondary bootloader (A.I. generated)          │
└───────────────────────────────────────────────────────────────────┘

1. ───── Boot2 overview ────────────────────────────────────────────
    - This function (`Boot2()`) is your low-level in-flash bootloader.
    - Its purpose is to copy a firmware image from external QSPI memory
      (usually after a CAN FD firmware update) into the internal flash
      program memory, then jump to the freshly written application.

2. ───── Key assumptions ──────────────────────────────────────────
    - Application code starts at 0x00004000 in internal flash.
    - Boot2 itself starts at 0x00079000 in internal flash.
    - QSPI memory is mapped to 0x04000000 for XIP, raw offset 0x00000000.
    - Internal flash writes in rows (256 bytes) and pages (64 bytes).
    - QSPI has already been populated by a previous process (update).

3. ───── Memory scan in QSPI ──────────────────────────────────────
    - Boot2 starts by scanning QSPI from offset 0.
    - It looks for 4 KB blocks (4096 bytes) that are not all 0xFF.
    - Stops when it finds a block full of 0xFF (meaning "no data").
    - This establishes `program_size` to copy.

4. ───── Critical disables ────────────────────────────────────────
    - Disables:
        * All interrupts via NVIC and FAULTMASK.
        * SysTick timer.
        * Watchdog.
        * Cache controller.
    - Ensures a clean state for flash writes.

5. ───── Copy loop ────────────────────────────────────────────────
    - Copies QSPI → internal flash by rows (256 bytes):
      • For each row:
          1. Erase internal flash row.
          2. Write four 64-byte pages (loaded from QSPI).
          3. Wait for NVM ready.
    - Continues until `program_size` reached (up to Boot2 region).

6. ───── MSP and reset vector validation ──────────────────────────
    - Reads the initial Main Stack Pointer (MSP) from new app.
    - Reads the reset handler address.
    - Checks:
        • MSP is inside SRAM (0x20000000 - 0x2002FFFF).
        • Reset handler is inside [0x00004000, 0x00079000).
    - If invalid, triggers a system reset.

7. ───── Jump to application ──────────────────────────────────────
    - Sets the new MSP (via `__set_MSP`).
    - Sets vector table (SCB->VTOR) to point at 0x00004000.
    - Calls reset handler pointer, transferring execution.
    - Never returns.

8. ───── Extra safety ─────────────────────────────────────────────
    - Multiple `NVIC_SystemReset()` guards ensure a failed update
      or invalid jump never bricks the device — it resets and tries again.

───────────────────────────────────────────────────────────────────
Result:  
    ✓ Safely transfers firmware from QSPI to internal flash
    ✓ Ensures vector tables and stacks are valid
    ✓ Launches new application seamlessly
    ✓ Core of your self-updating system via CAN FD + QSPI
*/
// +----------------------------+
// |       Step 1: Setup       |
// +----------------------------+
// | Erase full QSPI memory    |
// | Copy internal flash       |
// | from 0x4000 to 0x7FFFF    |
// | into QSPI at 0x0000       |
// +-------------+-------------+
//               |
//               v
// +----------------------------+
// |  Step 2: Wait for STX      |
// +----------------------------+
// | Monitor CAN input          |
// | Detect obfuscated STX      |
// +-------------+-------------+
//               |
//               v
// +----------------------------+
// |  On STX detected:          |
// +----------------------------+
// | Erase QSPI up to 0x7F000   |
// | Receive update stream      |
// | Save into QSPI @ 0x0000    |
// +-------------+-------------+
//               |
//               v
// +----------------------------+
// |   On ETX & CRC OK:         |
// +----------------------------+
// | Execute Boot2 in QSPI      |
// | address 0x79000            |
// +-------------+-------------+
//               |
//               v
// +----------------------------+
// |   Boot2 running from XPI   |
// +----------------------------+
// | Copy QSPI[0x0000..]        |
// | to internal flash @0x4000  |
// | Stop on 4KB of 0xFF block  |
// +-------------+-------------+
//               |
//               v
// +----------------------------+
// |           Reboot           |
// +----------------------------+

// typedef void (*boot2_fn_ptr)(void);
// boot2_fn_ptr boot2 = (boot2_fn_ptr)(0x04079000);
// Executes Boot2 in XIP mode from QSPI
// boot2();

//----------------------------------------------------------------------------------------
// Boot2: Secondary Bootloader for SAME51
//
// This function performs the critical job of copying a firmware binary from external QSPI
// memory into the internal flash memory of the SAME51 microcontroller, and then jumping
// to that application.
//
// Key Characteristics:
// - Written to reside in a separate .boot2 section (see linker script)
// - Executes entirely from internal flash
// - Handles QSPI access through Adafruit_SPIFlash
// - Validates stack pointer and reset vector before jumping
//
// Assumptions:
// - Application starts at FLASH_BASE_ADDR (0x00004000)
// - Bootloader starts at BOOT2_START_ADDR (0x00079000)
// - QSPI uses 0-based addressing (flash.readBuffer(offset, ...))
// - Each internal flash row is 256 bytes, composed of 4 pages of 64 bytes
// - Internal SRAM is 192 KB: 0x20000000 to 0x2002FFFF
// - Vector table begins at FLASH_BASE_ADDR
//----------------------------------------------------------------------------------------
__attribute__((section(".boot2"), used, noinline))
void Boot2(void)
{
    const uint32_t page_size   = 64;         // Size of one flash page
    const uint32_t row_size    = 256;        // Size of one flash row (4 pages)
    const uint32_t probe_block = 4096;       // Block size for scanning QSPI (4 KB)

    uint8_t write_buf[page_size];            // Buffer to write one page at a time
    uint8_t probe_buf[probe_block];          // Buffer to probe blocks from QSPI

    uint32_t offset = 0;                     // Offset into the QSPI flash
    uint32_t program_size = 0;               // Final detected size of firmware
    uint32_t i;

NVIC_SystemReset();
    //------------------------------------------------------------------------------------
    // STEP 1: Scan QSPI flash for valid content (first non-0xFF region)
    while (true)
    {
        if ((FLASH_BASE_ADDR + program_size + probe_block) > BOOT2_START_ADDR)
            break; // Prevent overwrite of bootloader

        if (!flash.readBuffer(program_size, probe_buf, probe_block))
            break; // QSPI read failed

        bool all_ff = true;
        for (i = 0; i < probe_block; i++)
        {
            if (probe_buf[i] != 0xFF) { all_ff = false; break; }
        }
        if (all_ff) break;
        program_size += probe_block;
    }

    if (program_size == 0)
    {
        if (IDE) Serial.println(F("No valid application found in QSPI ℹ️"));
        NVIC_SystemReset();
    }

    if (IDE)
    {
        Serial.println(F("Copying QSPI firmware to internal flash..."));
        Serial.print(F("Detected size: ")); Serial.println(program_size);
        Serial.print(F("BOOT2 start: 0x")); Serial.println(BOOT2_START_ADDR, HEX);
    }

    //------------------------------------------------------------------------------------
    // STEP 2: Disable interrupts, watchdog, cache, and SysTick
    //------------------------------------------------------------------------------------
    Serial.flush();
    __disable_irq();

    WDT->CTRLA.reg = 0;                     // Disable watchdog
    while(WDT->SYNCBUSY.bit.ENABLE);

    CMCC->CTRL.bit.CEN = 0;                 // Disable cache
    for (i = 0; i < 8; i++) NVIC->ICER[i] = NVIC->ICPR[i] = 0xFFFFFFFF;
    SysTick->CTRL = SysTick->LOAD = SysTick->VAL = 0;


   NVIC_SystemReset();

    //------------------------------------------------------------------------------------
    // STEP 3: Copy each row from QSPI to internal flash
    //------------------------------------------------------------------------------------
 //   for (offset = 0; offset < program_size; offset += row_size)
       for (offset = 0x27000; offset < program_size; offset += row_size)
    {
        uint32_t target_addr = FLASH_BASE_ADDR + offset;

        // Erase 256-byte flash row
        NVMCTRL->ADDR.reg = target_addr / 2;
        NVMCTRL->CTRLA.reg = (0xA5 << 8) | 0x02;
        while (!NVMCTRL->INTFLAG.bit.DONE);
        NVMCTRL->INTFLAG.reg = NVMCTRL_INTFLAG_DONE;

        // Write 4 pages of 64 bytes
        for (uint32_t page_offset = 0; page_offset < row_size; page_offset += page_size)
        {
            uint32_t src_offset = offset + page_offset;
            uint32_t dst_addr = target_addr + page_offset;

            if(!flash.readBuffer(src_offset, write_buf, page_size))
                NVIC_SystemReset();                                                              // Fatal: QSPI read failure

            uint32_t* dst = (uint32_t*)dst_addr;
            for (i = 0; i < page_size; i += 4)
                *dst++ = *((uint32_t*)&write_buf[i]);

            NVMCTRL->ADDR.reg = dst_addr / 2;
            NVMCTRL->CTRLA.reg = (0xA5 << 8) | 0x04;
            while (!NVMCTRL->INTFLAG.bit.DONE);
            NVMCTRL->INTFLAG.reg = NVMCTRL_INTFLAG_DONE;
        }
    }
return;


    //------------------------------------------------------------------------------------
    // STEP 4: Validate and jump to application
    //------------------------------------------------------------------------------------
    uint32_t app_msp           = *((uint32_t*)(FLASH_BASE_ADDR));
    uint32_t app_reset_handler = *((uint32_t*)(FLASH_BASE_ADDR + 4));

    bool valid_msp   = (app_msp >= RAM_START) && (app_msp < RAM_END);
    bool valid_reset = (app_reset_handler >= FLASH_BASE_ADDR) && (app_reset_handler < BOOT2_START_ADDR);

    if (!valid_msp || !valid_reset)
    {
        if (IDE) Serial.println(F("Invalid MSP or Reset_Handler 🚫"));
        NVIC_SystemReset();
        while (1) { __NOP(); }
    }
    
    for (i = 0; i < 8; i++) NVIC->ICER[i] = NVIC->ICPR[i] = 0xFFFFFFFF;
    SysTick->CTRL = SysTick->LOAD = SysTick->VAL = 0;
    CMCC->CTRL.bit.CEN = 0;

    __set_MSP(app_msp);                     // Set Main Stack Pointer
    SCB->VTOR = FLASH_BASE_ADDR;            // Redirect vector table

    if (IDE)
    {
      __enable_irq();
        Serial.println(F("Jumping to application 💥"));
        Serial.flush();
    }

    ((void (*)(void))app_reset_handler)();  // Final jump (never returns)
}
