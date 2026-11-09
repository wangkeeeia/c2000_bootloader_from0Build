//###########################################################################
//
// FILE:   f28e12x_flash_kernel_ex3_sci_flash_kernel.c
//
//###########################################################################
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

//
// Included Files
//

#include <string.h>
#include "flash_programming_f28e12x.h" // Flash API example header file
#include "FlashTech_F28E1xx_C28x.h"
#include "Types.h"
#include "cpu1bootrom.h"
#include "device.h"
#include "flash.h"
#include "gpio.h"
#include "interrupt.h"
#include "driverlib.h"
#include "sci.h"


//
// Function Prototypes
//
void exampleError();
void Flash_initSectors(void);
extern uint32_t sciGetFunction(uint32_t BootMode);
extern void sciaFlush(void);

//
// Main - SCIB UART Test: GPIO18(TX) / GPIO19(RX)
//
uint32_t main(void)
{
//
// Step 1. Minimal init: Flash kernel already runs from RAM.
//         ROM boot has configured PLL, disabled WDG, enabled all peripheral
//         clocks. Device_init() is NOT called because it reconfigures PLL
//         and accesses flash (memcpy ramfuncs) which crashes in RAM-resident
//         flash kernel mode.
//
    Device_initGPIO();

//
// Step 2. Set LSPCLK divider to /4 so SCI baud calculation is correct.
//         DEVICE_LSPCLK_FREQ = DEVICE_SYSCLK_FREQ / 4 = 40 MHz (at 160 MHz).
//
    SysCtl_setLowSpeedClock(SYSCTL_LSPCLK_PRESCALE_4);

//
// Step 3. Disable interrupts
//
    DINT;
    Interrupt_initModule();
    Interrupt_initVectorTable();
    IER = 0x0000;
    IFR = 0x0000;

//
// Step 4. Configure GPIO18 as SCIB_TX and GPIO19 as SCIB_RX
//
    GPIO_setPinConfig(GPIO_18_SCIB_TX);
    GPIO_setDirectionMode(18, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(18, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(18, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(GPIO_19_SCIB_RX);
    GPIO_setDirectionMode(19, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(19, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(19, GPIO_QUAL_ASYNC);

//
// Step 5. Enable SCIB peripheral clock and release from system reset.
//         ROM boot code only enables SCIA clock; SCIB must be enabled manually.
//
    EALLOW;
    SysCtl_resetPeripheral(SYSCTL_PERIPH_RES_SCIB);
    HWREG(CPUSYS_BASE + SYSCTL_O_PERCLKCR + 2U) |= (1UL << 3U);
    EDIS;
    asm(" RPT #5 || NOP");

//
// Step 6. Initialize SCIB: 9600 baud, 8 data bits, 1 stop bit, no parity
//
    SCI_resetChannels(SCIB_BASE);
    SCI_setConfig(SCIB_BASE, DEVICE_LSPCLK_FREQ, 9600,
                  SCI_CONFIG_WLEN_8 | SCI_CONFIG_STOP_ONE | SCI_CONFIG_PAR_NONE);
    SCI_enableModule(SCIB_BASE);
    SCI_performSoftwareReset(SCIB_BASE);

//
// Step 7. Main loop: continuously send 't', respond 'r' when 'x' received
//
    uint16_t rxData;

    while(1)
    {
        SCI_writeCharBlockingNonFIFO(SCIB_BASE, 't');

        DEVICE_DELAY_US(200000);

        if(SCI_isDataAvailableNonFIFO(SCIB_BASE))
        {
            rxData = SCI_readCharBlockingNonFIFO(SCIB_BASE);
            if(rxData == 'x')
            {
                SCI_writeCharBlockingNonFIFO(SCIB_BASE, 'r');
            }
        }
    }

    return(0);
}

//
// Flash_initSectors - Initialize flash API and active flash bank sectors
//
void Flash_initSectors(void)
{
    EALLOW;
    Fapi_StatusType oReturnCheck;
    oReturnCheck = Fapi_initializeAPI(FlashTech_CPU0_BASE_ADDRESS, 150); 
    if(oReturnCheck != Fapi_Status_Success)
    {
        exampleError(oReturnCheck);
    }

    EDIS;
}

//
// exampleError - For this example, if an error is found just stop here
//
#ifdef __TI_COMPILER_VERSION__
    #if __TI_COMPILER_VERSION__ >= 15009000
        #pragma CODE_SECTION(exampleError,".TI.ramfunc");
    #else
        #pragma CODE_SECTION(exampleError,"ramfuncs");
    #endif
#endif
void exampleError()
{
    //
    // Error code will be in the status parameter
    //
    __asm("    ESTOP0");
}

//
// End of file
//