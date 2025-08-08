
// ~/Arduino/QIF/db.h Located in parent directory and linked in subdirectory

#ifndef   DB_H
#define   DB_H

#define PIN00  0
#define PIN01  1                                                                
#define PIN04  4
#define PIN05  5  
#define PIN06  6                                                                                  
#define PIN09  9
#define PIN10  10 
#define PIN11  11 
#define PIN12  12
#define PIN13  13
#define PIN14  14  
#define PIN15  15
#define PIN16  16
#define PIN17  17     
#define PIN18  18
#define PIN19  19
#define PIN21  21                                                                             // SDA 
#define PIN22  22                                                                             // SCL
#define PIN23  23 
#define PIN24  24 
#define PIN25  25



const  uint8_t NB_PINS = 21;                                                                  // Number of available I/O pins per board

//      EasyEDA   Arduino#  Microchip                                                         // gpio# correspondance to Arduino nb#, GPIOXX reference from old schematic
#define GPIO00    1         // PB16                                                         
#define GPIO01    0         // PB17
#define GPIO02    21        // PA12/SDA                                                                             
#define GPIO03    22        // PA13/SCL                                                                               
#define GPIO04    4         // PA14
#define GPIO05    5         // PA15  
#define GPIO06    6         // PA18                                                                                  
#define GPIO08    23        // PB22
#define GPIO09    9         // PA19
#define GPIO10    10        // PA20
#define GPIO11    11        // PA21
#define GPIO12    12        // PA22
#define GPIO13    13        // PA23              
#define GPIO14    25        // PA17/SCK
#define GPIO15    24        // PB23/MOSI
#define GPIO24    18        // PA04/A4
#define GPIO25    19        // PA06/A5
#define GPIO26    14        // PA02/A0
#define GPIO27    15        // PA05/A1
#define GPIO28    16        // PB08/A2
#define GPIO29    17        // PB09/A3

#define BTNP0 GPIO28                                                                            // Switch board button pins
#define BTNP1 GPIO29 
#define BTNP2 GPIO24 
#define BTNP3 GPIO25
#define BTNP4 GPIO14  
#define BTNP5 GPIO15 
#define BTNP6 GPIO08
#define BTNP7 GPIO01

#define LED00 GPIO00                                                                            // Switch board led pins
#define LED01 GPIO04
#define LED02 GPIO13
#define LED03 GPIO12
#define LED04 GPIO11
#define LED05 GPIO10
#define LED06 GPIO09
#define LED07 GPIO06

#define PWCTRL   GPIO13

// Offset from the CAN base address for switches
#define sw0     0x00
#define sw1     0x01
#define sw2     0x02
#define sw3     0x03
#define sw4     0x04
#define sw5     0x05
#define sw6     0x06
#define sw7     0x07

// Offset from the CAN base address for leds
#define led0    0x00
#define led1    0x01
#define led2    0x02
#define led3    0x03
#define led4    0x04
#define led5    0x05
#define led6    0x06
#define led7    0x07

// Offset from the CAN base address for low power pwm

#define pwm0   0x00
#define pwm1   0x01
#define pwm2   0x02
#define pwm3   0x03
#define pwm4   0x04
#define pwm5   0x05
#define pwm6   0x06
#define pwm7   0x07
#define pwm8   0x08
#define pwr    0x09                                                          // Power control

// Offset from the CAN base address 0x000, Time = 0, Reset = 1 and so on
// Used for service processing message (base address + offset)
enum SERVICE : uint8_t
  {
    Time,                                                                                         // Time broadcast
    Reset,                                                                                        // Reboot
    Update,                                                                                       // Rx update
    Abme,                                                                                         // Alarm BME
    Hbt,                                                                                          // Heartbeat
    Ack,                                                                                          // Acknowledge
    Nack,                                                                                         // Negative acknowledge
    Gps,                                                                                          // GPS module                        
    Gyro,                                                                                         // Gyroscope module
    Anl = 0x09,                                                                                   // Analog return channel
    Level,                                                                                        // Water/Liquide level detector
    Pir,                                                                                          // PIR detector
    Ladar,                                                                                        // Ladar detector
    Coder,                                                                                        // Optical encoder
    Wind,                                                                                         // Wind speed module                                                                                
    Bme = 0x0F                                                                                    // BME sensor
  };                     

