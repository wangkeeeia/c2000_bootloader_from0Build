//###########################################################################
//
// FILE:   cpu1brom_boot_modes.h
//
// TITLE:  Contains boot modes and entry addresses
//
//###########################################################################
// $TI Release: F28E12x Support Library v26.01.00.00 $
// $Release Date: 05-20-2026 $
// $Copyright:
// Copyright (C) 2026 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
// 
//   Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the 
//   documentation and/or other materials provided with the   
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//###########################################################################

#ifndef BROM_BOOT_MODES_H
#define BROM_BOOT_MODES_H

//
// BootROM System Clock (10MHz)
//
#define BOOTROM_SYSCLK         8000000UL

//
// Boot Mode Values
//
#define PARALLEL_BOOT           0x00U   //0,1,2,3,4,5,6,7 - 16,11
#define PARALLEL_BOOT_ALT1      0x20U   //0,1,2,3,5,6,7,24 - 12,13

#define SCI_BOOT                0x01U   //GPIO29; GPIO28 (CCARD)
#define SCI_BOOT_ALT1           0x21U   //GPIO1;  GPIO0
#define SCI_BOOT_ALT2           0x41U   //GPIO8;  GPIO9
#define SCI_BOOT_ALT3           0x61U   //GPIO7;  GPIO3
#define SCI_BOOT_ALT4           0x81U   //GPIO16; GPIO3


#define FLASH_BOOT              0x03U   //BANK0 sector 0
#define FLASH_BOOT_ALT1         0x23U   //BANK0 sector 0

#define WAIT_BOOT               0x04U   //with WDOG enabled
#define WAIT_BOOT_ALT1          0x24U   //without WDOG enabled

#define RAM_BOOT                0x05U

#define SPI_CONTROLLER_BOOT         0x06U   //GPIO 7,1,3,5
#define SPI_CONTROLLER_BOOT_ALT1    0x26U   //GPIO 16,1,3,0
#define SPI_CONTROLLER_BOOT_ALT2    0x46U   //GPIO 8,10,9,11
#define SPI_CONTROLLER_BOOT_ALT3    0x66U   //GPIO 16,13,12,29

#define I2C_CONTROLLER_BOOT         0x07U   //GPIO0(SDA), GPIO1(SCL)
#define I2C_CONTROLLER_BOOT_ALT1    0x27U   //GPIO32 (SDA), GPIO33 (SCL)
#define I2C_CONTROLLER_BOOT_ALT2    0x47U   //GPIO5(SDA), GPIO4 (SCL)

#define SECURE_FLASH_BOOT           0x0AU    //BANK0 sector 0
#define SECURE_FLASH_BOOT_ALT1      0x2AU    //BANK0 sector 0

#define INVALID_BOOT                0xFFU    //Invalid Boot Mode

#define BOOTMODE_MASK               0x0FU    //mask to know which kind of boot mode we are in
//
// Entry Addresses
//
#define FLASH_ENTRY_POINT       0x00080000UL    //BANK0 sector 0
#define FLASH_ENTRY_POINT_ALT1  0x00088000UL    //BANK0 sector 0

#define RAM_ENTRY_POINT         0x000000U       //M0 start address

//
// Misc
//
#define BROM_EIGHT_BIT_HEADER   0x08AAU

//
// Bootloader function pointer
//
typedef uint16_t (*uint16fptr)(void);
extern  uint16fptr GetWordData;

//
// Function Prototypes
//
extern uint32_t GetLongData(void);
extern void CopyData(void);
extern void ReadReservedFn(void);

#endif // BROM_BOOT_MODES_H
