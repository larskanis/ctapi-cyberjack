#include "Platform.h"

#include "cjppa.h"
#include "cjpp.h"

#include <time.h>
#include <stdio.h>
#include <string.h>

#ifdef OS_LINUX
#include <malloc.h>
#include "cjppa_linux.h"
#endif

static void ClearFromArray(cjccidHANDLE hndl);
static int AddToArray(cjccidHANDLE hndl);
static int IsInArray(cjccidHANDLE hndl);

extern unsigned char easy_dat[32768];

static void SendKeyEvent(cjccidHANDLE hDevice,CCID_Interrupt* Intr)
{
    if(hDevice->backs.CallbackKey!=NULL)
	{
#ifdef _WINDOWS
       EnterCriticalSection(&(hDevice->backs.CritSectKeyEvent));
	   (*(hDevice->backs.CallbackKey))(hDevice->backs.hClass,Intr->Data.KeyEvent.KeyStatus);
	   if(Intr->Data.KeyEvent.KeyStatus==0x8a)
	   {
          hDevice->backs.state=COUNT_DIALOG;
	   }
	   else if(Intr->Data.KeyEvent.KeyStatus==0x8b && hDevice->backs.state==COUNT_DIALOG)
	   {
	   	  (*(hDevice->backs.CallbackKey))(hDevice->backs.hClass,0x8c);
          hDevice->backs.state=PIN_DIALOG;
	   }
       LeaveCriticalSection(&(hDevice->backs.CritSectKeyEvent));
#else
	   (*(hDevice->backs.CallbackKey))(hDevice->backs.hClass,Intr->Data.KeyEvent.KeyStatus);
#endif
	}
	if(Intr->Data.KeyEvent.KeyStatus==1)
		hDevice->hasCanceled=1;
}

void HandleCyberJackInterruptData(cjccidHANDLE hDevice, CCID_Interrupt* Intr)
{
	if(!hDevice->backs.Fini)
	{
		switch(Intr->bMessageType)
		{
		case 0x40:
			SendKeyEvent(hDevice,Intr);
			break;
		case 0x50:
			if(hDevice->backs.CallbackStatus!=NULL)
			{
				(*(hDevice->backs.CallbackStatus))(hDevice->backs.hClass,Intr->Data.NotifySlotChange.bmSlotICCState);
			}
			hDevice->Status=(Intr->Data.NotifySlotChange.bmSlotICCState & 1);
			break;
		default:;
		}
	}
}