typedef void (*FunctionPointer)(const CANFDMessage &);                                            // Matches FilterCallback
typedef void (*LinkAddress)(uint16_t);                                                            // Link address (CAN)

enum TYPE:uint8_t                                                                                 // Module type
  {
	  UNDEF,                                                                                        // Board does not exist
    SWITCH,                                                                                       // Switch
    LPOWER,                                                                                       // Low power
    MPOWER,                                                                                       // Medium power
    HPOWER                                                                                        // High power         
  };

enum pin:uint8_t                                                                                  // Pin initialization                                                                                                                                                         
  {
    UNSET,                                                                                        // Default (input / analog)
    OUT,                                                                                          // Output(totem-pôle)
    INP,                                                                                          // Input(pull-up)
    PWM                                                                                           // Pulse width modulation                                     
  };


const char *mode[] = { "UNDEF", "SWITCH", "LPOWER", "MPOWER", "HPOWER"};                          // Board mode

enum base : uint16_t                                                                              // 120 boards base address
{
    board000 = 0x000, board001 = 0x010, board002 = 0x020, board003 = 0x030,
    board004 = 0x040, board005 = 0x050, board006 = 0x060, board007 = 0x070,
    board008 = 0x080, board009 = 0x090, board010 = 0x0a0, board011 = 0x0b0,
    board012 = 0x0c0, board013 = 0x0d0, board014 = 0x0e0, board015 = 0x0f0,

    board016 = 0x100, board017 = 0x110, board018 = 0x120, board019 = 0x130,
    board020 = 0x140, board021 = 0x150, board022 = 0x160, board023 = 0x170,
    board024 = 0x180, board025 = 0x190, board026 = 0x1a0, board027 = 0x1b0,
    board028 = 0x1c0, board029 = 0x1d0, board030 = 0x1e0, board031 = 0x1d0,

    board032 = 0x200, board033 = 0x210, board034 = 0x220, board035 = 0x230,
    board036 = 0x240, board037 = 0x250, board038 = 0x260, board039 = 0x270,
    board040 = 0x280, board041 = 0x290, board042 = 0x2a0, board043 = 0x2b0,
    board044 = 0x2c0, board045 = 0x2d0, board046 = 0x2e0, board047 = 0x2f0,

    board048 = 0x300, board049 = 0x310, board050 = 0x320, board051 = 0x330,
    board052 = 0x340, board053 = 0x350, board054 = 0x360, board055 = 0x370,
    board056 = 0x380, board057 = 0x390, board058 = 0x3a0, board059 = 0x3b0,
    board060 = 0x3c0, board061 = 0x3d0, board062 = 0x3e0, board063 = 0x3f0,

    board064 = 0x400, board065 = 0x410, board066 = 0x420, board067 = 0x430,
    board068 = 0x440, board069 = 0x450, board070 = 0x460, board071 = 0x470,
    board072 = 0x480, board073 = 0x490, board074 = 0x4a0, board075 = 0x4b0,
    board076 = 0x4c0, board077 = 0x4d0, board078 = 0x4e0, board079 = 0x4f0,

    board080 = 0x500, board081 = 0x510, board082 = 0x520, board083 = 0x530,
    board084 = 0x540, board085 = 0x550, board086 = 0x560, board087 = 0x570,
    board088 = 0x580, board089 = 0x590, board090 = 0x5a0, board091 = 0x5b0,
    board092 = 0x5c0, board093 = 0x5d0, board094 = 0x5e0, board095 = 0x5f0,

    board096 = 0x600, board097 = 0x610, board098 = 0x620, board099 = 0x630,
    board100 = 0x640, board101 = 0x650, board102 = 0x660, board103 = 0x670,
    board104 = 0x680, board105 = 0x690, board106 = 0x6a0, board107 = 0x6b0,
    board108 = 0x6c0, board109 = 0x6d0, board110 = 0x6e0, board111 = 0x6f0,

    board112 = 0x700, board113 = 0x710, board114 = 0x720, board115 = 0x730,
    board116 = 0x740, board117 = 0x750, board118 = 0x760, board119 = 0x770,
    board120 = 0x780, board121 = 0x790, board122 = 0x7a0, board123 = 0x7b0,
    board124 = 0x7c0, board125 = 0x7d0, board126 = 0x7e0, board127 = 0x7f0
};

