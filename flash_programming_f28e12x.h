//#############################################################################
//
// FILE:   flash_programming_f28e12x.h
//
// TITLE:  A set of Constant Values for the F28E12x Family.
//
//#############################################################################
//
//
// $Copyright:
// Copyright (C) 2025 Texas Instruments Incorporated - http://www.ti.com/
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
//#############################################################################
#ifndef FLASH_PROGRAMMING_F28E12X_H_
#define FLASH_PROGRAMMING_F28E12X_H_

// Bank0 Sector start addresses
#define FlashBank0StartAddress      0x80000U
#define     Bzero_Sector0_start     0x00080000U
#define     Bzero_Sector1_start     0x00080400U
#define     Bzero_Sector2_start     0x00080800U
#define     Bzero_Sector3_start     0x00080C00U
#define     Bzero_Sector4_start     0x00081000U
#define     Bzero_Sector5_start     0x00081400U
#define     Bzero_Sector6_start     0x00081800U
#define     Bzero_Sector7_start     0x00081C00U
#define     Bzero_Sector8_start     0x00082000U
#define     Bzero_Sector9_start     0x00082400U
#define     Bzero_Sector10_start    0x00082800U
#define     Bzero_Sector11_start    0x00082C00U
#define     Bzero_Sector12_start    0x00083000U
#define     Bzero_Sector13_start    0x00083400U
#define     Bzero_Sector14_start    0x00083800U
#define     Bzero_Sector15_start    0x00083C00U
#define     Bzero_Sector16_start    0x00084000U
#define     Bzero_Sector17_start    0x00084400U
#define     Bzero_Sector18_start    0x00084800U
#define     Bzero_Sector19_start    0x00084C00U
#define     Bzero_Sector20_start    0x00085000U
#define     Bzero_Sector21_start    0x00085400U
#define     Bzero_Sector22_start    0x00085800U
#define     Bzero_Sector23_start    0x00085C00U
#define     Bzero_Sector24_start    0x00086000U
#define     Bzero_Sector25_start    0x00086400U
#define     Bzero_Sector26_start    0x00086800U
#define     Bzero_Sector27_start    0x00086C00U
#define     Bzero_Sector28_start    0x00087000U
#define     Bzero_Sector29_start    0x00087400U
#define     Bzero_Sector30_start    0x00087800U
#define     Bzero_Sector31_start    0x00087C00U
#define     Bzero_Sector32_start    0x00088000U
#define     Bzero_Sector33_start    0x00088400U
#define     Bzero_Sector34_start    0x00088800U
#define     Bzero_Sector35_start    0x00088C00U
#define     Bzero_Sector36_start    0x00089000U
#define     Bzero_Sector37_start    0x00089400U
#define     Bzero_Sector38_start    0x00089800U
#define     Bzero_Sector39_start    0x00089C00U
#define     Bzero_Sector40_start    0x0008A000U
#define     Bzero_Sector41_start    0x0008A400U
#define     Bzero_Sector42_start    0x0008A800U
#define     Bzero_Sector43_start    0x0008AC00U
#define     Bzero_Sector44_start    0x0008B000U
#define     Bzero_Sector45_start    0x0008B400U
#define     Bzero_Sector46_start    0x0008B800U
#define     Bzero_Sector47_start    0x0008BC00U
#define     Bzero_Sector48_start    0x0008C000U
#define     Bzero_Sector49_start    0x0008C400U
#define     Bzero_Sector50_start    0x0008C800U
#define     Bzero_Sector51_start    0x0008CC00U
#define     Bzero_Sector52_start    0x0008D000U
#define     Bzero_Sector53_start    0x0008D400U
#define     Bzero_Sector54_start    0x0008D800U
#define     Bzero_Sector55_start    0x0008DC00U
#define     Bzero_Sector56_start    0x0008E000U
#define     Bzero_Sector57_start    0x0008E400U
#define     Bzero_Sector58_start    0x0008E800U
#define     Bzero_Sector59_start    0x0008EC00U
#define     Bzero_Sector60_start    0x0008F000U
#define     Bzero_Sector61_start    0x0008F400U
#define     Bzero_Sector62_start    0x0008F800U
#define     Bzero_Sector63_start    0x0008FC00U

#define FlashBank0EndAddress        0x8FFFFU

//Sector length in number of 16bits
#define Sector2KB_u16length   0x400U
#define Bank128KB_u16length   0x10000U

//Sector length in number of 32bits
#define Sector2KB_u32length   0x200U
#define Bank128KB_u32length   0x8000U

//Flash wrapper base address
#define FLASH_WRAPPER_PROGRAM_BASE         0x52000U /*(FLASH0CMD_BASE - 0x1000UL)*/

/*!
    \brief Define to map the direct access to the FMC registers.
*/
#define FlashTech_CPU0_BASE_ADDRESS ((Fapi_FmcRegistersType *)FLASH_WRAPPER_PROGRAM_BASE)

#endif /* FLASH_PROGRAMMING_F280015X_H_ */
