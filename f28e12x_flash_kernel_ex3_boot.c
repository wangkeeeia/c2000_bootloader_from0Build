//###########################################################################
//
// FILE:    f28e12x_flash_kernel_ex3_boot.c
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
#include "cpu1bootrom.h"
#include "flash_kernel_ex3_commands.h"
#include "flash_programming_f28e12x.h"
#include "FlashTech_F28E1xx_C28x.h"
#include "Types.h"


#include "assert.h"

//
// Globals
//

// getWordData is a pointer to the function that interfaces to the peripheral.
// Each loader assigns this pointer to it's particular getWordData function.
//
uint16fptr getWordData;

typedef struct
{
    uint16_t status;
    uint32_t address;
    uint32_t data;
    uint16_t flashAPIError;
    uint32_t flashAPIFsmStatus;
} StatusCode;
extern StatusCode statusCode;

uint16_t miniBuffer[32];

//
// Because of ECC, must be multiple of 64 bits, or 4 words, BUFFER_SIZE % 4 == 0
//
#define BUFFER_SIZE                 0x80

#define DCSM_OTP_START           0x78000
#define DCSM_OTP_END             0x78400

#define DCSM_OTP_Z1_LNKPTR_START 0x78000
#define DCSM_OTP_Z1_LNKPTR_END   0x78020

#define DCSM_OTP_Z2_LNKPTR_START 0x78200
#define DCSM_OTP_Z2_LNKPTR_END   0x78220

//
// Function Prototypes
//
extern void sciSendChecksum(void);
uint32_t getLongData(void);
void copyData(void);
void readReservedFn(void);
void DCSM_OTP_Write(uint32_t destAddr, uint16_t* miniBuffer, uint32_t WE_Protection_Mask_UO);
void exampleError();
void setFlashAPIError(Fapi_StatusType status);
void Example_Error();
void ClearFSMStatus();