enum uid:uint64_t                                                                                         // 128 boards UID (UUID truncated)
  {
    uid000  = 0xffffffffffff, uid001  = 0x4fc844355348, uid002  = 0xc7e869175348, uid003  = 0x648c8ce65348,
    uid004  = 0xd3ff7d36534c, uid005  = 0x119bacab5348, uid006  = 0xe18fba535348, uid007  = 0x992f74165348,
    uid008  = 0xbe7d4c395348, uid009  = 0x7350ef11534c, uid010  = 0xedfeb5945348, uid011  = 0x21080886534c,
    uid012  = 0xbe0d94f9534c, uid013  = 0x8363ac12534c, uid014  = 0xdce2e5e2534c, uid015  = 0xa456cc63534c,

    uid016  = 0x1fe45adb5348, uid017  = 0x588cad905348, uid018  = 0xc83eaf86534c, uid019  = 0xd57cd6655348,
    uid020  = 0x8065ea62534c, uid021  = 0x42b7e5695348, uid022  = 0x0a13d0575348, uid023  = 0xba522264534c,
    uid024  = 0x0ef33f8d534c, uid025  = 0x50b17a56534c, uid026  = 0xd976c934534c, uid027  = 0x86c0407c534c,
    uid028  = 0x1254bf0f534c, uid029  = 0x1aa221f1534c, uid030  = 0x4d8b907a534c, uid031  = 0x4927d65e5348,

    uid032  = 0xe1dd1522534c, uid033  = 0x75f175f7534c, uid034  = 0xcfe96cfe5348, uid035  = 0x8aa0deff534c,
    uid036  = 0xcc7f4baa534c, uid037  = 0x11b21a88534c, uid038  = 0x9fa17d2c5348, uid039  = 0x28507f3f534c,
    uid040  = 0xeb28b6f9534c, uid041  = 0x05d371485348, uid042  = 0x1370a0f15348, uid043  = 0xfbba64725348,
    uid044  = 0xa02dc50c5348, uid045  = 0xe3e5a6585348, uid046  = 0x89c58bb7534c, uid047  = 0xf06d14625348,

    uid048  = 0x9ea1820a5348, uid049  = 0xd4702ab95348, uid050  = 0x8695c1185348, uid051  = 0x4fc844355348,
    uid052  = 0xffffffffffff, uid053  = 0xffffffffffff, uid054  = 0xffffffffffff, uid055  = 0xffffffffffff,
    uid056  = 0xffffffffffff, uid057  = 0xffffffffffff, uid058  = 0xffffffffffff, uid059  = 0xffffffffffff,
    uid060  = 0xffffffffffff, uid061  = 0xffffffffffff, uid062  = 0xffffffffffff, uid063  = 0xffffffffffff,

    uid064  = 0xffffffffffff, uid065  = 0xffffffffffff, uid066  = 0xffffffffffff, uid067  = 0xffffffffffff,
    uid068  = 0xffffffffffff, uid069  = 0xffffffffffff, uid070  = 0xffffffffffff, uid071  = 0xffffffffffff,
    uid072  = 0xffffffffffff, uid073  = 0xffffffffffff, uid074  = 0xffffffffffff, uid075  = 0xffffffffffff,
    uid076  = 0xffffffffffff, uid077  = 0xffffffffffff, uid078  = 0xffffffffffff, uid079  = 0xffffffffffff,

    uid080  = 0xffffffffffff, uid081  = 0xffffffffffff, uid082  = 0xffffffffffff, uid083  = 0xffffffffffff,
    uid084  = 0xffffffffffff, uid085  = 0xffffffffffff, uid086  = 0xffffffffffff, uid087  = 0xffffffffffff,
    uid088  = 0xffffffffffff, uid089  = 0xffffffffffff, uid090  = 0xffffffffffff, uid091  = 0xffffffffffff,
    uid092  = 0xffffffffffff, uid093  = 0xffffffffffff, uid094  = 0xffffffffffff, uid095  = 0xffffffffffff,

    uid096  = 0xffffffffffff, uid097  = 0xffffffffffff, uid098  = 0xffffffffffff, uid099  = 0xffffffffffff,
    uid100  = 0xffffffffffff, uid101  = 0x88F918495348, uid102  = 0x905113065348, uid103  = 0xffffffffffff,
    uid104  = 0xffffffffffff, uid105  = 0xffffffffffff, uid106  = 0xffffffffffff, uid107  = 0xffffffffffff,
    uid108  = 0xffffffffffff, uid109  = 0xffffffffffff, uid110  = 0xffffffffffff, uid111  = 0xffffffffffff,

    uid112  = 0xffffffffffff, uid113  = 0xffffffffffff, uid114  = 0xffffffffffff, uid115  = 0xffffffffffff,
    uid116  = 0xffffffffffff, uid117  = 0xffffffffffff, uid118  = 0xffffffffffff, uid119  = 0xffffffffffff,
    uid120  = 0xffffffffffff, uid121  = 0xffffffffffff, uid122  = 0xffffffffffff, uid123  = 0xffffffffffff,
    uid124  = 0xffffffffffff, uid125  = 0xffffffffffff, uid126  = 0xffffffffffff, uid127  = 0xffffffffffff                                                                                      
  };

