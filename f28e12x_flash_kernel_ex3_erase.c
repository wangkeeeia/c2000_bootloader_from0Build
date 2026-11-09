//###########################################################################
//
// FILE:    f28e12x_flash_kernel_ex3_erase.c
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
// Includes
//
#include "f28e12x_flash_kernel_ex3_erase.h"
#include "FlashTech_F28E1xx_C28x.h"

//
// Globals
//
typedef struct
{
    uint16_t status;
    uint32_t address;
    uint32_t data;
    uint16_t flashAPIError;
    uint32_t flashAPIFsmStatus;
} StatusCode;
extern StatusCode statusCode;

const uint32_t sectAddress[128] = {Bzero_Sector0_start, Bzero_Sector1_start,
                                  Bzero_Sector2_start, Bzero_Sector3_start,
                                  Bzero_Sector4_start, Bzero_Sector5_start,
                                  Bzero_Sector6_start, Bzero_Sector7_start,
                                  Bzero_Sector8_start, Bzero_Sector9_start,
                                  Bzero_Sector10_start, Bzero_Sector11_start,
                                  Bzero_Sector12_start, Bzero_Sector13_start,
                                  Bzero_Sector14_start, Bzero_Sector15_start,
                                  Bzero_Sector16_start, Bzero_Sector17_start,
                                  Bzero_Sector18_start, Bzero_Sector19_start,
                                  Bzero_Sector20_start, Bzero_Sector21_start,
                                  Bzero_Sector22_start, Bzero_Sector23_start,
                                  Bzero_Sector24_start, Bzero_Sector25_start,
                                  Bzero_Sector26_start, Bzero_Sector27_start,
                                  Bzero_Sector28_start, Bzero_Sector29_start,
                                  Bzero_Sector30_start, Bzero_Sector31_start,
                                  Bzero_Sector32_start, Bzero_Sector33_start,
                                  Bzero_Sector34_start, Bzero_Sector35_start,
                                  Bzero_Sector36_start, Bzero_Sector37_start,
                                  Bzero_Sector38_start, Bzero_Sector39_start,
                                  Bzero_Sector40_start, Bzero_Sector41_start,
                                  Bzero_Sector42_start, Bzero_Sector43_start,
                                  Bzero_Sector44_start, Bzero_Sector45_start,
                                  Bzero_Sector46_start, Bzero_Sector47_start,
                                  Bzero_Sector48_start, Bzero_Sector49_start,
                                  Bzero_Sector50_start, Bzero_Sector51_start,
                                  Bzero_Sector52_start, Bzero_Sector53_start,
                                  Bzero_Sector54_start, Bzero_Sector55_start,
                                  Bzero_Sector56_start, Bzero_Sector57_start,
                                  Bzero_Sector58_start, Bzero_Sector59_start,
                                  Bzero_Sector60_start, Bzero_Sector61_start,
                                  Bzero_Sector62_start, Bzero_Sector63_start};

const uint16_t sectSize = Sector2KB_u32length;

//
// Function Prototypes
//
void sharedErase(uint32_t sectors1, uint32_t sectors2, uint32_t bank);
void Clear_FSMStatus(void);
void Erase_Error();