//
// void CopyData(void) - This routine copies multiple blocks of data from the host
//                       to the specified RAM locations.  There is no error
//                       checking on any of the destination addresses.
//                       That is it is assumed all addresses and block size
//                       values are correct.
//
//                       Multiple blocks of data are copied until a block
//                       size of 00 00 is encountered.
//
void copyData()
{
    struct HEADER
    {
        uint16_t BlockSize;
        uint32_t DestAddr;
    } BlockHeader;

    uint16_t Buffer[BUFFER_SIZE];

    uint16_t i = 0;
    uint16_t j = 0;
    uint16_t k = 0;
    uint16_t wordData;
    uint16_t wordsWritten = 0;
    int fail = 0;

    assert(BUFFER_SIZE % 8 == 0);

    if (statusCode.status != NO_ERROR)
    {
        Example_Error();
    }

    //
    // Send checksum to satisfy before we begin
    //
#if checksum_enable
    sciSendChecksum();
#endif


    //
    // Get the size in words of the first block
    //
    BlockHeader.BlockSize = (*getWordData)();

    //
    // While the block size is > 0 copy the data
    // to the DestAddr.  There is no error checking
    // as it is assumed the DestAddr is a valid
    // memory location
    //

    EALLOW;
    while (BlockHeader.BlockSize != (uint16_t) 0x0000)
    {
        Fapi_StatusType oReturnCheck;
        Fapi_FlashStatusWordType oFlashStatusWord;
        Fapi_FlashStatusType oFlashStatus;

        // Initialize flag to keep track if writing to OTP or not
        uint16_t otpFlag = 0;
       
        BlockHeader.DestAddr = getLongData();

        // Check destination address to see if we are writing to OTP

        // Bank 0 OTP
        if ((BlockHeader.DestAddr >= OTP_MAP_BEGIN) && (BlockHeader.DestAddr <= OTP_MAP_END))
        {
            otpFlag = 1;
        }


        for (i = 0; i < BlockHeader.BlockSize; i += 0)
        {
            if (BlockHeader.BlockSize < BUFFER_SIZE)
            {
                for (j = 0; j < BlockHeader.BlockSize; j++)
                {
                    wordData = (*getWordData)();
                    Buffer[j] = wordData;
                    i++;
                }
                for (j = BlockHeader.BlockSize; j < BUFFER_SIZE; j++)
                {
                    Buffer[j] = 0xFFFF;
                }
            }
            
            //
            // BlockHeader.BlockSize >= BUFFER_SIZE
            //
            else 
            {
                //
                // less than one BUFFER_SIZE left
                //
                if ((BlockHeader.BlockSize - i) < BUFFER_SIZE) 
                {
                    //
                    // fill Buffer with rest of data
                    //
                    for (j = 0; j < BlockHeader.BlockSize - i; j++) 
                    {
                        wordData = (*getWordData)();
                        Buffer[j] = wordData;
                    }
                    
                    //
                    // increment i outside here so it doesn't affect loop above
                    //
                    i += j; 
                    
                    //
                    // fill the rest with 0xFFFF
                    //
                    for (; j < BUFFER_SIZE; j++) 
                    {
                        Buffer[j] = 0xFFFF;
                    }
                }
                else
                {
                    //
                    // fill up like normal, up to BUFFER_SIZE
                    //
                    for (j = 0; j < BUFFER_SIZE; j++) 
                    {
                        wordData = (*getWordData)();
                        Buffer[j] = wordData;
                        i++;
                    }
                }
            }

            // If writing to 512 bits of DCSM OTP containing link pointers, write 64-bits at a time
            if ((BlockHeader.DestAddr >= DCSM_OTP_Z1_LNKPTR_START) && (BlockHeader.DestAddr < DCSM_OTP_Z1_LNKPTR_END) || 
            (BlockHeader.DestAddr >= DCSM_OTP_Z2_LNKPTR_START) && (BlockHeader.DestAddr < DCSM_OTP_Z2_LNKPTR_END))
            {

                //
                // Fill miniBuffer with the data in Buffer in order to program the data
                // to flash; miniBuffer takes data from Buffer, 80 words at a time.
                //
                for (k = 0; k < (BUFFER_SIZE / 4); k++)
                {
                    uint16_t bufferOffset = k * 4;

                    miniBuffer[0] = Buffer[bufferOffset + 0];
                    miniBuffer[1] = Buffer[bufferOffset + 1];
                    miniBuffer[2] = Buffer[bufferOffset + 2];
                    miniBuffer[3] = Buffer[bufferOffset + 3];

                    //
                    // check that all the words have not already been written
                    //
                    if (wordsWritten < BlockHeader.BlockSize)
                    {

                        ClearFSMStatus();
                        // Write 64-bits to DCSM OTP
                        DCSM_OTP_Write(BlockHeader.DestAddr, miniBuffer, 0);

                    } //check if all the words are not already written
                    BlockHeader.DestAddr += 0x4;
                    wordsWritten += 0x4;
                } //for(int k); loads miniBuffer with Buffer elements
            } 
            else
            {
                for (k = 0; k < (BUFFER_SIZE/32); k++)
                {
                    uint16_t m;
                    uint16_t bufferOffset = k * 32;

                    // Load up miniBuffer
                    for (m =  0; m < 32; m++) 
                    {
                        miniBuffer[m] = Buffer[bufferOffset + m];
                    }
                
                    //
                    // check that all the words have not already been written
                    //
                    if (wordsWritten < BlockHeader.BlockSize)
                    {
                        if(fail == 0)
                        {

                            //
                            // Wait until FSM is done with the previous flash operation
                            //
                            ClearFSMStatus();

                            //
                            //program 32 words at once, 512-bits
                            //
                            //

                            // Set appropriate protection mask based on where we are writing
                            if (otpFlag)
                            {
                                
                                Fapi_setupBankSectorEnable(FLASH_WRAPPER_PROGRAM_BASE+FLASH_O_CMDWEPROT_UO, 0);
                                
                            } else 
                            {
                                //
                                // Disable erase/program protection
                                // Bits 0-11 of CMDWEPROTB is applicable for sectors 32-127, each bit represents
                                // a group of 8 sectors, e.g bit 0 represents sectors 32-39, bit 1 represents
                                // sectors 40-47, etc
                                //
                                Fapi_setupBankSectorEnable(FLASH_WRAPPER_PROGRAM_BASE+FLASH_O_CMDWEPROTA, 0);
                                Fapi_setupBankSectorEnable(FLASH_WRAPPER_PROGRAM_BASE+FLASH_O_CMDWEPROTB, 0);

                            }

                            oReturnCheck = Fapi_issueAutoEcc512ProgrammingCommand((uint32_t *) BlockHeader.DestAddr,
                                                                                  miniBuffer, sizeof(miniBuffer));
                                                                                  
                            while (Fapi_checkFsmForReady() == Fapi_Status_FsmBusy);

                            oFlashStatus = Fapi_getFsmStatus();
                            if (oReturnCheck != Fapi_Status_Success || oFlashStatus != 3)
                            {
                                statusCode.status = PROGRAM_ERROR;
                                statusCode.address = BlockHeader.DestAddr;
                                setFlashAPIError(oReturnCheck);
                                statusCode.flashAPIFsmStatus = oFlashStatus;

                                fail++;
                            }
                        
                            if (fail == 0) 
                            {
                                // Only verify if we are not writing to DCSM
                                if (!((BlockHeader.DestAddr >= DCSM_OTP_START) && (BlockHeader.DestAddr < DCSM_OTP_END))) 
                                {
                                    oReturnCheck = Fapi_doVerify((uint32_t *) (BlockHeader.DestAddr), 16, (uint32_t *) miniBuffer, &oFlashStatusWord);
                                    if (oReturnCheck != Fapi_Status_Success)
                                    {
                                        //
                                        // first fail
                                        //
                                        if (fail == 0)
                                        {
                                            statusCode.status = VERIFY_ERROR;
                                            statusCode.address = oFlashStatusWord.au32StatusWord[0];
                                            setFlashAPIError(oReturnCheck);
                                            //
                                            // FMSTAT not checked for Verify
                                            //
                                            statusCode.flashAPIFsmStatus = 0;
                                        }
                                        fail++;
                                    }
                                }
                            }

                        }


                    } //check if all the words are not already written
                    BlockHeader.DestAddr += 0x20;
                    wordsWritten += 0x20;
                }


            }
#if checksum_enable
            sciSendChecksum();
#endif
        }

        if (fail != 0) 
        {
            Example_Error();
        }
        //
        // get the size of the next block
        //
        BlockHeader.BlockSize = (*getWordData)();
        wordsWritten = 0;
    }
    EDIS;
    return;
}