/*
  Purpose: Represents an individual pin with a specific function (e.g., input, output) and a unique number.
  Bit-fields: Compact storage using bit-fields (7 bits total per Pin).
*/
struct Pin
  {                                                                                                        
    uint8_t FUNCTION:2;                                                                                   // Define the Pin structure: UNSET, OUTPUT, INPUT, PWM
    uint8_t NUMBER:5;                                                                                     // Pin number
  };

/*
  Purpose: Groups multiple Pin structures into one, making it easier to manage pins collectively.
  Use Case: Represents all pins for a board, like a control board or power board.
*/
struct Pins
  {                                             
    Pin pin[NB_PINS];                                                                                     // Define the Pins structure to encapsulate an array of 21 Pins elements
  };

/*
  Purpose: Represents an I/O board with identification, configuration, and pin details.
  Attributes:
  CAN, LBL, TYPE, and BCK are configuration details for each board.
  PINS contains the pin configuration for the board.
  Pins Initialization for Different Boards
*/
struct IO                                                                                                 // Define the IO structure
  {
    uint64_t  UID;                                                                                        // Truncated UUID, byte:15,14,13,12,1,0 of UUID
    uint16_t  CAN;                                                                                        // CAN address, 11 bits
    uint8_t   LBL;                                                                                        // Label, 7 bits
    uint8_t   TYPE;                                                                                       // Type, 4 bits
    FunctionPointer fct[16];                                                                              // Array of 16 function pointer
    int16_t  lnk[16];                                                                                     // Array of 16 link address
  };


/*
  Purpose: Configures all pins for a specific board type (e.g., LPOWER board).
  Each element specifies the function (PWM, INP, OUT, UNSET) and the pin number (PIN00, etc.).
  This pattern is repeated for MPOWER_PIN, HPOWER_PIN, and SWITCH_PIN.
*/

struct Pins LPOWER_PIN =
  {                                                                                                       // Define the PIN initialization for LPOWER board
    {                                                                                        
      {OUT,   PIN00}, {OUT,   PIN01}, {UNSET, PIN04}, {OUT,   PIN05},
      {OUT,   PIN06}, {OUT,   PIN09}, {OUT,   PIN10}, {OUT,   PIN11},
      {OUT,   PIN12}, {OUT,   PIN13}, {OUT,   PIN14}, {UNSET, PIN15},
      {UNSET, PIN16}, {UNSET, PIN17}, {UNSET, PIN18}, {UNSET, PIN19},
      {UNSET, PIN21}, {UNSET, PIN22}, {UNSET, PIN23}, {UNSET, PIN24},
      {UNSET, PIN25}
    }
  };

struct Pins MPOWER_PIN =
  {                                                                                                       // Define the PIN initialization for MPOWER board
    {
      {OUT,   PIN00},  {OUT,   PIN01}, {OUT,   PIN04}, {OUT,   PIN05},
      {OUT,   PIN06},  {OUT,   PIN09}, {OUT,   PIN10}, {OUT,   PIN11},
      {OUT,   PIN12},  {OUT,   PIN13}, {UNSET, PIN14}, {UNSET, PIN15},
      {UNSET, PIN16},  {UNSET, PIN17}, {UNSET, PIN18}, {OUT,   PIN19},
      {UNSET, PIN21},  {UNSET, PIN22}, {OUT,   PIN23}, {OUT,   PIN24},
      {UNSET, PIN25}
    }
  };