CJPP_EXP_TYPE CCID_DEVICE CJPP_CALL_CONV ctapiInit(const char *cDeviceName,CCID_CTX hClass,void (CJPP_CALLBACK_TYPE *CallbackStatus)(CCID_CTX,unsigned char),void (CJPP_CALLBACK_TYPE *CallbackKey)(CCID_CTX,unsigned char status))
{
   unsigned long LocalInfo;
   cjccidHANDLE Result;
#ifdef _WINDOWS
   char *myName;
#endif // _WINDOWS
   HANDLE hDevice;
//	MessageBox(NULL,"test","test",MB_OK);
#ifdef _WINDOWS
   cjppDebugOut(NULL,"Init: Enter",0,0);
#endif
   hDevice=cjppCreate(cDeviceName);
   if(hDevice==0)
      return 0;
   Result=malloc(sizeof(cjccidStruct));
   memset(Result,0,sizeof(cjccidStruct));
   Result->cjppStr.hDevice=hDevice;
	Result->backs.CallbackStatus=CallbackStatus;
   Result->backs.CallbackKey=CallbackKey;
   Result->backs.hClass=hClass;
   Result->backs.Fini=0;
   Result->hasCanceled=0;
   Result->reader_path_len=2;
	Result->cjppStr.Fini=&(Result->backs.Fini);
	Result->cjppStr.ReaderName=strdup(cDeviceName);
	Result->cjppStr.Owner=Result;
#ifdef _WINDOWS
   myName=strdup(cDeviceName);
   strupr(myName);
   if(strstr(myName,"COM")!=NULL)
   {
      Result->cjppStr.is_usb=0;
      Result->cjppStr.reader_id=0;
   }
   else
   {
      Result->cjppStr.is_usb=1;
#ifdef _WINDOWS
      if(strstr(cDeviceName,"rsct_usb"))
         Result->cjppStr.reader_id=atoi(cDeviceName+strlen(cDeviceName)-11);
      else
         Result->cjppStr.reader_id=atoi(cDeviceName+strlen(cDeviceName)-2);
#else
      Result->cjppStr.reader_id=0;
#endif
   }
   free(myName);
#endif /* _WINDOWS */

#ifdef _WINDOWS
   cjppDebugOut(Result,"Init: Object created",0,0);
#endif
   memcpy(Result->reader_path,"\x3f\x00",2);
	Result->cjppStr.Connected=1;
   cjppFillDevice(&(Result->cjppStr));
#ifdef _Macintosh
	 CyberJack_SetInterruptEventNotificationProc((DeviceIdentifierRef)hDevice, MacProcessInterruptEvents, (UInt32)Result);
#else
#ifdef _WINDOWS
   if(CallbackKey)
	   InitializeCriticalSection(&Result->backs.CritSectKeyEvent);
   Result->hIntThread=cjppCreateThread((void (*)(void *))IntThread,Result);
	if(Result->cjppStr.is_usb==0)
	{
      Result->hSerialPollThread=cjppCreateThread((void (*)(void *))SerialPollThread,Result);
	}

#else
#ifdef _LINUX
	if (cjppLinux_SetInterruptEventNotificationProc(Result, hDevice)) {
		ctapiClose(Result);
		return NULL;
	}
#endif /* _LINUX */
#endif /* _WINDOWS */
#endif /* _Macintosh */

   AddToArray(Result);

#ifdef _LINUX
      DEBUGP("Sending ICC PowerOff\n");
#endif
   cjccid_iccPowerOff(Result);

#ifdef _WINDOWS
   cjppDebugOut(Result,"Init: Power Off",0,0);
#endif
   if(cjppGetDeviceInfo(&(Result->cjppStr),&(Result->Info))!=CJPP_SUCCESS)
   {

#ifdef _LINUX
      DEBUGP("Error during GetDeviceInfo\n");
#endif
      /* FIXME: don't close since it calls PowerOff which calls Write without device */
#ifdef _WINDOWS
	   cjppDebugOut(Result,"Init Error: Get Reader Info",0,0);
#endif
      ctapiClose(Result);
      return NULL;
   }

	if((Result->Info.Config & 0x07)==0x04)
	{
		if(Result->Info.ActiveModule==0 && 
			Result->Info.Version<cjppSWAB_WORD(*(unsigned short *)(easy_dat+0x7eed)))
		{
			if(cjppEasyUpdate(Result,easy_dat)!=CJPP_SUCCESS)
			{
#ifdef _WINDOWS
		  	    cjppDebugOut(Result,"Init Error: Updateing Reader",0,0);
#endif
				ctapiClose(Result);
				return NULL;
			}
			if(cjppGetDeviceInfo(&(Result->cjppStr),&(Result->Info))!=CJPP_SUCCESS)
			{

#ifdef _WINDOWS
				cjppDebugOut(Result,"Init Error: Get Reader Info 2",0,0);
#endif
				ctapiClose(Result);
				return NULL;
			}
		}
#ifdef _WINDOWS
      cjppDebugOut(Result,"Init: Leaving",0,0);
#endif
      return (CCID_DEVICE)Result;
	}

	if((Result->Info.Config & 0x03)==0x03)
	{
      Result->cjppStr.isClass2=1;
	}
	else
	{
      Result->cjppStr.isClass2=0;
	}
   LocalInfo=cjppGetLocalInfo();
   if((LocalInfo & 1) && memcmp(Result->Info.ProductionDate,"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff",17)==0)
   {
      if(cjppSetDateTime(&(Result->cjppStr),0)!=CJPP_SUCCESS)
      {
         ctapiClose(Result);
         return NULL;
      }
   }
   if((LocalInfo & 2) && memcmp(Result->Info.TestDate,"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff",17)==0)
   {
      if(cjppSetDateTime(&(Result->cjppStr),1)!=CJPP_SUCCESS)
      {
#ifdef _WINDOWS
		cjppDebugOut(Result,"Init Error: Set Date",0,0);
#endif
         ctapiClose(Result);
         return NULL;
      }
   }
   if(Result->Info.ActiveModule!=0 && (LocalInfo & 4) && memcmp(Result->Info.Seriennummer,"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff",20)==0)
   {
      if(cjppSetSerNumber(&(Result->cjppStr))!=CJPP_SUCCESS)
      {
#ifdef _WINDOWS
		cjppDebugOut(Result,"Init Error: Set SerNo",0,0);
#endif
         ctapiClose(Result);
         return NULL;
      }
   }
   if((LocalInfo & 8) && memcmp(Result->Info.FirstDate,"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff",17)==0)
   {
      if(cjppSetDateTime(&(Result->cjppStr),2)!=CJPP_SUCCESS)
      {
#ifdef _WINDOWS
		cjppDebugOut(Result,"Init Error: Set Date 2",0,0);
#endif
         ctapiClose(Result);
         return NULL;
      }
   }
#ifdef _WINDOWS
   cjppDebugOut(Result,"Init: Leaving",0,0);
#endif
   return (CCID_DEVICE)Result;
}

