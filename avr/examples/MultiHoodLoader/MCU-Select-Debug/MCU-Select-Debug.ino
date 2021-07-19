/*
 * Copyright(©) 2021 Jonathan Vetter
 * Thanks a lot to Nico for writing this great USB Bootloader
 * This file is part of MultiHoodLoader, a modified version of Hoodloader to support:
 * • PowerSwitch
 * • SoftwareReset
 * • DebugWire
 * • MultiBoot
 */

// PowerSwitch:===========================================================================

//  Mosfet soldered between +5V and the output of the voltage regulator 
//  to switch of power supply of main MCU by software running on USB MCU
//  However the power supply of the USB MCU is not affected.
//  This modification doesn't affect standard Arduinos reset mechanism at all!!!
//  (Arduino modification to support DebugWire)

// SoftwareReset:=========================================================================

//  Due to SoftwareReset it is possible to remove the
//  Capacitor between DTR (pin 20) of USB MCU and RESET pin (PB6) of main MCU
//  Besides it enables ISP programming without an additional 100 μF capacitor
//  between main MCU's RESET and GND to bypass the internal capacitor
//  This modification only consumes 4 Bytes of flash memory
//  and the best thing is it doesn't affect upload to Arduinos with capacitor!
//  (Arduino modification to support DebugWire)

//  DebugWire:============================================================================
//  Thanks to the missing capacitor between DTR and RESET from now on it is possible to
//  use DebugWire, a shared pin function with RESET on the reset pin of the target IO-MCU

// Multiboot:=============================================================================

//  MultiHoodLoader is a modification of Hoodloader to support MultiBootLoading
//
//  The IO-MCU to be bootloaded by the USB-MCU is selected via Serial Monitor at Baud 300.
//
//  An 8-bit wide address (0 to 255) selects the device to be bootload.
//  The reset pin of the target is either selected directly via one pin (on PortB),
//  therefore a maximum of 8 targets is possible to address.
//
//  To support up to 255 devices an address decoder(74HC138/74HC238),
//  decoder with latch (74HC137/74HC237) or for best performance an 
//  (digital/analog) demultiplexer (HEF4051B/74HC4051).
//
//  When more than 8 outputs are desired a 16-channel multiplexer 
//  (HEF4067/74HC4067) can be used instead, but usually it is more
//  flexible and cheaper to chain multiple ICs taking inverters (HEF4007,HEF4049,HEF4069/
//  74HC04/74HC14/74HC4049)
//

//  Instructions:=========================================================================
//  
//  To select the target device enter #xyz in serial monitor. 
//  '#' symbolizes a number and represents a mask to prevent conflicts with other 
//  content sent on serial port at Baud 300 (default), 
//  while xyz is a decimal number in the range from 0 to 255.
//  0 stands for the main MCU on Arduino Uno/Mega Boards, the numbers from 1 to 255 stand for the PORT_MASK!!! on PORTB
//
//  The address of the target mcu is kept in memory until it is OVERWRITTEN by a new value or cleared on POWER loose.
//  It isn't cleared during BOOT process or RESET (of the USB-MCU residing MultiHoodLoader),
//  unless the register GPIOR0 was explicitly overwritten by the program code!!!
//
//  UPLOADING a new sketch to the USB-MCU doesn't effect the address kept in memory!
//  Therefore keep pointing to the previously selected target 
//  while updating the USB-MCUs application sketch (which should include MCU-Select) is possible.

#define INCLUDE_DEBUG_CODE // Include debug code
#define TEST 13 // Select the desired test code
//#define DUMMY_CODE // Dummy code; no write, read address value only!!!

class MultiHoodLoader {
  public:
  MultiHoodLoader() {
    
  }
  void begin () {
    this->BaudRate = 300;
    Serial.begin(0); // Start USB Serial
  }
  
