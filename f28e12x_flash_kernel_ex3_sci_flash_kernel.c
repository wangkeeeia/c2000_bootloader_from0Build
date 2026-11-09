//###########################################################################
//
// FILE:   f28e12x_flash_kernel_ex3_sci_flash_kernel.c
//
// TITLE:  F28E12 Bootloader - SCI Handshake & Flash Kernel
//
// DESCRIPTION:
//   Bootloader for F28E12 using SCIB (GPIO18/GPIO19).
//   Based on CMS32M bootloader architecture.
//   - Host sends 'B' every 20ms for handshake
//   - MCU echoes 'B' back on successful handshake
//   - 20ms timeout exits bootloader and jumps to application
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
#include "flash_programming_f28e12x.h"
#include "FlashTech_F28E1xx_C28x.h"
#include "Types.h"
#include "cpu1bootrom.h"
#include "device.h"
#include "flash.h"
#include "gpio.h"
#include "driverlib.h"
#include "sci.h"
#include "cputimer.h"

//
// Defines
//
#define COMMAND_NACK                0x01U
#define COMMAND_HANDSHAKE           'B'
#define COMMAND_READ_EEPROM         'F'
#define COMMAND_LOAD_EEPROM         'E'
#define COMMAND_READ_FLASH          'N'
#define COMMAND_LOAD_FLASH          'M'
#define COMMAND_SET_ADDRESS         'D'
#define COMMAND_CHANGE_BAUD         '*'
#define ETX                         0x03U
#define COMMAND_ERROR               '?'

#define TIMEOUT_MS                  20U
#define SYSCLK_FREQ                 160000000U
#define TIMEOUT_PERIOD              ((SYSCLK_FREQ / 1000U) * TIMEOUT_MS)

//
// Flash command address (volatile: Flash words are 16‑bit aligned)
//
static uint32_t g_flash_addr = 0xFFFFFFFFU;

//
// Function Prototypes
//
void exampleError(Fapi_StatusType status);
void Flash_initSectors(void);
static void Hardware_Init(void);
static void Timer_Init(void);
static void Timer_Start(void);
static void Timer_Stop(void);
static bool Is_TimeOut(void);
static uint8_t SCI_GetChar(void);
static void SCI_PutChar(uint8_t ch);
static void JumpToApp(void);
static void Set_Address(void);

extern uint32_t sciGetFunction(uint32_t BootMode);
extern void sciaFlush(void);

//
// Main - SCIB Bootloader
//   GPIO18(TX) / GPIO19(RX)
//   上位机每20ms发送一次 'B'，MCU收到后回显 'b' 进入bootloader模式
//   协议: 上位机发大写命令 → MCU回小写确认 → 上位机发后续数据包
//   非法命令回复 COMMAND_ERROR ('?')
//
uint32_t main(void)
{
    uint8_t command;
    bool bootloader_active = false;

    Hardware_Init();
    Timer_Init();
    Timer_Start();

    while(1)
    {
        command = SCI_GetChar();

        if(bootloader_active == false)
        {
            if(command == COMMAND_HANDSHAKE)
            {
                SCI_PutChar('b');
                Timer_Stop();
                bootloader_active = true;
            }
            continue;
        }

        switch(command)
        {
            case COMMAND_HANDSHAKE:
            {
                SCI_PutChar('b');
                break;
            }
            case COMMAND_READ_EEPROM:
            {
                SCI_PutChar('f');
                break;
            }
            case COMMAND_LOAD_EEPROM:
            {
                SCI_PutChar('e');
                break;
            }
            case COMMAND_READ_FLASH:
            {
                SCI_PutChar('n');
                break;
            }
            case COMMAND_LOAD_FLASH:
            {
                SCI_PutChar('m');
                break;
            }
            case COMMAND_SET_ADDRESS:
            {
                SCI_PutChar('d');
                Set_Address();
                break;
            }
            case COMMAND_CHANGE_BAUD:
            {
                SCI_PutChar('*');
                break;
            }
            case COMMAND_NACK:
            {
                JumpToApp();
                break;
            }
            default:
            {
                SCI_PutChar(COMMAND_ERROR);
                break;
            }
        }
    }

    return(0);
}