int cjccid_iccPowerOn(HANDLE cjDevice,unsigned char Voltage,unsigned char *ATR,int *len)
{
	CCID_Message Message;
	CCID_Response Response;

   Message.bMessageType=PC_TO_RDR_ICCPOWERON;
   Message.dwLength=0;
   Message.bSlot=0;
   Message.Header.iccPowerOn.bPowerSelect=Voltage;
   *len=0;
   CJPP_TEST(cjppWriteAndRead((HANDLE)&(((cjccidHANDLE)cjDevice)->cjppStr),&Message,&Response))
   if(Response.bStatus==0x42)
      return CJPP_ERR_NO_ICC;
   if(Response.bStatus==0x41)
   {
      if(Response.bError==XFR_PARITY_ERROR)
         return CJPP_ERR_PARITY;
      else if(Response.bError==ICC_MUTE)
         return CJPP_ERR_TIMEOUT;
      else
         return CJPP_ERR_OPEN_ICC;
   }
   *len=Response.dwLength;
   memcpy(ATR,Response.Data.abData,Response.dwLength);
   return CJPP_SUCCESS;
}

int cjccid_iccPowerOff(HANDLE cjDevice)
{
	CCID_Message Message;
	CCID_Response Response;

   Message.bMessageType=PC_TO_RDR_ICCPOWEROFF;
   Message.dwLength=0;
   Message.bSlot=0;
   CJPP_TEST(cjppWriteAndRead((HANDLE)&(((cjccidHANDLE)cjDevice)->cjppStr),&Message,&Response))
   return CJPP_SUCCESS;
}

int cjccid_GetSlotStatus(HANDLE cjDevice)
{
	CCID_Message Message;
	CCID_Response Response;

   Message.bMessageType=PC_TO_RDR_GETSLOTSTATUS;
   Message.dwLength=0;
   Message.bSlot=0;
   CJPP_TEST(cjppWriteAndRead((HANDLE)&(((cjccidHANDLE)cjDevice)->cjppStr),&Message,&Response))
   ((cjccidHANDLE)cjDevice)->Status=1;
   if(Response.bStatus&0x40)
   {
      return CJPP_ERR_PROT;
   }
   if(Response.bStatus==2)
   {
     ((cjccidHANDLE)cjDevice)->Status=0;
      return CJPP_ERR_NO_ICC;
   }
   if(Response.bStatus==1)
      return CJPP_ERR_NO_ACTIVE_ICC;
   return CJPP_SUCCESS;
}

