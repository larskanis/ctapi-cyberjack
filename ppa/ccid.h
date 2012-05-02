#ifndef CCID_H
#define CCID_H

#include <inttypes.h>

#define CCID_VOLTAGE_18         3
#define CCID_VOLTAGE_30         2
#define CCID_VOLTAGE_50         1
#define CCID_VOLTAGE_AUTO       0

#define PC_TO_RDR_ICCPOWERON		0x62
#define PC_TO_RDR_ICCPOWEROFF		0x63
#define PC_TO_RDR_GETSLOTSTATUS		0x65
#define PC_TO_RDR_XFRBLOCK  		0x6F
#define PC_TO_RDR_GETPARAMETERS		0x6C
#define PC_TO_RDR_RESETPARAMETERS	0x6D
#define PC_TO_RDR_SETPARAMETERS		0x61
#define PC_TO_RDR_ESCAPE    		0x6B
#define PC_TO_RDR_ICCCLOCK    		0x6E
#define PC_TO_RDR_TAPDU             0x6A
#define PC_TO_RDR_SECURE            0x69
#define PC_TO_RDR_MECHANICAL        0x71
#define PC_TO_RDR_ABORT             0x72
#define PC_TO_RDR_SETDATARATEANDCLOCKFREQUENCY    0x73

#define RDR_TO_PC_DATABLOCK			0x80
#define RDR_TO_PC_SLOTSTATUS		0x81
#define RDR_TO_PC_PARAMETERS		0x82
#define RDR_TO_PC_ESCAPE			0x83
#define RDR_TO_PC_DATARATEANDCLOCKFREQUENCY			0x84
#define RDR_TO_PC_STATUSCLOCK       0x85
#define RDR_TO_PC_NOTIFYSLOTCHANGE  0x50 
#define RDR_TO_PC_HARWAREERROR      0x51 
#define RDR_TO_PC_KEYEVENT          0x40

#define CCID_ESCAPE_INPUT           0x00
#define CCID_ESCAPE_UPDATE          0x01
#define CCID_ESCAPE_VERIFY          0x02
#define CCID_ESCAPE_STATUS          0x03
#define CCID_ESCAPE_UPDATE_START    0x04
#define CCID_ESCAPE_GET_INFO        0x05
#define CCID_ESCAPE_SET_DATE_TIME   0x06
#define CCID_ESCAPE_SET_SERNUMBER   0x07
#define CCID_ESCAPE_GET_KEY_VERSION 0x08
#define CCID_ESCAPE_VERIFY_KEY      0x09 
#define CCID_ESCAPE_DIRECT_TO_RAM   0x0a 
#define CCID_ESCAPE_EXEC_RAM        0x0b
#define CCID_ESCAPE_GET_STACKSIGNCOUNTER 0x0c
 


#define CMD_ABORTED 				0xff
#define ICC_MUTE 					0xfe
#define XFR_PARITY_ERROR            0xfd
#define XFR_OVERRUN					0xfc
#define HW_ERROR					0xfb

#define BAD_ATR_TS					0xf8
#define BAD_ATR_TCK					0xf7
#define ICC_PROTOCOL_NOT_SUPPORTED  0xf6
#define ICC_CLASS_NOT_SUPPORTED     0xf5
#define PROCEDURE_BYTE_CONFLICT     0xf4
#define DEACTIVATED_PROTOCOL        0xf3
#define BUSY_WITH_AUTOSEQUENCE      0xf2

#define PIN_TIMEOUT					0xf0
#define PIN_CANCELED				0xef
#define PIN_DIFFERENT				0xee

#define CMD_SLOT_BUSY               0xe0


#ifdef __GNUC__
#define __CCID_PACKED __attribute__ ((packed))
#else
#define __CCID_PACKED
#pragma pack(push,savepack,1)
#endif


