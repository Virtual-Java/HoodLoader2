/*
Copyright(c) 2014-2015 NicoHood
See the readme for credit to other people.

This file is part of Hoodloader2.

Hoodloader2 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Hoodloader2 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Hoodloader2.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __BOARD_HOODLOADER_H__
#define __BOARD_HOODLOADER_H__

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif


/** • SoftwareReset:		(Arduino modification to support DebugWire)
*  Due to SoftwareReset it is possible to optionaly replace the capacitor
*  between DTR (pin 20) of the USB MCU and RESET pin (PB6) of the main MCU by a resistor.
*  Besides it enables ISP programming without a bypass capacitor of 10 μF
*  usually necessary to be connected to main MCU's RESET and GND.
*  This modification consumes 4 Bytes of flash memory only
*  and the best thing is it doesn't affect upload to Arduinos with capacitor!
*/
#define USING_SOFTWARE_RESET

/** • PowerSwitch:			(Arduino modification to support DebugWire)
*  Mosfet soldered between +5V and the output of the voltage regulator
*  to switch on/off power supply of the main MCU by software running on USB MCU.
*  However the power supply of the USB MCU is not affected.
*  The default VCCEN pin is PB5 to control the active low p-channel mosfet.
*  This modification consumes 4 Bytes of flash memory additionally
*  and doesn't affect standard Arduinos at all!
*/
#define WITH_VCC_ENABLE
#ifdef WITH_VCC_ENABLE
	#define VCCEN_ACTIVE_HIGH  false // false (default) for p-ch Mosfet (recommanded)
	// change this to true for n-ch Mosfet (tricky to use because of inflated GS-Voltage)