int cjccid_SetParameters(HANDLE cjDevice,unsigned char ProtocolNum,unsigned char FI_DI,unsigned char EDC,unsigned char GuardTime,unsigned char WaitingInteger)
{
	CCID_Message Message;
	CCID_Response Response;

   Message.bMessageType=PC_TO_RDR_SETPARAMETERS;
   Message.Header.SetParameters.bProtocolNum=ProtocolNum;
   Message.bSlot=0;
   switch(ProtocolNum)
   {
      case 0:
         Message.dwLength=5;
         Message.Data.SetParameters.T0.bmFindexDindex=FI_DI;
         Message.Data.SetParameters.T0.bmTCCKST0=0;
         Message.Data.SetParameters.T0.GuardTimeT0=GuardTime;
         Message.Data.SetParameters.T0.bWaitingIntegerT0=WaitingInteger;
         Message.Data.SetParameters.T0.bClockStop=0;
         break;
      case 1:
         Message.dwLength=7;
         Message.Data.SetParameters.T1.bmFindexDindex=FI_DI;
         Message.Data.SetParameters.T1.bmTCCKST1=EDC;
         Message.Data.SetParameters.T1.GuardTimeT1=GuardTime;
         Message.Data.SetParameters.T1.bWaitingIntegerT1=WaitingInteger;
         Message.Data.SetParameters.T1.bClockStop=0;
         Message.Data.SetParameters.T1.bIFSC=254;
         Message.Data.SetParameters.T1.bNadValue=0;
         break;
      default:;
   }
   CJPP_TEST(cjppWriteAndRead((HANDLE)&(((cjccidHANDLE)cjDevice)->cjppStr),&Message,&Response))
   if(Response.bStatus==0x42)
      return CJPP_ERR_NO_ICC;
   if(Response.bStatus==0x41)
      return CJPP_ERR_NO_ACTIVE_ICC;
   return CJPP_SUCCESS;
}


int cjccid_XfrBlock(HANDLE cjDevice,unsigned char BWI,unsigned char *out,int out_len,unsigned char *in,int *in_len,unsigned short wLevelParameter)
{
	CCID_Message Message;
	CCID_Response Response;

   Message.bMessageType=PC_TO_RDR_XFRBLOCK;
   Message.dwLength=out_len;
   Message.bSlot=0;
   Message.Header.XfrBlock.bBWI=BWI;
   Message.Header.XfrBlock.wLevelParameter=cjppSWAB_WORD_2(wLevelParameter);
   memcpy(Message.Data.abData,out,out_len);
   CJPP_TEST(cjppWriteAndRead((HANDLE)&(((cjccidHANDLE)cjDevice)->cjppStr),&Message,&Response))
   if(Response.bStatus & 0x02)
      return CJPP_ERR_NO_ICC;
   if(Response.bStatus & 0x01)
      return CJPP_ERR_NO_ACTIVE_ICC;
   if(Response.bStatus==0x40)
   {
      if(Response.bError==XFR_PARITY_ERROR)
         return CJPP_ERR_PARITY;
      else if(Response.bError==ICC_MUTE)
         return CJPP_ERR_TIMEOUT;
      else
         return CJPP_ERR_LEN;
   }
   if(*in_len<Response.dwLength)
      return CJPP_ERR_RBUFFER_TO_SMALL;
   memcpy(in,Response.Data.abData,Response.dwLength);
   *in_len=Response.dwLength;
   return CJPP_SUCCESS;
}


char CJPP_CALL_CONV innerctapiClose(CCID_DEVICE hDevice)
{
   ((cjccidHANDLE)hDevice)->backs.Fini=1;
   cjccidClose(&((cjccidHANDLE)hDevice)->cjppStr);
#ifdef _WINDOWS
   WaitForSingleObject(((cjccidHANDLE)hDevice)->hIntThread,2000);
   cjppTerminateThread(((cjccidHANDLE)hDevice)->hIntThread);
   if(((cjccidHANDLE)hDevice)->backs.CallbackKey)
	   DeleteCriticalSection(&(((cjccidHANDLE)hDevice)->backs.CritSectKeyEvent));
	if(((cjccidHANDLE)hDevice)->cjppStr.is_usb==0)
	{
	   WaitForSingleObject(((cjccidHANDLE)hDevice)->hSerialPollThread,2000);
      cjppTerminateThread(((cjccidHANDLE)hDevice)->hSerialPollThread);
	}
	DeleteCriticalSection(&(((cjccidHANDLE)hDevice)->cjppStr.CritSectClose));
#endif
   ClearFromArray((cjccidHANDLE)hDevice);
   free(((cjccidHANDLE)hDevice)->cjppStr.ReaderName);
   free((cjccidHANDLE)hDevice);
   return 0;
}


CJPP_EXP_TYPE char CJPP_CALL_CONV ctapiClose(CCID_DEVICE hDevice)
{
   if(!IsInArray((cjccidHANDLE)hDevice))
	{
#ifdef _WINDOWS
      cjppDebugOut(NULL,"Close Error: No Object",0,0);
#endif
      return 0;
	}
#ifdef _WINDOWS
      cjppDebugOut(&(((cjccidHANDLE)hDevice)->cjppStr),"Closing",0,0);
#endif
	cjccid_iccPowerOff(hDevice);
	innerctapiClose(hDevice);
   return 0;
}