// DCSM_OTP_Write is used to write 64-bits to DCSM OTP 
// This is needed because we cannot write 128 or 512 bits to the link pointers
void DCSM_OTP_Write(uint32_t destAddr, uint16_t* miniBuffer, uint32_t WE_Protection_Mask_UO)
{
    Fapi_FlashStatusType oFlashStatus;

    //
    // Error return variable
    //
    Fapi_StatusType oReturnCheck;

    //
    //program 4 words at once, 64-bits
    //
    //

    //
    // Disable erase/program protection
    Fapi_setupBankSectorEnable(FLASH_WRAPPER_PROGRAM_BASE+FLASH_O_CMDWEPROT_UO, WE_Protection_Mask_UO);

    oReturnCheck = Fapi_issueProgrammingCommand((uint32_t *) destAddr, miniBuffer,
                                                4, 0, 0, Fapi_AutoEccGeneration);

    while (Fapi_checkFsmForReady() == Fapi_Status_FsmBusy);

    oFlashStatus = Fapi_getFsmStatus();

    if (oReturnCheck != Fapi_Status_Success || oFlashStatus != 3)
    {
        Example_Error();
    }    
}

//
// uint32_t getLongData(void) - This routine fetches a 32-bit value from the peripheral
//                              input stream.
//
uint32_t getLongData()
{
    uint32_t longData;

    //
    // Fetch the upper 1/2 of the 32-bit value
    //
    longData = ((uint32_t) (*getWordData)() << 16);

    //
    // Fetch the lower 1/2 of the 32-bit value
    //
    longData |= (uint32_t) (*getWordData)();

    return longData;
}

//
//
// void readReservedFn(void) - This function reads 8 reserved words in the header.
//                             None of these reserved words are used by the
//                             this boot loader at this time, they may be used in
//                             future devices for enhancements.  Loaders that use
//                             these words use their own read function.
//
void readReservedFn()
{
    uint16_t i;

    //
    // Read and discard the 8 reserved words.
    //
    for (i = 0; i < 8; i++)
    {
        getWordData();
    }

    return;
}



//
// void setFlashAPIError(Fapi_StatusType status) - sets Flash API error from 
//                                                 operation into struct to 
//                                                 display on console
//
void setFlashAPIError(Fapi_StatusType status)
{
    if(status == Fapi_Error_AsyncIncorrectDataBufferLength)
    {
        statusCode.flashAPIError = INCORRECT_DATA_BUFFER_LENGTH;
    }
    else if(status == Fapi_Error_AsyncIncorrectEccBufferLength)
    {
        statusCode.flashAPIError = INCORRECT_ECC_BUFFER_LENGTH;
    }
    else if(status == Fapi_Error_AsyncDataEccBufferLengthMismatch)
    {
        statusCode.flashAPIError = DATA_ECC_BUFFER_LENGTH_MISMATCH;
    }
    else if(status == Fapi_Error_FlashRegsNotWritable)
    {
        statusCode.flashAPIError = FLASH_REGS_NOT_WRITABLE;
    }
    else if(status == Fapi_Error_FeatureNotAvailable)
    {
        statusCode.flashAPIError = FEATURE_NOT_AVAILABLE;
    }
    else if(status == Fapi_Error_InvalidAddress)
    {
        statusCode.flashAPIError = INVALID_ADDRESS;
    }
    else if(status == Fapi_Error_Fail)
    {
        statusCode.flashAPIError = FAILURE;
    }
    else
    {
        statusCode.status = NOT_RECOGNIZED;
    }
}

void ClearFSMStatus()
{

    Fapi_StatusType oReturnCheck;
    Fapi_FlashStatusType oFlashStatus;
        
    while (Fapi_checkFsmForReady() != Fapi_Status_FsmReady){}
    oFlashStatus = Fapi_getFsmStatus();
    if(oFlashStatus != 0)
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
            Example_Error();
        }
    }
}

void Example_Error()
{
    asm(" ESTOP0");
}