#endif

	/* Includes: */
		#include <LUFA/Common/Common.h>
		#include <LUFA/Drivers/Board/LEDs.h>

	/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			extern "C" {
		#endif

	/* Preprocessor Checks: */
		#if !defined(__INCLUDE_FROM_BOARD_H)
			#error Do not include this file directly. Include LUFA/Drivers/Board/Board.h instead.
		#endif

	/* Public Interface - May be used in end-application: */
		/* Macros: */
			/** Indicates the board has hardware LEDs mounted. */
			#define BOARD_HAS_LEDS
			
			// USB VID/PID settings
			#define LUFA_VID					0x03EB
			#define LUFA_PID					0x204A

			#define ARDUINO_VID					0x2341
			#define ARDUINO_UNO_PID				0x0043 // R3 (0001 R1)
			#define ARDUINO_MEGA_PID			0x0042 // R3 (0010 R1)
			#define ARDUINO_MEGA_ADK_PID		0x0044 // R3 (003F R1)
			#define ARDUINO_LEONARDO_PID		0x0036 // Bootloader, not program!
			#define ARDUINO_MICRO_PID   		0x0037 // Bootloader, not program!
			#define ARDUINO_DUE_PID             0x003D
			
			// USB product string settings
			#if (PRODUCTID == ARDUINO_UNO_PID)
			#define USB_DESCRIPTOR_STRING L"HoodLoader2 Uno"
			#elif (PRODUCTID == ARDUINO_MEGA_PID)
			#define USB_DESCRIPTOR_STRING L"HoodLoader2 Mega"
			#elif (PRODUCTID == ARDUINO_ADK_PID)
			#define USB_DESCRIPTOR_STRING L"HoodLoader2 ADK"
			#elif (PRODUCTID == ARDUINO_LEONARDO_PID)
			#define USB_DESCRIPTOR_STRING L"HoodLoader2 Leo"
			#elif (PRODUCTID == ARDUINO_MICRO_PID)
			#define USB_DESCRIPTOR_STRING L"HoodLoader2 Micro"
			#elif (PRODUCTID == ARDUINO_DUE_PID)
			#define USB_DESCRIPTOR_STRING L"HoodLoader2 Due"
			#else
			#define USB_DESCRIPTOR_STRING L"HoodLoader2 Lufa"
			#endif
			
			// Arduino Due 16u2
			#if (PRODUCTID == ARDUINO_DUE_PID)

			#error DUE is currently not supported because of its reset mechanism. See Issue #16 on Github for more information.

			#define AVR_RESET_LINE_PORT PORTC
			#define AVR_RESET_LINE_DDR DDRC
			#define AVR_RESET_LINE_MASK (1 << 7)

			#define AVR_ERASE_LINE_PORT PORTC
			#define AVR_ERASE_LINE_DDR DDRC
			#define AVR_ERASE_LINE_MASK (1 << 6)
			
			#define AUTORESET_PORT PORTB
			#define AUTORESET_DDR DDRB
			#define AUTORESET_PIN PINB
			#define AUTORESET_MASK (1 << PB6) // D6
			
			/* Inline Functions: */
		#if !defined(__DOXYGEN__)
			static inline void Board_Init(void)
			{
				// On Due reset and erase pins are on PortC instead of PortD
				// Portmanipulation with cbi and sbi will be used here
				DDRD |= LEDS_ALL_LEDS | (1 << PD3);
				PORTD |= (1 << PD2);
	
				// INACTIVE => set as INPUT (internal pullup on target /RESET keep it at 3.3v)
				AVR_RESET_LINE_DDR  &= ~AVR_RESET_LINE_MASK;
				AVR_RESET_LINE_PORT &= ~AVR_RESET_LINE_MASK;
	
				// Target /ERASE line is active HIGH: there is a mosfet that inverts logic
				AVR_ERASE_LINE_PORT |= AVR_ERASE_LINE_MASK;
				AVR_ERASE_LINE_DDR  |= AVR_ERASE_LINE_MASK;	
			}
			
			static inline void Board_Reset(bool reset)
			{
				/* Target /RESET line  */
				if (reset) {
					/* ACTIVE   => OUTPUT LOW (0v on target /RESET) */
					AVR_RESET_LINE_DDR  |= AVR_RESET_LINE_MASK;
					AVR_RESET_LINE_PORT &= ~AVR_RESET_LINE_MASK;
				} 
				else {
				 	/* INACTIVE => set as INPUT (internal pullup on target /RESET keep it at 3.3v) */
					AVR_RESET_LINE_DDR  &= ~AVR_RESET_LINE_MASK;
					AVR_RESET_LINE_PORT &= ~AVR_RESET_LINE_MASK;
				}
			}
			
			static inline void Board_Erase(bool erase)
			{
				#ifdef AUTORESET_JUMPER
				if(!(AUTORESET_PIN & AUTORESET_MASK))
					return;
				#endif
				
				if (erase)
					AVR_ERASE_LINE_PORT &= ~AVR_ERASE_LINE_MASK;
				else
					AVR_ERASE_LINE_PORT |= AVR_ERASE_LINE_MASK;
			}
		#endif

			// Arduino Leonardo/Micro
			#elif defined(__AVR_ATmega32U4__)
			
			/** Pin that can reset the main MCU. */
			// PORTB would also be possible (D8-11 + SPI)
			// I will not use it since PB contains the only PCINT
			// And the pins on PD are not better or worse
			#define AVR_RESET_LINE_PORT PORTD
			#define AVR_RESET_LINE_DDR DDRD
			#define AVR_RESET_LINE_PIN PIND
			#define AVR_RESET_LINE_MASK (1 << PD4) // PD4 = D4, PD6 = D12, PD7 = D7
			
			#define AUTORESET_PORT PORTB
			#define AUTORESET_DDR DDRB
			#define AUTORESET_PIN PINB
			#define AUTORESET_MASK (1 << PB4) // D8
			
			/* Inline Functions: */
		#if !defined(__DOXYGEN__)
			static inline void Board_Init(void)
			{
				// We use = here since the pins should be input/low anyways.
				// This saves us some more bytes for flash
				DDRD = LEDMASK_TX | (1 << PD3) | AVR_RESET_LINE_MASK;
			 	// Results in sbi instructions
				DDRB  |= LEDMASK_RX;
				PORTD |= AVR_RESET_LINE_MASK;
				PORTD |= (1 << PD2);
			}
			
			static inline void Board_Reset(bool reset)
			{
				if (reset)
					AVR_RESET_LINE_PORT &= ~AVR_RESET_LINE_MASK;
				else
					AVR_RESET_LINE_PORT |= AVR_RESET_LINE_MASK;
			}
			
			static inline void Board_Erase(bool erase)
			{
				// No erase pin
			}
		#endif
			
			// Arduino Uno/Mega 8/16/32u2
			#else
			
			/* Pin that can reset the main MCU. */
			#define AVR_RESET_LINE_PORT PORTD
			#define AVR_RESET_LINE_DDR DDRD
			#define AVR_RESET_LINE_PIN PIND
			#define AVR_RESET_LINE_MASK (1 << PD7)
			
			/* Pin that is used to de-/activate (0/1) Autoreset */
			#define AUTORESET_PORT PORTB
			#define AUTORESET_DDR DDRB
			#define AUTORESET_PIN PINB
			#define AUTORESET_MASK (1 << PB6) // D6
			// For my modificated Uno board only:
			//#define AUTORESET_PORT PORTD
			//#define AUTORESET_DDR DDRD
			//#define AUTORESET_PIN PIND
			//#define AUTORESET_MASK (1 << PD6) // pin12 RTS

			/* Pin that can power the main MCU */
			#define AVR_VCCEN_LINE_PORT  PORTB
			#define AVR_VCCEN_LINE_DDR   DDRB
			#define AVR_VCCEN_LINE_MASK  (1 << PB5)
			// For my modificated Uno board only:
			//#define AVR_VCCEN_LINE_PORT  PORTC
			//#define AVR_VCCEN_LINE_DDR   DDRC
			//#define AVR_VCCEN_LINE_MASK  (1 << PC4)

			
			/* Inline Functions: */
		#if !defined(__DOXYGEN__)
			static inline void Board_Init(void)
			{
				/* switch on the main MCU */
				#ifdef WITH_VCC_ENABLE
					AVR_VCCEN_LINE_DDR  |= AVR_VCCEN_LINE_MASK; // Set VCCEN pin to output
					#if(VCCEN_ACTIVE_HIGH)
						AVR_VCCEN_LINE_PORT |= AVR_VCCEN_LINE_MASK; // VCCEN is HIGH active (n-ch mosfet)
					#else
						AVR_VCCEN_LINE_PORT &= ~AVR_VCCEN_LINE_MASK; // VCCEN is LOW active (p-ch mosfet)
					#endif
				#endif
				DDRD |= LEDS_ALL_LEDS | (1 << PD3) | AVR_RESET_LINE_MASK;
				PORTD |= AVR_RESET_LINE_MASK;
				PORTD |= (1 << PD2);
			}
			
			static inline void Board_Reset(bool reset)
			{
				#ifdef AUTORESET_JUMPER
				if(!(AUTORESET_PIN & AUTORESET_MASK))
					return;
				#endif

				if (reset)
				{
					AVR_RESET_LINE_PORT &= ~AVR_RESET_LINE_MASK;
					#ifdef USING_SOFTWARE_RESET
					// Changing this delay to specific values between 3 and 42 μs saves some bytes of flash (due to perfect timer interrupt)
					// 4 Bytes for 3, 6, 12, 18, 24, 30, 36, 42 μs
					// 6 Bytes for 1, 2, 5, 7, 8, 10, 20, 40, 46, μs,
					// 10 Bytes for 200, 500, 1000 μs (values > 48 μs)
					// 1, 2, 3, 5, 12, 20, 40, 200, 500 μs values tested with Arduino Uno R3 @ 16 MHz, so the valid range is: 1 to 500 μs; values like 800, 1000 μs didn't work for me
					// 12 μs seams to bee a good default value
					_delay_us(12);  // wait 12 microsecords to reset main MCU on boards with resistor instead of capacitor on reset line
					AVR_RESET_LINE_PORT |= AVR_RESET_LINE_MASK;
					#endif
				}
				else
					AVR_RESET_LINE_PORT |= AVR_RESET_LINE_MASK;
			}
			
			static inline void Board_Erase(bool erase)
			{
				// No erase pin
			}
		#endif
			
			#endif // Arduino Uno/Mega 8/16/32u2

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif

#endif