int cjppSetDateTime(HANDLE cjppDevice,unsigned char bOffset)
{
	CCID_Message Message;
	CCID_Response Response;
   struct tm *t;
   time_t tim;

   time(&tim);
   t=localtime(&tim);


   Message.bMessageType=PC_TO_RDR_ESCAPE;
   Message.dwLength=19;
   Message.bSlot=0;
   Message.Data.Escape.bFunction=CCID_ESCAPE_SET_DATE_TIME;
   Message.Data.Escape.Data.SetDateTime.bOffset=bOffset;
#ifdef _WINDOWS
   sprintf((char *)Message.Data.Escape.Data.SetDateTime.Date,"%#02d.%#02d.%#04d",t->tm_mday,t->tm_mon+1,t->tm_year+1900);
   sprintf((char *)Message.Data.Escape.Data.SetDateTime.Time,"%#02d:%#02d",t->tm_hour,t->tm_min);
#else
	 ///\todo check correct usage of format string, "%#02d" is not ANSI standard!
   sprintf((char *)Message.Data.Escape.Data.SetDateTime.Date,"%02d.%02d.%04d",t->tm_mday,t->tm_mon+1,t->tm_year+1900);
   sprintf((char *)Message.Data.Escape.Data.SetDateTime.Time,"%02d:%02d",t->tm_hour,t->tm_min);
#endif

   CJPP_TEST(cjppWriteAndRead(cjppDevice,&Message,&Response))
   if(Response.dwLength!=0)
      return CJPP_ERR_WRONG_LENGTH;
   return CJPP_SUCCESS;
}

int cjppSetSerNumber(HANDLE cjppDevice)
{
   int Res;
	CCID_Message Message;
	CCID_Response Response;
   unsigned long uid;

   Message.bMessageType=PC_TO_RDR_ESCAPE;
   Message.dwLength=21;
   Message.bSlot=0;
   Message.Data.Escape.bFunction=CCID_ESCAPE_SET_SERNUMBER;
   uid=cjppGetUniqueID();
   memset(Message.Data.Escape.Data.SetSerNumber.SerNumber,0,sizeof(Message.Data.Escape.Data.SetSerNumber.SerNumber));
   for(Res=0;Res<20;Res+=2)
   {
      Message.Data.Escape.Data.SetSerNumber.SerNumber[Res]=(unsigned char)('0'+uid%10);
      uid/=10;
   }

   CJPP_TEST(cjppWriteAndRead(cjppDevice,&Message,&Response))
   if(Response.dwLength!=0)
      return CJPP_ERR_WRONG_LENGTH;
   return CJPP_SUCCESS;

}




static int AddToArray(cjccidHANDLE hndl)
{
   int i;
   for(i=0;i<512;i++)
   {
      if(AllHandles[i]==NULL)
      {
         AllHandles[i]=hndl;
         return 1;
      }
   }
   return 0;
}

static void ClearFromArray(cjccidHANDLE hndl)
{
   int i;
   for(i=0;i<512;i++)
   {
      if(AllHandles[i]==hndl)
      {
         AllHandles[i]=0;
         break;
      }
   }
}

static int IsInArray(cjccidHANDLE hndl)
{
   int i;
   for(i=0;i<512;i++)
   {
      if(AllHandles[i]==hndl)
      {
         return -1;
      }
   }
	return 0;
}

#ifdef IT_TEST
   unsigned char Slot=0;
#endif

#ifdef IT_TEST
int cjccid_SecurePV(HANDLE cjDevice,unsigned char Timeout,
                    unsigned char PinPosition,unsigned char PinType,
                    unsigned char PinLengthSize,unsigned char PinLength,
                    unsigned char PinLengthPosition,
                    unsigned char Min, unsigned char Max,
                    unsigned char Condition,unsigned char *Prologue,
                    unsigned char *out,int out_len,unsigned char *in,int *in_len,unsigned char Slot)
#else
int cjccid_SecurePV(HANDLE cjDevice,unsigned char Timeout,
                    unsigned char PinPosition,unsigned char PinType,
                    unsigned char PinLengthSize,unsigned char PinLength,
                    unsigned char PinLengthPosition,
                    unsigned char Min, unsigned char Max,
                    unsigned char Condition,unsigned char *Prologue,
                    unsigned char *out,int out_len,unsigned char *in,int *in_len)