struct Pins HPOWER_PIN =
  {                                                                                                       // Define the PIN initialization for HPOWER board   
    {                                                                                                     
      {UNSET, PIN00},  {UNSET, PIN01},  {UNSET, PIN04},  {UNSET, PIN05},
      {UNSET, PIN06},  {UNSET, PIN09},  {UNSET, PIN10},  {UNSET, PIN11},
      {UNSET, PIN12},  {UNSET, PIN13},  {UNSET, PIN14},  {UNSET, PIN15},
      {UNSET, PIN16},  {UNSET, PIN17},  {UNSET, PIN18},  {UNSET, PIN19},
      {UNSET, PIN21},  {UNSET, PIN22},  {UNSET, PIN23},  {UNSET, PIN24},
      {UNSET, PIN25}
    }
  };

struct Pins SWITCH_PIN =
  {                                                                                                         // Define PIN initialization for a SWITCH board
    {                                                                                                
      {INP,   PIN00}, {OUT, PIN01},   {OUT, PIN04},   {UNSET, PIN05},
      {OUT,   PIN06}, {OUT, PIN09},   {OUT, PIN10},   {OUT, PIN11},
      {OUT,   PIN12}, {OUT, PIN13},   {UNSET, PIN14}, {UNSET, PIN15},
      {INP,   PIN16}, {INP, PIN17},   {INP, PIN18},   {INP, PIN19},
      {UNSET, PIN21}, {UNSET, PIN22}, {INP, PIN23},   {INP, PIN24},
      {INP,   PIN25}
    }
  };

struct Pins UNSET_PIN =                                                                                          
  {                                                                                                         // Define the default PIN initialization
    {
      {UNSET, PIN00}, {UNSET, PIN01}, {UNSET, PIN04}, {UNSET, PIN05},
      {UNSET, PIN06}, {UNSET, PIN09}, {UNSET, PIN10}, {UNSET, PIN11},
      {UNSET, PIN12}, {UNSET, PIN13}, {UNSET, PIN14}, {UNSET, PIN15},
      {UNSET, PIN16}, {UNSET, PIN17}, {UNSET, PIN18}, {UNSET, PIN19},
      {UNSET, PIN21}, {UNSET, PIN22}, {UNSET, PIN23}, {UNSET, PIN24},
      {UNSET, PIN25}
    }
  };

//----------------------------------------------------------------------------------------
/*
  Initialized DB structure with default values
  Each IO entry in the database is initialized with its UID, CAN address, and default pin configuration (UNSET_PIN).
  The use of bit-fields and structures ensures efficient memory usage.
  Scalable Design: Allows defining multiple types of boards with distinct pin configurations.
  Default Configuration: Ensures all entries are initialized, even if specific configurations aren't available.
  Fields used in a cold start module setup
  .UID  Module indentification: UUID truncated
  .CAN  Module CAN bus address
  .LBL  Module sticker value
  .TYPE Module type: UNDEF, SWITCH, LPOWER, MPOWER
  .fct  16 pointers to function
  Calling a function, DB[label].fct[0]();                   
*/
int16_t None = -1;                                                                                  // Used for linked board -1 means not linked

