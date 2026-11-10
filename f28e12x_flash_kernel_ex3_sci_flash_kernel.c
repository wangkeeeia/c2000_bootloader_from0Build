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

#define VERSION_STR                 "F28E12_BL_V1.0"
#define MODULE_ID_STR               "F28E12X"
#define RELEASE_DATE_STR            "20261109"
#define VERSION_STR_LEN             14U
#define MODULE_ID_LEN                7U
#define RELEASE_DATE_LEN             8U
#define VERSION_INFO_TOTAL          (VERSION_STR_LEN + MODULE_ID_LEN + RELEASE_DATE_LEN)

#define FLASH_PAGE_SIZE             128U

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
static void Clock_Init(void);
static void Hardware_Init(void);
static void Timer_Init(void);
static void Timer_Start(void);
static void Timer_Stop(void);
static bool Is_TimeOut(void);
static uint8_t SCI_GetChar(void);
static void SCI_PutChar(uint8_t ch);
static void SCI_PutString(const char *str, uint16_t len);
static void JumpToApp(void);
static void Set_Address(void);
static void Read_Version_Info(void);
static void Read_Flash_Page(void);
static void Load_Flash_Page(void);

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
            else
            {
                Timer_Start();
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
                if(g_flash_addr == 0xFFFFFFFFU)
                {
                    Read_Version_Info();
                }
                else
                {
                    Read_Flash_Page();
                }
                break;
            }
            case COMMAND_LOAD_FLASH:
            {
                Load_Flash_Page();
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
// Clock_Init - 手动配置系统时钟到 160MHz，跳过 DCC 校验
//   SysCtl_setClock() 内部会用 DCC 校验 PLL 频率，其等待 Done/Error 标志
//   的循环没有超时保护。Flash 配置区被擦除后，DCC 永远等不到标志位，程序
//   会永久卡死在该循环里（表现为运行到 SysCtl_setClock 后无任何反应）。
//   这里复制 SysCtl_setClock() 的寄存器操作，但完全跳过 DCC，并用带超时的
//   PLL lock 等待，避免死循环。
//   时钟源: 内部振荡器 SYSOSCDIV4 (8MHz)
//   最终:   PLLSYSCLK = 8MHz * 40 / (1 * 2 * 1) = 160MHz
//
static void Clock_Init(void)
{
    uint32_t pllMult;
    uint32_t timeout;
    uint16_t pllLockStatus;

    //
    // QDIV(40) | PDIV(1) | RDIVCLK0(2) 组合成 SYSPLLMULT 寄存器值
    // 与 SysCtl_setClock() 内部的计算完全一致
    //
    pllMult = (uint32_t)SYSCTL_PLLMULT_40 |
              ((uint32_t)SYSCTL_PDIV_1 << SYSCTL_SYSPLLMULT_PDIV_S) |
              ((uint32_t)SYSCTL_RDIVCLK0_2 << SYSCTL_SYSPLLMULT_RDIVCLK0_S);

    //
    // 0. 先配置 Flash wait state（160MHz 需要 3 个 wait state）
    //    Flash 配置区被擦除后，boot ROM 无法读取正确的 Flash 参数，
    //    这里手动设置 FRDCNTL.RWAIT。当前系统时钟还是低速，写该寄存器安全。
    //
    EALLOW;
    HWREG(FLASH0CTRL_BASE + FLASH_O_FRDCNTL) =
        (HWREG(FLASH0CTRL_BASE + FLASH_O_FRDCNTL) &
         ~(uint32_t)FLASH_FRDCNTL_RWAIT_M) |
        ((uint32_t)DEVICE_FLASH_WAITSTATES << FLASH_FRDCNTL_RWAIT_S);
    EDIS;

    //
    // 1. 旁路 PLL（PLL 输出暂时不接入系统时钟路径）
    //
    EALLOW;
    HWREGH(CLKCFG_BASE + SYSCTL_O_SYSPLLCTL) &= ~SYSCTL_SYSPLLCTL_PLLCLKEN;
    EDIS;
    SysCtl_delay(23U);

    //
    // 2. 切换到内部振荡器 SYSOSCDIV4
    //
    SysCtl_selectOscSource(SYSCTL_OSCSRC_SYSOSCDIV4);
    SysCtl_delay(11U);

    //
    // 3. 系统分频先设为 /1，保证 PLL 配置阶段最快
    //
    SysCtl_setPLLSysClk(0U);

    //
    // 4. 关闭 PLL，准备重新配置
    //
    EALLOW;
    HWREGH(CLKCFG_BASE + SYSCTL_O_SYSPLLCTL) &= ~SYSCTL_SYSPLLCTL_PLLEN;
    EDIS;
    SysCtl_delay(12U);

    //
    // 5. 写 multiplier 并重新打开 PLL
    //
    EALLOW;
    HWREG(CLKCFG_BASE + SYSCTL_O_SYSPLLMULT) = pllMult;
    HWREGH(CLKCFG_BASE + SYSCTL_O_SYSPLLCTL) |= SYSCTL_SYSPLLCTL_PLLEN;
    EDIS;

    //
    // 6. 等待 PLL 锁定（带超时，避免死循环）
    //    lock 时间约 1024 * OSCCLK * PDIV，8MHz 下约 128us
    //
    timeout = 4096U;
    pllLockStatus = HWREGH(CLKCFG_BASE + SYSCTL_O_SYSPLLSTS) &
                    SYSCTL_SYSPLLSTS_LOCKS;
    while((pllLockStatus != SYSCTL_SYSPLLSTS_LOCKS) && (timeout != 0U))
    {
        pllLockStatus = HWREGH(CLKCFG_BASE + SYSCTL_O_SYSPLLSTS) &
                        SYSCTL_SYSPLLSTS_LOCKS;
        timeout--;
    }

    //
    // 7. 先给一个较慢的分频（/3），限制电流上升
    //
    SysCtl_setPLLSysClk((uint16_t)SYSCTL_SYSDIV_1 + 2U);

    //
    // 8. 把 PLL 接入系统时钟路径
    //
    EALLOW;
    HWREGH(CLKCFG_BASE + SYSCTL_O_SYSPLLCTL) |= SYSCTL_SYSPLLCTL_PLLCLKEN;
    EDIS;

    //
    // 9. 等待电压调节器稳定
    //
    SysCtl_delay(40U);

    //
    // 10. 最终分频 /1 → 160MHz
    //
    SysCtl_setPLLSysClk((uint16_t)SYSCTL_SYSDIV_1);
}

//
// Hardware_Init - 初始化GPIO、SCI、CPUTimer
//
static void Hardware_Init(void)
{
    //
    // Flash 配置区被擦除后，boot ROM 无法正确初始化时钟。
    // 这里手动配置时钟：用内部振荡器 SYSOSCDIV4 (8MHz) + PLL 到 160MHz。
    // 不调用 Device_init()：它使用外部晶振 XTAL，而 XTAL 起振失败会
    // 卡死在 SysCtl_selectXTAL() 的 ESTOP0 循环，导致程序跑飞。
    //
    SysCtl_disableWatchdog();
    SysCtl_resetMCD();

    Clock_Init();

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
    HWREGH(CPUTIMER0_BASE + CPUTIMER_O_TCR) |= CPUTIMER_TCR_TIF;
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
// SCI_PutString - 发送定长字符串（不含'\0'）
//
static void SCI_PutString(const char *str, uint16_t len)
{
    uint16_t i;
    for(i = 0U; i < len; i++)
    {
        SCI_PutChar((uint8_t)str[i]);
    }
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
// Read_Version_Info - 发送版本信息
//   协议: 总长度(2B) + 版本字符串 + 模块ID + 发布日期 + ETX
//
static void Read_Version_Info(void)
{
    uint16_t total;

    total = VERSION_INFO_TOTAL;

    SCI_PutChar((uint8_t)(total >> 8U));
    SCI_PutChar((uint8_t)(total & 0xFFU));

    SCI_PutString(VERSION_STR, VERSION_STR_LEN);
    SCI_PutString(MODULE_ID_STR, MODULE_ID_LEN);
    SCI_PutString(RELEASE_DATE_STR, RELEASE_DATE_LEN);

    SCI_PutChar(ETX);
}

//
// Read_Flash_Page - 从 g_flash_addr 读取 FLASH_PAGE_SIZE 字节
//   F28E12 Flash 是内存映射的，直接指针读取即可
//
static void Read_Flash_Page(void)
{
    uint16_t i;
    const uint8_t *data;

    SCI_PutChar((uint8_t)(FLASH_PAGE_SIZE >> 8U));
    SCI_PutChar((uint8_t)(FLASH_PAGE_SIZE & 0xFFU));

    data = (const uint8_t *)g_flash_addr;

    for(i = 0U; i < FLASH_PAGE_SIZE; i++)
    {
        SCI_PutChar(data[i]);
    }

    SCI_PutChar(ETX);
}

//
// Load_Flash_Page - 写 Flash 数据协议（遵循 CMS32 协议）:
//   1. 收 2 字节数据长度 + N 字节数据
//   2. 擦除 Flash Bank
//   3. 编程 Flash（512-bit chunk）
//   4. 回校验回传: 'm' + 00 + 02 + checksum_high + checksum_low + ETX
//
static void Load_Flash_Page(void)
{
    uint16_t byte_count;
    uint16_t ram_checksum = 0U;
    uint16_t i;
    uint16_t data_buf[FLASH_PAGE_SIZE / 2];     /* 64 words = 128 bytes = 2x 512-bit */
    uint8_t  *byte_ptr = (uint8_t *)data_buf;
    Fapi_StatusType status;
    Fapi_FlashStatusType flashStatus;

    /* ----- 接收数据长度 (2 bytes: high, low) ----- */
    byte_count  = ((uint16_t)SCI_GetChar() << 8);
    byte_count |= SCI_GetChar();

    if(byte_count > FLASH_PAGE_SIZE)
    {
        byte_count = FLASH_PAGE_SIZE;
    }

    /* ----- 接收数据，计算 RAM 校验和 ----- */
    for(i = 0U; i < byte_count; i++)
    {
        byte_ptr[i] = SCI_GetChar();
        ram_checksum += byte_ptr[i];
    }

    /* 剩余填 0xFF */
    for(; i < FLASH_PAGE_SIZE; i++)
    {
        byte_ptr[i] = 0xFFU;
    }

    /* ----- 初始化 Flash API (仅首次) ----- */
    Flash_initSectors();

    /* ----- 关闭所有扇区写保护 ----- */
    Fapi_setupBankSectorEnable(FLASH_WRAPPER_PROGRAM_BASE + FLASH_O_CMDWEPROTA, 0);
    Fapi_setupBankSectorEnable(FLASH_WRAPPER_PROGRAM_BASE + FLASH_O_CMDWEPROTB, 0);

    /* ----- 擦除: 暂时跳过 (Flash 已为 FF 擦除态) -----
     * 注意: 后续需改为按扇区擦除，保护 sector0 配置区，
     *       不要用 mask=0 全片擦除，否则会破坏时钟配置区。
     */

    /* ----- 编程 Flash：128 字节分 2 次 512-bit 写入 ----- */
    if(byte_count > 0U)
    {
        /* 第1块: 前 32 words (64 bytes) */
        status = Fapi_issueAutoEcc512ProgrammingCommand(
                    (uint32_t *)g_flash_addr,
                    data_buf,
                    32
                 );

        while(Fapi_checkFsmForReady() != Fapi_Status_FsmReady)
        {
        }

        flashStatus = Fapi_getFsmStatus();
        if(status != Fapi_Status_Success || flashStatus != 3)
        {
            SCI_PutChar(COMMAND_ERROR);
            return;
        }

        /* 第2块: 后 32 words (64 bytes) */
        status = Fapi_issueAutoEcc512ProgrammingCommand(
                    (uint32_t *)(g_flash_addr + 64U),
                    &data_buf[32],
                    32
                 );

        while(Fapi_checkFsmForReady() != Fapi_Status_FsmReady)
        {
        }

        flashStatus = Fapi_getFsmStatus();
        if(status != Fapi_Status_Success || flashStatus != 3)
        {
            SCI_PutChar(COMMAND_ERROR);
            return;
        }
    }

    /* ----- 响应: 'm' + 长度(2B) + 校验和(2B) + ETX ----- */
    SCI_PutChar('m');
    SCI_PutChar(0);
    SCI_PutChar(2);
    SCI_PutChar((uint8_t)(ram_checksum >> 8));
    SCI_PutChar((uint8_t)(ram_checksum & 0xFF));
    SCI_PutChar(ETX);
}

//
// Flash_initSectors - Initialize flash API and active flash bank sectors
//
void Flash_initSectors(void)
{
    static bool initialized = false;

    if(initialized)
    {
        return;
    }
    initialized = true;

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