//
// void sharedErase(uint32_t sectors,
//                  uint32_t bank) -    This routine takes the 32-bit sectors
//                                      variable and bank variable as parameters.  
//                                      Each bit corresponds to a sector, starting 
//                                      with bit 0 and sector 0.  This routine
//                                      attempts to erase the sectors specified 
//                                      within the bank specified.
//
void sharedErase(uint32_t sectors1, uint32_t sectors2, uint32_t bank)
{
    statusCode.status = NO_ERROR;
    statusCode.address = 0x12345678;
    int i = 0;
    Fapi_StatusType oReturnCheck;
    Fapi_FlashStatusWordType oFlashStatusWord;
    Fapi_FlashStatusType oFlashStatus;
    int fail = 0;

    EALLOW;

    //
    // Set Bank Erase mask by taking the bitwise NOT of the sectors passed in
    // The sectors masked will not be erased
    //
    uint32_t mask1 = ~sectors1;
    uint32_t mask2 = ~sectors2;

    //
    // Wait until FSM is done with the previous flash operation
    //
    while (Fapi_checkFsmForReady() != Fapi_Status_FsmReady){}
    oFlashStatus = Fapi_getFsmStatus();
    if(oFlashStatus != 3)
    {

        /* Clear the Status register */
        oReturnCheck = Fapi_issueAsyncCommand(Fapi_ClearStatus);
        //
        // Wait until status is cleared
        //
        while (Fapi_getFsmStatus() != 0) {}

        if(oReturnCheck != Fapi_Status_Success)
        {
            statusCode.status = CLEAR_STATUS_ERROR;
            statusCode.address = 0;
            setFlashAPIError(oReturnCheck);
            statusCode.flashAPIFsmStatus = 0; // not used here
            fail++;
        }
    }
    // Enable erase/program protection for the sectors that do not need to be erased
    // CMDWEPROTA is applicable for sectors 0-31
    // Bits 0-11 of CMDWEPROTB is applicable for sectors 32-127, each bit represents
    // a group of 8 sectors, e.g bit 0 represents sectors 32-39, bit 1 represents
    // sectors 40-47, etc
    //
    Fapi_setupBankSectorEnable(FLASH_WRAPPER_PROGRAM_BASE+FLASH_O_CMDWEPROTA, mask1);
    Fapi_setupBankSectorEnable(FLASH_WRAPPER_PROGRAM_BASE+FLASH_O_CMDWEPROTB, mask2);

    oReturnCheck = Fapi_issueBankEraseCommand((uint32 *)Bzero_Sector0_start);

    // Wait until FSM is done with erase bank operation
    while (Fapi_checkFsmForReady() != Fapi_Status_FsmReady)
    {
    }

    oFlashStatus = Fapi_getFsmStatus();

    if(oReturnCheck != Fapi_Status_Success || oFlashStatus != 3)
    {
        //
        // first fail
        //
            statusCode.status = BANK_ERASE_ERROR;
            //
            // Address is not needed for Bank Erase Error
            //
            statusCode.address = 0;
            setFlashAPIError(oReturnCheck);
            statusCode.flashAPIFsmStatus = oFlashStatus;
            fail = 1;
    }

    for (i = 0; i < 32; i++)
    {
        if ((sectors1 & 0x00000001) == 0x00000001 && fail == 0)
        {

            //
            // All 8K sector size
            //
            oReturnCheck = Fapi_doBlankCheck((uint32_t *) (sectAddress[i]),
                                             sectSize,
                                             &oFlashStatusWord);

            if (oReturnCheck != Fapi_Status_Success)
            {

                statusCode.status = BLANK_ERROR;
                statusCode.address = oFlashStatusWord.au32StatusWord[0];
                setFlashAPIError(oReturnCheck);
                statusCode.flashAPIFsmStatus = 0;

                fail++;
            }
        }
        sectors1 = sectors1 >> 1;
    }

    for (i = 32; i < 64; i+=8)
        {
            if ((sectors2 & 0x00000001) == 0x00000001 && fail == 0)
            {

                //
                // All 8K sector size
                //
                oReturnCheck = Fapi_doBlankCheck((uint32_t *) (sectAddress[i]),
                                                 sectSize*8,
                                                 &oFlashStatusWord);

                if (oReturnCheck != Fapi_Status_Success)
                {

                    statusCode.status = BLANK_ERROR;
                    statusCode.address = oFlashStatusWord.au32StatusWord[0];
                    setFlashAPIError(oReturnCheck);
                    statusCode.flashAPIFsmStatus = 0;

                    fail++;
                }
            }
            sectors2 = sectors2 >> 1;
        }

    EDIS;
    return;
}

void Clear_FSMStatus(void){
    Fapi_FlashStatusType  oFlashStatus;
    Fapi_StatusType  oReturnCheck;

    //
    // Wait until FSM is done with the previous flash operation
    //
    while (Fapi_checkFsmForReady() != Fapi_Status_FsmReady){}
    oFlashStatus = Fapi_getFsmStatus();
    if(oFlashStatus != 3)
    {

        /* Clear the Status register */
        oReturnCheck = Fapi_issueAsyncCommand(Fapi_ClearStatus);
        //
        // Wait until status is cleared
        //
        while (Fapi_getFsmStatus() != 0) {}

        if(oReturnCheck != Fapi_Status_Success)
        {
            statusCode.status = CLEAR_STATUS_ERROR;
            statusCode.address = 0;
            setFlashAPIError(oReturnCheck);
            statusCode.flashAPIFsmStatus = 0; // not used here
            Erase_Error();
        }
    }
}


void Erase_Error() 
{
    asm(" ESTOP0");
}

//
// End of file
//
