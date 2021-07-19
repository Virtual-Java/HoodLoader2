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

//
//  
//  To select the target device enter #xyz in serial monitor. 
//  '#' symbolizes a number and represents a mask to prevent conflicts with other 
//  content sent on serial port at Baud 300 (default), 
//  while xyz is a decimal number in the range from 0 to 255.
//  0 stands for the main MCU on Arduino Uno/Mega Boards, the numbers from 1 to 255 stand for the PORT_MASK!!! on PORTB
//

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

  void setMemoryAddress (uint8_t address) {
    this->addr = address;
  }
  void update () {
    uint8_t readAvailable = Serial.available();
    if (Serial.baud() == BaudRate && readAvailable > 0) {
      //Serial.readBytes(buf, len); // needs 18 bytes more than Serial.read()
      //for(uint8_t b = 0; b < len; b++) {
      //  buf[b] = Serial.read();
      //}
      if(Serial.read() == '#') {

        Serial.print("Prev: "); // debug code
        Serial.print(test);     // debug code
        Serial.print('\n');     // debug code
        
        uint8_t mcu = 0;
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
        if(digit) {        
          Serial.print(F("MCU: "));
          Serial.print(mcu);
        } else {
          Serial.print(F("NAN"));
          //Serial.print(buf[b]);
        }
        Serial.print('\n');
        (*(volatile uint8_t *)(addr)) = mcu;
#ifdef INCLUDE_DEBUG_CODE
  // TODO: Ensure the address isn't cleared until next boot
  #if (TEST == 1)
        test = (*(volatile uint8_t *)(addr)); // is working
  // TODO: Test variable is always 0 in assembly code
  #elif (TEST == 2) // store address to reserved MMIO location using assembler
        asm (
              "out %[ADDR], %[MCU]  \n"
            : : [MCU] "r" (mcu), [ADDR] "I" (addr)
        );
        asm (
              "in %[TEST], %[ADDR]  \n"
            : [TEST] "=r" (test) : [ADDR] "I" (addr)
        );
  // TODO: Test variable is always 0 in assembly code
  #elif (TEST == 3) // store address to reserved MMIO location using assembler
        asm (
          "st  Z, %[MCU] \n"
          : : [MCU] "r" (mcu), [ADDR] "z" (addr + 0x20)
        );
        asm (
          "ld %[TEST], Z \n"
          : : [TEST] "r" (test), [ADDR] "z" (addr + 0x20)
        );
  #endif
          Serial.print("TEST: ");
          Serial.print(test);
          Serial.print('\n');
#endif
        }
      }
    }

  private:
    uint8_t addr = 0x1A; // a reserved IO register location on Arduino Micro/Leonardo (AtMega8u2, 16u2, 32u2, 16u2, 32u4) and Arduino Mega (AtMega1280, 2560)
    uint32_t BaudRate;
    static const uint8_t len = 4;
    uint8_t buf[len];
    static const uint8_t numOffset = 48;
    uint8_t mcu = 0;
    
    volatile uint8_t test = 0;
};

MultiHoodLoader myHL;
//MultiHoodLoader* myHL = new MultiHoodLoader();

void setup() {
  myHL.begin(300);
}

void loop() {
  myHL.update();
}