#endif
{
	CCID_Message Message;
	CCID_Response Response;
	int Res;

   Message.bMessageType=PC_TO_RDR_SECURE;
   Message.dwLength=out_len+15;
#ifdef IT_TEST
   Message.bSlot=Slot;
#else
   Message.bSlot=0;
#endif
   Message.Header.Secure.bBWI=0;
   Message.Header.Secure.wLevelParameter=cjppSWAB_WORD_2(0);
   Message.Data.Secure.bPINOperation=0;
   Message.Data.Secure.bTimeOut=Timeout;
   Message.Data.Secure.bmFormatString=(unsigned char)(0x80 | (PinPosition<<3) | PinType);
   Message.Data.Secure.bmPINBlockString=(unsigned char)((PinLengthSize<<4) | PinLength);
   Message.Data.Secure.bmPINLengthFormat=PinLengthPosition;
   Message.Data.Secure.Data.Verify.wPINMaxExtraDigit=cjppSWAB_WORD_2((((unsigned short)Min)<<8)+Max);
   Message.Data.Secure.Data.Verify.bEntryValidationCondition=Condition;
   Message.Data.Secure.Data.Verify.bNumberMessage=0xff;
   Message.Data.Secure.Data.Verify.wLangId=cjppSWAB_WORD_2(0x0409);
   Message.Data.Secure.Data.Verify.bMsgIndex=0;
   memcpy(Message.Data.Secure.Data.Verify.bTeoPrologue,Prologue,3);
   memcpy(Message.Data.Secure.Data.Verify.abData,out,out_len);
   Res=cjppWriteAndRead((HANDLE)&(((cjccidHANDLE)cjDevice)->cjppStr),&Message,&Response);
#ifdef _WINDOWS
   if((((cjccidHANDLE)cjDevice)->Info.Flags & 1)==0)
   {
       EnterCriticalSection(&(((cjccidHANDLE)cjDevice)->backs.CritSectKeyEvent));
       if(((cjccidHANDLE)cjDevice)->backs.state==COUNT_DIALOG)
	   {
	      (*(((cjccidHANDLE)cjDevice)->backs.CallbackKey))(((cjccidHANDLE)cjDevice)->backs.hClass,0x8b);
		  ((cjccidHANDLE)cjDevice)->backs.state=NO_DIALOG;
	   }
       else if(((cjccidHANDLE)cjDevice)->backs.state==PIN_DIALOG)
	   {
	      (*(((cjccidHANDLE)cjDevice)->backs.CallbackKey))(((cjccidHANDLE)cjDevice)->backs.hClass,0x8d);
		  ((cjccidHANDLE)cjDevice)->backs.state=NO_DIALOG;
	   }
       LeaveCriticalSection(&(((cjccidHANDLE)cjDevice)->backs.CritSectKeyEvent));
   }
#endif
   if(Res==CJPP_SUCCESS)
   {
		if(Response.bStatus & 0x02)
		  Res=CJPP_ERR_NO_ICC;
		else if(Response.bStatus & 0x01)
		  Res=CJPP_ERR_NO_ACTIVE_ICC;
		else if(Response.bStatus==0x40)
		{
		  if(Response.bError==XFR_PARITY_ERROR)
			 Res=CJPP_ERR_PARITY;
		  else if(Response.bError==ICC_MUTE)
			 Res=CJPP_ERR_TIMEOUT;
		  else if(Response.bError==PIN_TIMEOUT)
			 Res=CJPP_ERR_PIN_TIMEOUT;
		  else if(Response.bError==PIN_CANCELED)
			 Res=CJPP_ERR_PIN_CANCELED;
		  else if(Response.bError==5)
			 Res=CJPP_ERR_WRONG_PARAMETER;
		  else
			 Res=CJPP_ERR_LEN;
		}
		if(*in_len<Response.dwLength)
		  Res=CJPP_ERR_RBUFFER_TO_SMALL;
   }
   if(Res)
	   return Res;
   memcpy(in,Response.Data.abData,Response.dwLength);
   *in_len=Response.dwLength;
   return CJPP_SUCCESS;
}