//
// Hardware_Init - 初始化GPIO、SCI、CPUTimer
//
static void Hardware_Init(void)
{
    Device_initGPIO();

    SysCtl_setLowSpeedClock(SYSCTL_LSPCLK_PRESCALE_4);

    DINT;
    IER = 0x0000;
    IFR = 0x0000;

    GPIO_setPinConfig(GPIO_18_SCIB_TX);
    GPIO_setDirectionMode(18, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(18, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(18, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(GPIO_19_SCIB_RX);
    GPIO_setDirectionMode(19, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(19, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(19, GPIO_QUAL_ASYNC);

    EALLOW;
    SysCtl_resetPeripheral(SYSCTL_PERIPH_RES_SCIB);
    HWREG(CPUSYS_BASE + SYSCTL_O_PERCLKCR + 2U) |= (1UL << 3U);
    EDIS;
    asm(" RPT #5 || NOP");

    SCI_resetChannels(SCIB_BASE);
    SCI_setConfig(SCIB_BASE, DEVICE_LSPCLK_FREQ, 9600,
                  SCI_CONFIG_WLEN_8 | SCI_CONFIG_STOP_ONE | SCI_CONFIG_PAR_NONE);
    SCI_enableModule(SCIB_BASE);
    SCI_performSoftwareReset(SCIB_BASE);
}

//
// Timer_Init - 初始化 CPUTimer0 为 20ms 超时定时器
//
static void Timer_Init(void)
{
    CPUTimer_setPeriod(CPUTIMER0_BASE, TIMEOUT_PERIOD);
    CPUTimer_setPreScaler(CPUTIMER0_BASE, 0);
    CPUTimer_stopTimer(CPUTIMER0_BASE);
}

//
// Timer_Start - 启动超时定时器
//
static void Timer_Start(void)
{
    CPUTimer_reloadTimerCounter(CPUTIMER0_BASE);
    CPUTimer_startTimer(CPUTIMER0_BASE);
}

//
// Timer_Stop - 停止超时定时器
//
static void Timer_Stop(void)
{
    CPUTimer_stopTimer(CPUTIMER0_BASE);
}

//
// Is_TimeOut - 检查超时定时器是否溢出
//
static bool Is_TimeOut(void)
{
    return CPUTimer_getTimerOverflowStatus(CPUTIMER0_BASE);
}

//
// SCI_GetChar - 带超时的串口接收（阻塞式，轮询）
//   返回: 收到的字节，超时则返回 COMMAND_NACK
//
static uint8_t SCI_GetChar(void)
{
    uint16_t rxData;

    while(1)
    {
        if(Is_TimeOut())
        {
            return COMMAND_NACK;
        }

        if(SCI_isDataAvailableNonFIFO(SCIB_BASE))
        {
            rxData = SCI_readCharBlockingNonFIFO(SCIB_BASE);
            return (uint8_t)rxData;
        }
    }
}

//
// SCI_PutChar - 发送一个字节
//
static void SCI_PutChar(uint8_t ch)
{
    SCI_writeCharBlockingNonFIFO(SCIB_BASE, ch);
}

//
// JumpToApp - 跳转到应用程序入口
//
static void JumpToApp(void)
{
    // uint32_t appEntry;

    // appEntry = *(uint32_t *)(FLASH_ENTRY_POINT + 2U);

    SCI_PutChar(0x00);

    // SCI_resetChannels(SCIB_BASE);
    // Timer_Stop();

    // DINT;
    // IER = 0x0000;
    // IFR = 0x0000;

    // ((void (*)(void))appEntry)();
}

//
// Set_Address - 接收2字节地址，存储为Flash绝对地址
//   上位机发地址高字节、低字节
//   0xFFFF为特殊标记（后续用于读版本信息）
//   其他值: 绝对地址 = FLASH_ENTRY_POINT + (偏移量 << 1)
//
static void Set_Address(void)
{
    uint8_t  addr_high;
    uint8_t  addr_low;
    uint16_t offset;

    addr_high = SCI_GetChar();
    addr_low  = SCI_GetChar();
    offset    = ((uint16_t)addr_high << 8U) | addr_low;

    if(offset == 0xFFFFU)
    {
        g_flash_addr = 0xFFFFFFFFU;
    }
    else
    {
        g_flash_addr = FLASH_ENTRY_POINT + ((uint32_t)offset << 1U);
    }

    SCI_PutChar(ETX);
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
void exampleError(Fapi_StatusType status)
{
    __asm("    ESTOP0");
}

//
// End of file
//