typedef struct
{
   unsigned char bMessageType;
   uint32_t dwLength;
   unsigned char bSlot;
   unsigned char bSeq;
   union
   {
      unsigned char abRFU[3];
      struct
      {
         unsigned char bPowerSelect;
         unsigned char abRFU[2];
      }__CCID_PACKED iccPowerOn;
      struct
      {
         unsigned char bBWI;
         unsigned short wLevelParameter;
      }__CCID_PACKED XfrBlock; 
      struct
      {
         unsigned char bProtocolNum;
         unsigned char abRFU[2];
      }__CCID_PACKED SetParameters;
      struct
      {
         unsigned char bClockCommand;
         unsigned char abRFU[2];
      }__CCID_PACKED iccClock; 
      struct
      {
         unsigned char bChanges;
         unsigned char bClassGetResponse;
         unsigned char bClassEnvelope;
      }__CCID_PACKED T0APDU; 
      struct
      {
         unsigned char bBWI;
         unsigned short wLevelParameter;
      }__CCID_PACKED Secure; 
      struct
      {
         unsigned char bFunction;
         unsigned char abRFU[2];
      }__CCID_PACKED Mechanical; 
   }__CCID_PACKED Header;
   union
   {
      unsigned char abData[260];
      union
      { 
         struct
         {
            unsigned char bmFindexDindex;
            unsigned char bmTCCKST0;
            unsigned char GuardTimeT0;
            unsigned char bWaitingIntegerT0;
            unsigned char bClockStop;
         }__CCID_PACKED T0;
         struct
         {
            unsigned char bmFindexDindex;
            unsigned char bmTCCKST1;
            unsigned char GuardTimeT1;
            unsigned char bWaitingIntegerT1;
            unsigned char bClockStop;
            unsigned char bIFSC;
            unsigned char bNadValue;
         }__CCID_PACKED T1;
      }__CCID_PACKED SetParameters;
      struct
      {
         unsigned char bPINOperation;
         unsigned char bTimeOut;
         unsigned char bmFormatString;
         unsigned char bmPINBlockString;
         unsigned char bmPINLengthFormat;
         union
         {
            struct
            {
               unsigned short wPINMaxExtraDigit;
               unsigned char bEntryValidationCondition;
               unsigned char bNumberMessage;
               unsigned short wLangId;
               unsigned char bMsgIndex;
               unsigned char bTeoPrologue[3];
               unsigned char abData[49];
            }__CCID_PACKED Verify;
            struct
            {
               unsigned char bInsertionOffsetOld;
               unsigned char bInsertionOffsetNew;
               unsigned short wPINMaxExtraDigit;
               unsigned char bConfirmPIN;
               unsigned char bEntryValidationCondition;
               unsigned char bNumberMessage;
               unsigned short wLangId;
               unsigned char bMsgIndex1;
               unsigned char bMsgIndex2;
               unsigned char bMsgIndex3;
               unsigned char bTeoPrologue[3];
               unsigned char abData[34];
            }__CCID_PACKED Modify;
            struct
            {
               unsigned char bTeoPrologue[3];
            }__CCID_PACKED Next;
         }__CCID_PACKED Data;
      }__CCID_PACKED Secure; 
      struct
      {
         uint32_t dwClockFrequency;
         uint32_t dwDataRate;
      }__CCID_PACKED SetDataRateAndClockFrequency; 
      struct
      {
	    unsigned char bFunction;
		 union
		 {
		   struct
			{
			   unsigned short wOffset;
			   unsigned char bLength;
			   unsigned char Data[128];
			}__CCID_PACKED Update;
		   struct
			{
			   unsigned char bOffset;
			   unsigned char Date[11];
			   unsigned char Time[6];
			}__CCID_PACKED SetDateTime;
		   struct
			{
			   unsigned char SerNumber[20];
			}__CCID_PACKED SetSerNumber;
		   struct
			{
			   unsigned char Timeout;
			}__CCID_PACKED Input;
	      struct
			{
            unsigned char Key[128];
			}__CCID_PACKED UpdateKey;
			struct
			{
				unsigned short wOffset;
				unsigned char bCode[128];
			}__CCID_PACKED ToRam;
			struct
			{
				unsigned char bIntTable[128];
			}__CCID_PACKED ExecRam;

		 }__CCID_PACKED Data;
      }__CCID_PACKED Escape;
   }__CCID_PACKED Data;
}__CCID_PACKED CCID_Message;

typedef struct
{
   unsigned char bMessageType;
   uint32_t dwLength;
   unsigned char bSlot;
   unsigned char bSeq;
   unsigned char bStatus;
   unsigned char bError;
   union
   {
      unsigned char bRFU;
      unsigned char bChainParameter;
      unsigned char bClockStatus;
      unsigned char bProtocolNum;
      
   }__CCID_PACKED Header;
   union
   {
      unsigned char abData[260];
      union
      { 
         struct
         {
            unsigned char bmFindexDindex;
            unsigned char bmTCCKST0;
            unsigned char GuardTimeT0;
            unsigned char bWaitingIntegerT0;
            unsigned char bClockStop;
         }__CCID_PACKED T0;
         struct
         {
            unsigned char bmFindexDindex;
            unsigned char bmTCCKST1;
            unsigned char GuardTimeT1;
            unsigned char bWaitingIntegerT1;
            unsigned char bClockStop;
            unsigned char bIFSC;
            unsigned char bNadValue;
         }__CCID_PACKED T1;
      }__CCID_PACKED Parameters;
      struct
      {
         uint32_t dwClockFrequency;
         uint32_t dwDataRate;
      }__CCID_PACKED DataRateAndClockFrequency; 
      struct
      {
         union
         {
            unsigned short StackSignCounter;
         }__CCID_PACKED Function;
      }__CCID_PACKED Escape; 
   }__CCID_PACKED Data;
}__CCID_PACKED CCID_Response;
   	
#define RDR_TO_PC_NOTIFYSLOTCHANGE 		0x50
#define RDR_TO_PC_HARDWAREERROR 		0x51
#define RDR_TO_PC_KEYEVENT      		0x40

typedef struct
{
   unsigned char bMessageType;
   union
   {
      struct
      {
         unsigned char bmSlotICCState;
      }__CCID_PACKED NotifySlotChange;
      struct
      {
         unsigned char bSlot;
         unsigned char bSeq;
         unsigned char bHardwareErrorCode;
      }__CCID_PACKED HardwareError;
      struct
      {
         unsigned char KeyStatus;
      }__CCID_PACKED KeyEvent;

   }__CCID_PACKED Data;
}__CCID_PACKED CCID_Interrupt;

typedef union
{
   CCID_Message Message;
   CCID_Response Response;
   CCID_Interrupt Interrupt;
}__CCID_PACKED CCID_transfer;

#ifndef __GNUC__
#pragma pack(pop,savepack)
#endif

#endif