int cjccid_SecureMV(HANDLE cjDevice,unsigned char Timeout,
                    unsigned char PinPosition,unsigned char PinType,
                    unsigned char PinLengthSize,unsigned char PinLength,
                    unsigned char PinLengthPosition,
                    unsigned char Min, unsigned char Max,
                    unsigned char bConfirmPIN,
                    unsigned char Condition,unsigned char *Prologue,
                    unsigned char OffsetOld,unsigned char OffsetNew,
                    unsigned char *out,int out_len,unsigned char *in,int *in_len)
{
	CCID_Message Message;
	CCID_Response Response;

   Message.bMessageType=PC_TO_RDR_SECURE;
   Message.dwLength=out_len+20;
   Message.bSlot=0;
   Message.Header.Secure.bBWI=0;
   Message.Header.Secure.wLevelParameter=cjppSWAB_WORD_2(0);
   Message.Data.Secure.bPINOperation=1;
   Message.Data.Secure.bTimeOut=Timeout;
   Message.Data.Secure.bmFormatString=(unsigned char)(0x80 | (PinPosition<<3) | PinType);
   Message.Data.Secure.bmPINBlockString=(unsigned char)((PinLengthSize<<4) | PinLength);
   Message.Data.Secure.bmPINLengthFormat=PinLengthPosition;
   Message.Data.Secure.Data.Modify.bInsertionOffsetOld=OffsetOld;
   Message.Data.Secure.Data.Modify.bInsertionOffsetNew=OffsetNew;
   Message.Data.Secure.Data.Modify.wPINMaxExtraDigit=cjppSWAB_WORD_2((((unsigned short)Min)<<8)+Max);
   Message.Data.Secure.Data.Modify.bConfirmPIN= bConfirmPIN;
   Message.Data.Secure.Data.Modify.bEntryValidationCondition=Condition;
   Message.Data.Secure.Data.Modify.bNumberMessage=0xff;
   Message.Data.Secure.Data.Modify.wLangId=cjppSWAB_WORD_2(0x0409);
   Message.Data.Secure.Data.Modify.bMsgIndex1=0;
   Message.Data.Secure.Data.Modify.bMsgIndex2=0;
   Message.Data.Secure.Data.Modify.bMsgIndex3=0;
   memcpy(Message.Data.Secure.Data.Modify.bTeoPrologue,Prologue,3);
   memcpy(Message.Data.Secure.Data.Modify.abData,out,out_len);
   CJPP_TEST(cjppWriteAndRead((HANDLE)&(((cjccidHANDLE)cjDevice)->cjppStr),&Message,&Response))
   if(Response.bStatus & 0x02)
      return CJPP_ERR_NO_ICC;
   if(Response.bStatus & 0x01)
      return CJPP_ERR_NO_ACTIVE_ICC;
   if(Response.bStatus==0x40)
   {
      if(Response.bError==XFR_PARITY_ERROR)
         return CJPP_ERR_PARITY;
      else if(Response.bError==ICC_MUTE)
         return CJPP_ERR_TIMEOUT;
      else if(Response.bError==PIN_TIMEOUT)
         return CJPP_ERR_PIN_TIMEOUT;
      else if(Response.bError==PIN_CANCELED)
         return CJPP_ERR_PIN_CANCELED;
      else if(Response.bError==PIN_DIFFERENT)
         return CJPP_ERR_PIN_DIFFERENT;
      else
         return CJPP_ERR_LEN;
   }
   if(*in_len<Response.dwLength)
      return CJPP_ERR_RBUFFER_TO_SMALL;
   memcpy(in,Response.Data.abData,Response.dwLength);
   *in_len=Response.dwLength;
   return CJPP_SUCCESS;
}

int cjccid_GetStackSignCounter(HANDLE cjppDevice,unsigned short *Counter)
{
	CCID_Message Message;
	CCID_Response Response;
	
	Message.bMessageType=PC_TO_RDR_ESCAPE;
	Message.dwLength=1;
	Message.bSlot=0;
	Message.Data.Escape.bFunction=CCID_ESCAPE_GET_STACKSIGNCOUNTER;
	CJPP_TEST(cjppWriteAndRead(cjppDevice,&Message,&Response))
	if(Response.dwLength!=2)
		return CJPP_ERR_WRONG_LENGTH;
	*Counter=cjppSWAB_WORD(Response.Data.Escape.Function.StackSignCounter);
	return CJPP_SUCCESS;
}