  void begin (uint32_t BaudRate) {
    this->BaudRate = BaudRate;
    Serial.begin(0); // Start USB Serial
  }
  /*
  void setMemoryAddress (uint8_t address) {
    this->addr = address;
  }
  */
  void update () {
    uint8_t readAvailable = Serial.available();
    if (Serial.baud() == BaudRate && readAvailable > 0) {
      //Serial.readBytes(buf, len); // needs 18 bytes more than Serial.read()
      //for(uint8_t b = 0; b < len; b++) {
      //  buf[b] = Serial.read();
      //}
      if(Serial.read() == '#') {
        volatile uint8_t prev = 0;
        volatile uint8_t test = 0;
        volatile uint8_t mcu = 0;
        bool digit = true;
        for(uint8_t b = 1; b < len && b < (readAvailable-1); b++) {
          buf[b] = Serial.read();
          //if(!(isdigit(buf[b]) || buf[b] == ' ')) {
          //  digit = false;
          //  break;
          //}
          if(isdigit(buf[b])) {
            mcu = (mcu * 10) + (uint8_t) (buf[b] - '0');
          } else {
            if(!(buf[b] == ' ')) {
              digit = false;
              break;
            }
          }
        }
#ifndef INCLUDE_DEBUG_CODE
        if(digit) {        
          Serial.print(F("MCU: "));
          Serial.print(mcu);
        } else {
          Serial.print(F("NAN"));
          //Serial.print(buf[b]);
        }
        Serial.print('\n');
        (*(volatile uint8_t *)(addr)) = mcu;
#else // Include debug code
  // GPIO = General Purpose Input Output (register) ≈ the port registers
  // IOMM = Input Output Mapped Memory (location) ≈ memory containing GPIOs and perpherals
  // Tests were performed with a setup consisting of:
  // • An Arduino Micro (AtMega32u4); 
  // • Sketches were compiled with gcc5.4
  // • Compiled size was determined with Hoodloader version 2.0.5
  
  // Do nothing to compare code size
  #if(TEST == 0)
    // Empty sketch containing HoodLoaders makros
    // With debug code EXcluded this Sketch (MCU-Select) compiles to 3898 bytes (user size);
    // With debug code INcluded this Sketch (MCU-Select) compiles to 4032 bytes (reference);
    // Rest of the code is 632 bytes in size (compared with empty sketch) yet

  // Store address to IOMM and read value from IOMM afterwards using C/C++ pointers
  #elif(TEST == 1)
    #ifndef DUMMY_CODE
    // uses 20 bytes of code
      //#define FIX_ADDR  PORTC
      // NEVER WRITE TO A RESERVED GPIO REGISTER!!!
      // The value read can differ from the one written before!
      //#define FIX_ADDR  0x1A // Reserved
      #define FIX_ADDR  GPIOR0
        (*(volatile uint8_t *)(FIX_ADDR)) = mcu;
        test = (*(volatile uint8_t *)(FIX_ADDR)); // is working
    #else // DUMMY CODE, read addr value only!!!
    // uses 10 bytes of code
      //#define FIX_ADDR  PORTC
      // NEVER WRITE TO A RESERVED GPIO REGISTER!!!
      // The value read can differ from the one written before!
      //#define FIX_ADDR  0x1A // Reserved
      #define FIX_ADDR  GPIOR0
        test = (*(volatile uint8_t *)(FIX_ADDR)); // is working
    #endif
    
  // Store address to IOMM and read value from IOMM afterwards using C/C++ pointers
  #elif(TEST == 2)
    #ifndef DUMMY_CODE
    // uses 28 bytes of code
      //#define FIX_ADDR  PORTC
      // NEVER WRITE TO A RESERVED GPIO REGISTER!!!
      // The value read can differ from the one written before!
      //#define FIX_ADDR  0x1A // Reserved
      #define FIX_ADDR  GPIOR0
         // TODO: outputs wrong values;
         // when FIXADDR is GPIOR0 address lowest digit is read/output as 0 all the time
         // e.g. addr was 26 previous ==> prev = 20; addr 2n, n∈ℕ ∧ 0≤n≤9 ==> prev = 20
        prev = (*(volatile uint8_t *)(FIX_ADDR)); // does not work, outputs wrong values
        (*(volatile uint8_t *)(FIX_ADDR)) = mcu;
        test = (*(volatile uint8_t *)(FIX_ADDR)); // is working
    #else // DUMMY CODE, read addr value only!!!
    // uses 20 bytes of code
      //#define FIX_ADDR  PORTC
      // NEVER WRITE TO A RESERVED GPIO REGISTER!!!
      // The value read can differ from the one written before!
      //#define FIX_ADDR  0x1A // Reserved
      #define FIX_ADDR  GPIOR0
         // TODO: outputs wrong values;
         // when FIXADDR is GPIOR0 address lowest digit is read/output as 0 all the time
         // e.g. addr was 26 previous ==> prev = 20; addr 2n, n∈ℕ ∧ 0≤n≤9 ==> prev = 20
        prev = (*(volatile uint8_t *)(FIX_ADDR)); // does not work, outputs wrong values
        test = (*(volatile uint8_t *)(FIX_ADDR)); // is working
    #endif

  // Store address to IOMM and read value from IOMM afterwards using assembly
  #elif(TEST == 3)
  // uses 12 bytes of code
    #ifndef DUMMY_CODE
      //#define FIX_ADDR  PORTC
      // NEVER WRITE TO A RESERVED GPIO REGISTER!!!
      // The value read can differ from the one written before!
      //#define FIX_ADDR  0x1A // Reserved
      #define FIX_ADDR  GPIOR0
        asm volatile(
              "out %[ADDR], %[MCU]  \n"
              "in %[TST], %[ADDR]   \n"
            : [TST] "=r" (test) : [MCU] "r" (mcu), [ADDR] "M" (_SFR_IO_ADDR(FIX_ADDR))
        );
    #else // DUMMY CODE, read addr value only!!!
    // uses 8 bytes of code
      //#define FIX_ADDR  PORTC
      // NEVER WRITE TO A RESERVED GPIO REGISTER!!!
      // The value read can differ from the one written before!
      //#define FIX_ADDR  0x1A // Reserved
      #define FIX_ADDR  GPIOR0
        asm volatile(
              "in %[TST], %[ADDR]   \n"
            : [TST] "=r" (test) : [ADDR] "M" (_SFR_IO_ADDR(FIX_ADDR))
        );
    #endif

  // Store address to IOMM and read value from IOMM afterwards using assembly
  #elif(TEST == 4)
  // uses 16 bytes of code
    //#define FIX_ADDR  PORTC
    // NEVER WRITE TO A RESERVED GPIO REGISTER!!!
    // The value read can differ from the one written before!
    //#define FIX_ADDR  0x1A // Reserved
    #define FIX_ADDR  GPIOR0
        asm volatile(
              "in %[PRV], %[ADDR]   \n"
              "out %[ADDR], %[MCU]  \n"
              "in %[TST], %[ADDR]   \n"
            : [PRV] "=&r" (prev), [TST] "=r" (test) : [MCU] "r" (mcu), [ADDR] "M" (_SFR_IO_ADDR(FIX_ADDR))
        );
  
  // Store address to IOMM and read value from IOMM afterwards using assembly
  #elif(TEST == 5)
        
        asm(
              "out %[ADDR], %[MCU]  \n"
            : : [MCU] "r" (mcu), [ADDR] "M" (FIX_ADDR)
        );
        asm(
              //"in __tmp_reg__, __SREG__  \n"
              //"cli                       \n"
              "in %[TST], %[ADDR]  \n"
              //"out __SREG__, __tmp_reg__ \n"
            : [TST] "=r" (test) : [ADDR] "M" (FIX_ADDR)
        );
  // Store address to IOMM and read value from IOMM afterwards using assembly
  #elif(TEST == 6)
        #define FIX_ADDR  0x08 // PORTC
        asm volatile(
              "out %[ADDR], %[MCU]  \n"
            : : [MCU] "r" (mcu), [ADDR] "M" (FIX_ADDR)
        );
        asm volatile(
              "in __tmp_reg__, __SREG__  \n" // SREG keeps unchanged ==> cmd not necessary
              "cli                       \n" // SREG keeps unchanged ==> cmd not necessary
              "in %[TST], %[ADDR]  \n"
              "out __SREG__, __tmp_reg__ \n" // SREG keeps unchanged ==> cmd not necessary
            : [TST] "=r" (test) : [ADDR] "M" (0x06)
        );
  // TODO: Test variable is always 0 in assembly code
  // store address to reserved EXTIO location using assembler
  #elif(TEST == 7)
        asm (
          "st  Z, %[MCU] \n"
          : : [MCU] "r" (mcu), [ADDR] "z" (addr + 0x20)
        );
        asm (
          "ld %[TST], Z \n"
          : : [TST] "r" (test), [ADDR] "z" (addr + 0x20)
        );
  // store address to reserved EXTIO location using assembler        
  #elif(TEST == 11)
    #ifndef DUMMY_CODE
    // uses 20 bytes of code
      //#define FIX_ADDR  PORTC
      // NEVER WRITE TO A RESERVED GPIO REGISTER!!!
      // The value read can differ from the one written before!
      //#define FIX_ADDR  0x1A // Reserved
      //#define FIX_ADDR  GPIOR0
      #define FIX_ADDR  GPIOR1
      //#define FIX_ADDR  GPIOR2
        (*(volatile uint8_t *)(FIX_ADDR)) = mcu;
        test = (*(volatile uint8_t *)(FIX_ADDR)); // is working
    #else // DUMMY CODE, read addr value only!!!
    // uses 10 bytes of code
      //#define FIX_ADDR  PORTC
      // NEVER WRITE TO A RESERVED GPIO REGISTER!!!
      // The value read can differ from the one written before!
      //#define FIX_ADDR  0x1A // Reserved
      //#define FIX_ADDR  GPIOR0
      #define FIX_ADDR  GPIOR1
      //#define FIX_ADDR  GPIOR2
        test = (*(volatile uint8_t *)(FIX_ADDR)); // is working
    #endif
    
  // Store address to EXTIO and read value from EXTIO afterwards using C/C++ pointers
  #elif(TEST == 12)
    #ifndef DUMMY_CODE
    // uses 28 bytes of code
      //#define FIX_ADDR  PORTC
      // NEVER WRITE TO A RESERVED GPIO REGISTER!!!
      // The value read can differ from the one written before!
      //#define FIX_ADDR  0x1A // Reserved
      //#define FIX_ADDR  GPIOR0
      #define FIX_ADDR  GPIOR1
      //#define FIX_ADDR  GPIOR2
         // TODO: outputs wrong values;
         // when FIXADDR is GPIOR0 address lowest digit is read/output as 0 all the time
         // e.g. addr was 26 previous ==> prev = 20; addr 2n, n∈ℕ ∧ 0≤n≤9 ==> prev = 20
        prev = (*(volatile uint8_t *)(FIX_ADDR)); // does not work, outputs wrong values
        (*(volatile uint8_t *)(FIX_ADDR)) = mcu;
        test = (*(volatile uint8_t *)(FIX_ADDR)); // is working
    #else // DUMMY CODE, read addr value only!!!
    // uses 20 bytes of code
      //#define FIX_ADDR  PORTC
      // NEVER WRITE TO A RESERVED GPIO REGISTER!!!
      // The value read can differ from the one written before!
      //#define FIX_ADDR  0x1A // Reserved
      //#define FIX_ADDR  GPIOR0
      #define FIX_ADDR  GPIOR1
      //#define FIX_ADDR  GPIOR2
         // TODO: outputs wrong values;
         // when FIXADDR is GPIOR0 address lowest digit is read/output as 0 all the time
         // e.g. addr was 26 previous ==> prev = 20; addr 2n, n∈ℕ ∧ 0≤n≤9 ==> prev = 20
        prev = (*(volatile uint8_t *)(FIX_ADDR)); // does not work, outputs wrong values
        test = (*(volatile uint8_t *)(FIX_ADDR)); // is working
    #endif

  // Store address to EXTIO and read value from EXTIO afterwards using assembly
  #elif(TEST == 13)
  // uses 16 bytes of code
    #ifndef DUMMY_CODE
      //#define FIX_ADDR  PORTC
      // NEVER WRITE TO A RESERVED GPIO REGISTER!!!
      // The value read can differ from the one written before!
      //#define FIX_ADDR  0x1A // Reserved
      //#define FIX_ADDR  GPIOR0
      #define FIX_ADDR  GPIOR1
      //#define FIX_ADDR  GPIOR2
        asm volatile(
              "sts %[ADDR], %[MCU]  \n"
              "lds %[TST], %[ADDR]   \n"
            : [TST] "=r" (test) : [MCU] "r" (mcu), [ADDR] "X" (FIX_ADDR)
        );
    #else // DUMMY CODE, read addr value only!!!
    // uses 10 bytes of code
      //#define FIX_ADDR  PORTC
      // NEVER WRITE TO A RESERVED GPIO REGISTER!!!
      // The value read can differ from the one written before!
      //#define FIX_ADDR  0x1A // Reserved
      //#define FIX_ADDR  GPIOR0
      #define FIX_ADDR  GPIOR1
      //#define FIX_ADDR  GPIOR2
        asm volatile(
              "lds %[TST], %[ADDR]   \n"
            : [TST] "=r" (test) : [ADDR] "X" (FIX_ADDR)
        );
    #endif

  // Store address to EXTIO and read value from EXTIO afterwards using assembly
  #elif(TEST == 14)
  // uses 32 bytes of code
    //#define FIX_ADDR  PORTC
    // NEVER WRITE TO A RESERVED GPIO REGISTER!!!
    // The value read can differ from the one written before!
    //#define FIX_ADDR  0x1A // Reserved
    //#define FIX_ADDR  GPIOR0
    #define FIX_ADDR  GPIOR1
    //#define FIX_ADDR  GPIOR2
        asm volatile(
              "lds %[PRV], %[ADDR]   \n"
              "sts %[ADDR], %[MCU]  \n"
              "lds %[TST], %[ADDR]   \n"
            : [PRV] "=&r" (prev), [TST] "=r" (test) : [MCU] "r" (mcu), [ADDR] "X" (_SFR_IO_ADDR(FIX_ADDR))
        );
// TODO: uint16_t MCU = 0; 16-bit target address (pm_hi(GPIOR2), pm_lo(GPIOR1))





  // Store address to IOMM and read value from IOMM afterwards using assembly
  #elif(TEST == 19)
  // uses 10 bytes of code
    //#define FIX_ADDR  PORTC
    // NEVER WRITE TO A RESERVED GPIO REGISTER!!!
    // The value read can differ from the one written before!
    //#define FIX_ADDR  0x1A // Reserved
    //#define FIX_ADDR  GPIOR0
    #define FIX_ADDR  GPIOR1
    //#define FIX_ADDR  GPIOR2

        asm volatile(
              "sts %[ADDR], %[MCU]  \n"
            : : [MCU] "r" (mcu), [ADDR] "X" (FIX_ADDR)
        );
    
//    uint8_t dummy;
//        asm volatile(
//              "ldi %[MCU],    pm_lo(FIX_ADDR) \n"
//              "sts %[hook],   %[MCU]     \n"
//              "ldi %[MCU],    pm_hi(FIX_ADDR) \n"
//              "sts %[hook]+1, %[MCU]     \n"
//            : [MCU] "=r" (mcu) : [hook] "M" (GPIOR1)
//        );
    
//        asm volatile(
//              "ldi %[MCU],    %[ADDR_lo] \n"
//              "sts %[hook],   %[MCU]     \n"
//              "ldi %[MCU],    %[ADDR_lo] \n"
//              "sts %[hook]+1, %[MCU]     \n"
//            : "=d" (dummy) : [MCU] "r" (mcu), [hook] "s" (, [ADDR_lo] "M" (pm_lo(FIX_ADDR)), [ADDR_hi] "M" (pm_hi(FIX_ADDR))
//        );
  #endif
  Serial.print(F("Prev: "));
  Serial.print(prev);
  Serial.print('\n');
  Serial.print(F("MCU: "));
  Serial.print(mcu);
  Serial.print('\n');
  Serial.print(F("TST: "));
  Serial.print(test);
  Serial.print('\n');
#endif
        }
      }
    }

  private:
    const uint8_t addr = 0x1A; // a reserved IO register location on Arduino Micro/Leonardo (AtMega8u2, 16u2, 32u2, 16u2, 32u4) and Arduino Mega (AtMega1280, 2560)
    uint32_t BaudRate;
    static const uint8_t len = 4;
    uint8_t buf[len];
    static const uint8_t numOffset = 48;
};

MultiHoodLoader myHL;
//MultiHoodLoader* myHL = new MultiHoodLoader();

void setup() {
  myHL.begin(300);
  DDRC |= 0xC0;
}

void loop() {
  myHL.update();
}