const IO DB[128] = {
  { .UID = uid000, .CAN = board000, .LBL = 0, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid001, .CAN = board001, .LBL = 1, .TYPE = LPOWER,                                       // Board defined as a low power.........to be continued
    .fct = { FCT00,FCT01,FCT02,FCT03,FCT04,FCT05,FCT06,FCT07,
             FCT08,FCT09,FCT10,FCT11,FCT12,FCT13,FCT14,FCT15 },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,board002+Bme } },

  { .UID = uid002, .CAN = board002, .LBL = 2, .TYPE = SWITCH,                                       // Board defined as a switch
    .fct = { FCT00,FCT01,FCT02,FCT03,FCT04,FCT05,FCT06,FCT07,
             FCT08,FCT09,FCT10,FCT11,FCT12,FCT13,FCT14,FCT15 },
    .lnk = { board004+led0,board004+led1,board004+led2,board004+led3,                               // Send value to board004
             board004+led6,board004+led5,board004+led6,board004+led7 } },

  { .UID = uid003, .CAN = board003, .LBL = 3, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid004, .CAN = board004, .LBL = 4, .TYPE = SWITCH,                                       // Board defined as a switch
    .fct = { FCT00,FCT01,FCT02,FCT03,FCT04,FCT05,FCT06,FCT07,
             FCT08,FCT09,FCT10,FCT11,FCT12,FCT13,FCT14,FCT15 },
    .lnk = { board002+led0,board002+led1,board002+led2,board002+led3,                               // Send value to board002
             board002+led6,board002+led5,board002+led6,board002+led7 } },

  { .UID = uid005, .CAN = board005, .LBL = 5, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid006, .CAN = board006, .LBL = 6, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid007, .CAN = board007, .LBL = 7, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid008, .CAN = board008, .LBL = 8, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid009, .CAN = board009, .LBL = 9, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid010, .CAN = board010, .LBL = 10, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid011, .CAN = board011, .LBL = 11, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid012, .CAN = board012, .LBL = 12, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid013, .CAN = board013, .LBL = 13, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid014, .CAN = board014, .LBL = 14, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid015, .CAN = board015, .LBL = 15, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid016, .CAN = board016, .LBL = 16, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid017, .CAN = board017, .LBL = 17, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid018, .CAN = board018, .LBL = 18, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid019, .CAN = board019, .LBL = 19, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid020, .CAN = board020, .LBL = 20, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid021, .CAN = board021, .LBL = 21, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid022, .CAN = board022, .LBL = 22, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid023, .CAN = board023, .LBL = 23, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid024, .CAN = board024, .LBL = 24, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid025, .CAN = board025, .LBL = 25, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid026, .CAN = board026, .LBL = 26, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid027, .CAN = board027, .LBL = 27, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid028, .CAN = board028, .LBL = 28, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid029, .CAN = board029, .LBL = 29, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid030, .CAN = board030, .LBL = 30, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid031, .CAN = board031, .LBL = 31, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid032, .CAN = board032, .LBL = 32, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid033, .CAN = board033, .LBL = 33, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid034, .CAN = board034, .LBL = 34, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid035, .CAN = board035, .LBL = 35, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid036, .CAN = board036, .LBL = 36, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid037, .CAN = board037, .LBL = 37, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid038, .CAN = board038, .LBL = 38, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid039, .CAN = board039, .LBL = 39, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid040, .CAN = board040, .LBL = 40, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid041, .CAN = board041, .LBL = 41, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid042, .CAN = board042, .LBL = 42, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid043, .CAN = board043, .LBL = 43, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid044, .CAN = board044, .LBL = 44, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid045, .CAN = board045, .LBL = 45, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid046, .CAN = board046, .LBL = 46, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid047, .CAN = board047, .LBL = 47, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid048, .CAN = board048, .LBL = 48, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid049, .CAN = board049, .LBL = 49, .TYPE = LPOWER,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid050, .CAN = board050, .LBL = 50, .TYPE = LPOWER,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid051, .CAN = board051, .LBL = 51, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid052, .CAN = board052, .LBL = 52, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid053, .CAN = board053, .LBL = 53, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid054, .CAN = board054, .LBL = 54, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid055, .CAN = board055, .LBL = 55, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid056, .CAN = board056, .LBL = 56, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid057, .CAN = board057, .LBL = 57, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid058, .CAN = board058, .LBL = 58, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid059, .CAN = board059, .LBL = 59, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid060, .CAN = board060, .LBL = 60, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid061, .CAN = board061, .LBL = 61, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid062, .CAN = board062, .LBL = 62, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid063, .CAN = board063, .LBL = 63, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid064, .CAN = board064, .LBL = 64, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid065, .CAN = board065, .LBL = 65, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid066, .CAN = board066, .LBL = 66, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid067, .CAN = board067, .LBL = 67, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid068, .CAN = board068, .LBL = 68, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid069, .CAN = board069, .LBL = 69, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid070, .CAN = board070, .LBL = 70, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid071, .CAN = board071, .LBL = 71, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid072, .CAN = board072, .LBL = 72, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid073, .CAN = board073, .LBL = 73, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid074, .CAN = board074, .LBL = 74, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid075, .CAN = board075, .LBL = 75, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid076, .CAN = board076, .LBL = 76, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid077, .CAN = board077, .LBL = 77, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid078, .CAN = board078, .LBL = 78, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid079, .CAN = board079, .LBL = 79, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid080, .CAN = board080, .LBL = 80, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid081, .CAN = board081, .LBL = 81, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid082, .CAN = board082, .LBL = 82, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid083, .CAN = board083, .LBL = 83, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid084, .CAN = board084, .LBL = 84, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid085, .CAN = board085, .LBL = 85, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid086, .CAN = board086, .LBL = 86, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid087, .CAN = board087, .LBL = 87, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid088, .CAN = board088, .LBL = 88, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid089, .CAN = board089, .LBL = 89, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid090, .CAN = board090, .LBL = 90, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid091, .CAN = board091, .LBL = 91, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid092, .CAN = board092, .LBL = 92, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid093, .CAN = board093, .LBL = 93, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid094, .CAN = board094, .LBL = 94, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid095, .CAN = board095, .LBL = 95, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid096, .CAN = board096, .LBL = 96, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid097, .CAN = board097, .LBL = 97, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid098, .CAN = board098, .LBL = 98, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid099, .CAN = board099, .LBL = 99, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid100, .CAN = board100, .LBL = 100, .TYPE = SWITCH,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { board102+led0,board102+led1,board102+led2,board102+led3,                           // Send value to board102
             board102+led6,board102+led5,board102+led6,board102+led7 } },

  { .UID = uid101, .CAN = board101, .LBL = 101, .TYPE = MPOWER,                                 // Board defined as a switch
    .fct = { FCT00,FCT01,FCT02,FCT03,FCT04,FCT05,FCT06,FCT07,
             FCT08,FCT09,FCT10,FCT11,FCT12,FCT13,FCT14,FCT15 },
    .lnk = { None,None,None,None,None,None,None,None,                           
             None,None,None,None,None,None,None,None } },

  { .UID = uid102, .CAN = board102, .LBL = 102, .TYPE = SWITCH,                                 // Board defined as a switch
    .fct = { FCT00,FCT01,FCT02,FCT03,FCT04,FCT05,FCT06,FCT07,
             FCT08,FCT09,FCT10,FCT11,FCT12,FCT13,FCT14,FCT15 },
    .lnk = { board100+led0,board100+led1,board100+led2,board100+led3,                           // Send value to board100
             board100+led6,board100+led5,board100+led6,board100+led7 } },

  { .UID = uid103, .CAN = board103, .LBL = 103, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid104, .CAN = board104, .LBL = 104, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid105, .CAN = board105, .LBL = 105, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid106, .CAN = board106, .LBL = 106, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid107, .CAN = board107, .LBL = 107, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid108, .CAN = board108, .LBL = 108, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid109, .CAN = board109, .LBL = 109, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid110, .CAN = board110, .LBL = 110, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid111, .CAN = board111, .LBL = 111, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid112, .CAN = board112, .LBL = 112, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid113, .CAN = board113, .LBL = 113, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid114, .CAN = board114, .LBL = 114, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid115, .CAN = board115, .LBL = 115, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid116, .CAN = board116, .LBL = 116, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid117, .CAN = board117, .LBL = 117, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid118, .CAN = board118, .LBL = 118, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid119, .CAN = board119, .LBL = 119, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid120, .CAN = board120, .LBL = 120, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid121, .CAN = board121, .LBL = 121, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid122, .CAN = board122, .LBL = 122, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid123, .CAN = board123, .LBL = 123, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid124, .CAN = board124, .LBL = 124, .TYPE = UNDEF,
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,
             None,None,None,None,None,None,None,None } },

  { .UID = uid125, .CAN = board125, .LBL = 125, .TYPE = UNDEF,                                 
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,                                                                            
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },
    .lnk = { None,None,None,None,None,None,None,None,                                           
             None,None,None,None,None,None,None,None } },

  { .UID = uid126, .CAN = board126, .LBL = 126, .TYPE = UNDEF,                                 
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },                                       
    .lnk = { None,None,None,None,None,None,None,None,                           
             None,None,None,None,None,None,None,None } },                                                  

  { .UID = uid127, .CAN = board127, .LBL = 127, .TYPE = UNDEF,                                 
    .fct = { DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,
             DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY,DUMMY },                                                                   
    .lnk = { None,None,None,None,None,None,None,None,                                                                            
             None,None,None,None,None,None,None,None } }                                        
};

const uint8_t DB_count = sizeof(DB) / sizeof(DB[0]);

#endif