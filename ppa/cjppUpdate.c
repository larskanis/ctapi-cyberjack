
#include "Platform.h"
#include "cjppa.h"
#include "cjpp.h"
#include "cjpp.h"

#ifdef OS_LINUX
#include <string.h>
#include <malloc.h>
#endif /* OS_LINUX */


static int cjppVerifyKey(HANDLE cjppDevice,unsigned char *sign);

//extern int cjppVerifyData(HANDLE cjppDevice,unsigned short addr,unsigned char *Data,unsigned char len);

static void cjppWINAPI ProgressbarThread(ProgressStr *Params)
{
	while(--(Params->CountProgessCallbacks) && !Params->Fini)
	{
		cjppSleep(Params->SleepValue);
		if(!Params->Fini)
			(*(Params->CallbackProgress))(Params->hClass);
	}
}

/*static HANDLE cjppOpen(char *cDeviceName)
{
char *myName;
cjppHANDLE hcjppDevice=malloc(sizeof(cjppStruct));
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
Result->cjppStr.reader_id=atoi(cDeviceName+strlen(cDeviceName)-2);
}
free(myName);
memset(hcjppDevice,0,sizeof(cjppStruct));
cjppFillDevice(hcjppDevice);
hcjppDevice->hDevice = cjppCreate( cDeviceName);
if(hcjppDevice->hDevice==0)
{
CloseHandle(hcjppDevice->o_transmit.hEvent);
CloseHandle(hcjppDevice->o_receive.hEvent);
free(hcjppDevice);
return (HANDLE)CJPP_ERR_OPENING_DEVICE;
}
return (HANDLE)hcjppDevice;
}*/

CJPP_EXP_TYPE int CJPP_CALL_CONV cJppFirmwareUpdate(char *cDeviceName,              /*Geraetenamen: z.B. \\.\cjccid01*/
													unsigned long Length,           /*Lnge der UpdateDaten*/
													unsigned char *UpdateData,      /*Daten wie in SGN-Datei*/
																					HANDLE hClass,                  /*this-Pointer der der
																													Callbackfunktion
																													übergeben wird */
																													void (CJPP_CALLBACK_TYPE *CallbackProgress)(CCID_CTX),
																													int CountProgessCallbacks)      /*Anzahl der gewünschten
																													Callbacks*/
{
	int Res;
	HANDLE hCtDevice;
	cjccidHANDLE hCt;
	unsigned short AnzahlSign;
	unsigned short AnzahlSignHelp;
	unsigned long time_needed=0;
	unsigned char *loader;
	unsigned char *appl;
	unsigned short addr;
	ProgressStr Progress;
	HANDLE hThread=NULL;
	unsigned char *sign;
	unsigned long minSize=0;
	
	//MessageBox(NULL,"test","test",MB_OK);
	hCt=(cjccidHANDLE)ctapiInit(cDeviceName,NULL,NULL,NULL);
	if((int)hCt==0)
		return CJPP_ERR_OPENING_DEVICE;
	hCtDevice = (HANDLE)&(((cjccidHANDLE)hCt)->cjppStr);
	
    if(!(((cjccidHANDLE)hCt)->Info.Config & 0x01)) /*It's a not updatable Reader*/
	{
		ctapiClose(hCt);
		return CJPP_NOT_UPDATABLE;
	}
	
	
	/*Berechnung der bentigten Zeit*/
	if(((cjccidHANDLE)hCt)->Info.BootLoaderVersion==0x0050)
	{
		/*unsigned char Programm[32768];*/
		unsigned char KeyVersion=0;
		
		minSize=65542 + // DPW Section
			32772 + // Loader Section for DRV and DSU
			32770 + // Application Section for DRV and DSU
			135   + // Public Key + Constands
			128   + // at least one Signature
			32768;  // Application Section for RSCT
		
		if(Length<minSize)
		{
			ctapiClose(hCt);
			return CJPP_ERR_FIRMWARE_OLD;
		}
		
		//      UpdateData+=131084;
		UpdateData+=65542;//Skip the DPW Section;
		
		//    Skip the Loaderblocks for DRV and DSU      
		AnzahlSign=cjppSWAB_WORD_2(*(unsigned short *)UpdateData);
		UpdateData+=2;
		UpdateData+=(32770L * AnzahlSign);
		
		//    Skip the Applicationblock for DRV and DSU      
		UpdateData+=32770;
		
		minSize+=(AnzahlSign-1)*32770;
		if(Length<minSize)
		{
			ctapiClose(hCt);
			return CJPP_ERR_FIRMWARE_OLD;
		}
		
		AnzahlSign=cjppSWAB_WORD_2(*(unsigned short *)UpdateData);
		minSize+=(AnzahlSign-1)*128;
		if(Length<minSize)
		{
			ctapiClose(hCt);
			return CJPP_ERR_FIRMWARE_OLD;
		}
		
		// Do we have an flashed class 1 reader? Not yet!
		if(!(((cjccidHANDLE)hCt)->Info.Config & 0x02))
		{
			minSize+=32768+135+128;
			if(Length<minSize)
			{
				ctapiClose(hCt);
				return CJPP_ERR_FIRMWARE_OLD;
			}
			UpdateData+=32768+135+AnzahlSign*128;
			AnzahlSign=cjppSWAB_WORD_2(*(unsigned short *)UpdateData);
			minSize+=(AnzahlSign-1)*128;
			if(Length<minSize)
			{
				ctapiClose(hCt);
				return CJPP_ERR_FIRMWARE_OLD;
			}
		}
		
		appl=UpdateData+4+131+AnzahlSign*128;
		UpdateData+=2;
		
		//if the firmware in the reader is newer than that of this file
#ifdef IT_TEST
		if(0)
#else
		if(cjppSWAB_WORD(*(unsigned short *)(appl+0x7eed))<hCt->Info.ApplicationVersion && hCt->Info.ApplicationVersion!=0xffff)
#endif
		{
			ctapiClose(hCt);
			return CJPP_ERR_FIRMWARE_OLD;
		}
			// if the application is aktive than we have to switch to the loader
		if(hCt->Info.ActiveModule==CJPP_APPLICATION)
		{
			CJPP_TEST2(cjppStartLoader(hCtDevice))
			cjppSleep(500);
		}
			
			//Do we need to update the key
		CJPP_TEST2(cjppGetKeyVersion(hCtDevice,&KeyVersion))
		if(cjppSWAB_WORD_2(*(unsigned short *)UpdateData)!=KeyVersion)
		{
				//Is the key in the reader newer than that of this file
			if(KeyVersion>cjppSWAB_WORD_2(*(unsigned short *)UpdateData))
			{
				ctapiClose(hCt);
				return CJPP_ERR_NO_SIGN;
			}
				
			sign=UpdateData+2+(int)KeyVersion*128; //sign with old private Key
			UpdateData+=2+AnzahlSign*128;          //pointer to the new public key
				
				//calculate the time we need for the update application and keys
			time_needed=50000;
			if(CallbackProgress!=NULL)
			{
				Progress.SleepValue=time_needed/CountProgessCallbacks;
				Progress.CallbackProgress=CallbackProgress;
				Progress.hClass=hClass;
				Progress.CountProgessCallbacks=CountProgessCallbacks;
				Progress.Fini=0;
				hThread=cjppCreateThread((void (*)(void *))ProgressbarThread,&Progress);
			}
				
				//Updating the Key
			CJPP_TEST2(cjppUpdateData(hCtDevice,0x4000,UpdateData,64))
			CJPP_TEST2(cjppUpdateData(hCtDevice,0x4040,UpdateData+64,64))
			CJPP_TEST2(cjppUpdateData(hCtDevice,0x4080,UpdateData+128,3))
			CJPP_TEST2(cjppVerifyKey(hCtDevice,sign))
		}
		else
		{
			time_needed=30000;
			if(CallbackProgress!=NULL)
			{
				Progress.SleepValue=time_needed/CountProgessCallbacks;
				Progress.CallbackProgress=CallbackProgress;
				Progress.hClass=hClass;
				Progress.CountProgessCallbacks=CountProgessCallbacks;
				Progress.Fini=0;
				hThread=cjppCreateThread((void (*)(void *))ProgressbarThread,&Progress);
			}
		}
			
			//Updating the programm
			//		memcpy(Programm,appl,sizeof(Programm));
			//		appl=Programm;
		for(addr=0x0000;addr<0x7f00;addr+=(unsigned short)64,appl+=64)
		{
			CJPP_TEST2(cjppUpdateData(hCtDevice,addr,appl,64))
		}
		CJPP_TEST2(cjppVerifyUpdate(hCtDevice))
		cjppSleep(500);
		ctapiClose(hCt);
		if(CallbackProgress!=NULL)
		{
			Progress.Fini=1;
			cjppSleep(1000);
			cjppTerminateThread(hThread);
		}
		return CJPP_SUCCESS;
	}
	else if(hCt->Info.Version>10)
	{
		AnzahlSign=cjppSWAB_WORD_2(*(unsigned short *)UpdateData);
		UpdateData+=2;
		
		//if it is an DRV or DSU than we have to skip the DPW Section
		if(hCt->Info.BootLoaderVersion!=0x0212)
		{
			UpdateData+=(int)(AnzahlSign+1)*32770;
			AnzahlSign=cjppSWAB_WORD_2(*(unsigned short *)UpdateData);
			UpdateData+=2;
		}
		AnzahlSignHelp=AnzahlSign;
		while(AnzahlSign>1 && cjppSWAB_WORD_2(*(unsigned short *)(UpdateData+32770))<=hCt->Info.LoaderVersion)
		{
			UpdateData+=32770;
			AnzahlSign--;
		}
		loader=UpdateData+2;
		UpdateData+=(int)AnzahlSign*32770;
		
		
		
		if(cjppSWAB_WORD(*(unsigned short *)(loader+0x7e6b))>hCt->Info.LoaderVersion)
		{
			if(hCt->Info.LoaderVersion>=0x100e)
				time_needed+=60000;
			else
				time_needed+=110000;
		}
		else
		{
			loader=NULL;
		}
		
		AnzahlSign=AnzahlSignHelp;
		
		appl=UpdateData+2;
#ifdef IT_TEST
		if(1)
#else
		if(cjppSWAB_WORD(*(unsigned short *)(appl+0x7e6b))>=hCt->Info.ApplicationVersion || hCt->Info.ApplicationVersion==0xffff)
#endif
		{
			time_needed+=35000;
		}
		else
		{
			appl=NULL;
		}
	}
	else
	{
		AnzahlSign=cjppSWAB_WORD_2(*(unsigned short *)UpdateData);/*first Version of reader, only tree parts delivered*/
		UpdateData+=2;
		loader=UpdateData+2;
		UpdateData+=(int)AnzahlSign*32770;
		appl=UpdateData+2;
		time_needed=215000;
	}
	
	if(loader!=NULL || appl!=NULL)
	{
		if(hCt->Info.ActiveModule==CJPP_APPLICATION && hCt->Info.BootLoaderVersion!=0x0050)
			time_needed+=8000;
		if(CallbackProgress!=NULL)
		{
			Progress.SleepValue=time_needed/CountProgessCallbacks;
			Progress.CallbackProgress=CallbackProgress;
			Progress.hClass=hClass;
			Progress.CountProgessCallbacks=CountProgessCallbacks;
			Progress.Fini=0;
			hThread=cjppCreateThread((void (*)(void *))ProgressbarThread,&Progress);
		}
		
		if(hCt->Info.ActiveModule==CJPP_APPLICATION && hCt->Info.BootLoaderVersion!=0x0050)
		{
			CJPP_TEST2(cjppStartLoader(hCtDevice))
			innerctapiClose(hCt);
			cjppSleep(8000);
			hCt=ctapiInit(cDeviceName,NULL,NULL,NULL);
			if((int)hCt==0)
			{
				if(CallbackProgress!=NULL)
				{
				   Progress.Fini=1;
				   cjppSleep(1000);
				   cjppTerminateThread(hThread);
				}
				return CJPP_ERR_OPENING_DEVICE;
			}
		}
		if(loader)
		{
			loader+=8;
			CJPP_TEST2(cjppUpdateData(hCtDevice,8,loader,56))
				loader+=56;
			CJPP_TEST2(cjppUpdateData(hCtDevice,64,loader,64))
				loader+=64;
			CJPP_TEST2(cjppUpdateData(hCtDevice,128,loader,64))
				loader+=64;
			CJPP_TEST2(cjppUpdateData(hCtDevice,192,loader,64))
				loader+=64+0x500;
			for(addr=0x0600;addr<0x7f00;addr+=(unsigned short)64,loader+=64)
			{
				CJPP_TEST2(cjppUpdateData(hCtDevice,addr,loader,64))
			}
			CJPP_TEST2(cjppVerifyUpdate(hCtDevice))
			innerctapiClose(hCt);
			cjppSleep(12000);
		}
		if(appl)
		{
			if(loader)
			{
				hCt=ctapiInit(cDeviceName,NULL,NULL,NULL);
				if((int)hCt==0)
				{
					if(CallbackProgress!=NULL)
					{
					   Progress.Fini=1;
					   cjppSleep(1000);
					   cjppTerminateThread(hThread);
					}
					return CJPP_ERR_OPENING_DEVICE;
				}
			}
			appl+=8;
			CJPP_TEST2(cjppUpdateData(hCtDevice,8,appl,56))
			appl+=56;
			CJPP_TEST2(cjppUpdateData(hCtDevice,64,appl,64))
			appl+=64;
			CJPP_TEST2(cjppUpdateData(hCtDevice,128,appl,64))
			appl+=64;
			CJPP_TEST2(cjppUpdateData(hCtDevice,192,appl,64))
			appl+=64+0x500;
			for(addr=0x600;addr<0x7f00;addr+=(unsigned short)64,appl+=64)
			{
				CJPP_TEST2(cjppUpdateData(hCtDevice,addr,appl,64))
			}
			appl+=8;
			CJPP_TEST2(cjppVerifyUpdate(hCtDevice))
			innerctapiClose(hCt);
			cjppSleep(5000);
		}
		if(CallbackProgress!=NULL)
		{
			Progress.Fini=1;
			cjppSleep(1000);
			cjppTerminateThread(hThread);
		}
		return CJPP_SUCCESS;
   }
   else
   {
	   ctapiClose(hCt);
	   return CJPP_ERR_FIRMWARE_OLD;
   }
}

int cjppWriteAndRead(HANDLE cjppDevice,CCID_Message *Message,CCID_Response *Response)
{
	Message->dwLength=cjppSWAB_DWORD_2(Message->dwLength);
	Message->bSeq=((cjppHANDLE)cjppDevice)->bSeq++;
	CJPP_TEST(cjppTransfer(cjppDevice,Message,Response))
		Response->dwLength=cjppSWAB_DWORD_2(Response->dwLength);
	if(Message->bSeq!=Response->bSeq) {
#ifdef _LINUX
		DEBUGP("Message-bSeq=0x%08x, Response->bSeq=0x%08x\n",
			Message->bSeq, Response->bSeq);
#endif
		return CJPP_ERR_SEQ;
	}
	switch(Message->bMessageType)
	{
	case PC_TO_RDR_ICCPOWERON:
	case PC_TO_RDR_XFRBLOCK:
	case PC_TO_RDR_SECURE:
        if(Response->bMessageType!=RDR_TO_PC_DATABLOCK)
			return CJPP_ERR_WRONG_ANSWER;
        break;
	case PC_TO_RDR_ICCPOWEROFF:
	case PC_TO_RDR_GETSLOTSTATUS:
	case PC_TO_RDR_TAPDU:
	case PC_TO_RDR_MECHANICAL:
	case PC_TO_RDR_ABORT:
        if(Response->bMessageType!=RDR_TO_PC_SLOTSTATUS)
			return CJPP_ERR_WRONG_ANSWER;
        break;
	case PC_TO_RDR_GETPARAMETERS:
	case PC_TO_RDR_RESETPARAMETERS:
	case PC_TO_RDR_SETPARAMETERS:
        if(Response->bMessageType!=RDR_TO_PC_PARAMETERS)
			return CJPP_ERR_WRONG_ANSWER;
        break;
	case PC_TO_RDR_ESCAPE:
        if(Response->bMessageType!=RDR_TO_PC_ESCAPE)
			return CJPP_ERR_WRONG_ANSWER;
        break;
	case PC_TO_RDR_ICCCLOCK:
        if(Response->bMessageType!=RDR_TO_PC_STATUSCLOCK)
			return CJPP_ERR_WRONG_ANSWER;
        break;
	case PC_TO_RDR_SETDATARATEANDCLOCKFREQUENCY:
        if(Response->bMessageType!=RDR_TO_PC_DATARATEANDCLOCKFREQUENCY)
			return CJPP_ERR_WRONG_ANSWER;
        break;
	default:
        return CJPP_ERR_WRONG_ANSWER;
	}
	return CJPP_SUCCESS;
}

int cjppGetDeviceInfo(HANDLE cjppDevice,cjpp_Info *cjppInfo)
{
	CCID_Message Message;
	CCID_Response Response;
	
	Message.bMessageType=PC_TO_RDR_ESCAPE;
	Message.dwLength=1;
	Message.bSlot=0;
	Message.Data.Escape.bFunction=CCID_ESCAPE_GET_INFO;
#ifdef _WINDOWS
    cjppDebugOut((cjppHANDLE)cjppDevice,"GetDevInfo",0,0);
#endif
	
	CJPP_TEST(cjppWriteAndRead(cjppDevice,&Message,&Response))
		if(Response.dwLength!=sizeof(cjpp_Info) && Response.dwLength!=18) {
#ifdef _LINUX
			DEBUGP("Response.dwLength=%u, cjpp_Info=%u\n", 
				Response.dwLength, sizeof(cjpp_Info));
#endif
			return CJPP_ERR_WRONG_LENGTH;
		}
		memcpy(cjppInfo,Response.Data.abData,sizeof(cjpp_Info));
		cjppInfo->Version=cjppSWAB_WORD(cjppInfo->Version);
		cjppInfo->LoaderVersion=cjppSWAB_WORD(cjppInfo->LoaderVersion);
		cjppInfo->ApplicationVersion=cjppSWAB_WORD(cjppInfo->ApplicationVersion);
		cjppInfo->BootLoaderVersion=cjppSWAB_WORD(cjppInfo->BootLoaderVersion);
		return CJPP_SUCCESS;
}

int cjppStartLoader(HANDLE cjppDevice)
{
	CCID_Message Message;
	CCID_Response Response;
	
	Message.bMessageType=PC_TO_RDR_ESCAPE;
	Message.dwLength=1;
	Message.bSlot=0;
	Message.Data.Escape.bFunction=CCID_ESCAPE_UPDATE_START;
	
	CJPP_TEST(cjppWriteAndRead(cjppDevice,&Message,&Response))
		if(Response.dwLength!=0)
			return CJPP_ERR_WRONG_LENGTH;
		
		return CJPP_SUCCESS;
}

int cjppUpdateData(HANDLE cjppDevice,unsigned short addr,unsigned char *Data,unsigned char len)
{
	CCID_Message Message;
	CCID_Response Response;
	
	Message.bMessageType=PC_TO_RDR_ESCAPE;
	Message.dwLength=4+len;
	Message.bSlot=0;
	Message.Data.Escape.bFunction=CCID_ESCAPE_UPDATE;
	Message.Data.Escape.Data.Update.wOffset=cjppSWAB_WORD(addr);
	Message.Data.Escape.Data.Update.bLength=len;
	memcpy(Message.Data.Escape.Data.Update.Data,Data,len);
	CJPP_TEST(cjppWriteAndRead(cjppDevice,&Message,&Response))
		if(Response.dwLength!=0)
			return CJPP_ERR_WRONG_LENGTH;
		if(Response.bError!=0)
			return CJPP_ERR_WRONG_SIZE;
		return CJPP_SUCCESS;
}

int cjppGetKeyVersion(HANDLE cjppDevice,unsigned char *KeyVersion)
{
	CCID_Message Message;
	CCID_Response Response;
	
	Message.bMessageType=PC_TO_RDR_ESCAPE;
	Message.dwLength=1;
	Message.bSlot=0;
	Message.Data.Escape.bFunction=CCID_ESCAPE_GET_KEY_VERSION;
	CJPP_TEST(cjppWriteAndRead(cjppDevice,&Message,&Response))
		if(Response.dwLength!=1)
			return CJPP_ERR_WRONG_LENGTH;
		if(Response.bError!=0)
			return CJPP_ERR_WRONG_SIZE;
		*KeyVersion=Response.Data.abData[0];
		return CJPP_SUCCESS;
}

int cjppVerifyUpdate(HANDLE cjppDevice)
{
	CCID_Message Message;
	CCID_Response Response;
	
	Message.bMessageType=PC_TO_RDR_ESCAPE;
	Message.dwLength=1;
	Message.bSlot=0;
	Message.Data.Escape.bFunction=CCID_ESCAPE_VERIFY;
	CJPP_TEST(cjppWriteAndRead(cjppDevice,&Message,&Response))
		if(Response.dwLength!=0)
			return CJPP_ERR_WRONG_LENGTH;
		if(Response.bError!=0)
			return CJPP_ERR_SIGN;
		return CJPP_SUCCESS;
}

int cjppVerifyKey(HANDLE cjppDevice,unsigned char *sign)
{
	CCID_Message Message;
	CCID_Response Response;
	
	Message.bMessageType=PC_TO_RDR_ESCAPE;
	Message.dwLength=129;
	Message.bSlot=0;
	Message.Data.Escape.bFunction=CCID_ESCAPE_VERIFY_KEY;
	memcpy(Message.Data.Escape.Data.UpdateKey.Key,sign,128);
	CJPP_TEST(cjppWriteAndRead(cjppDevice,&Message,&Response))
		if(Response.dwLength!=0)
			return CJPP_ERR_WRONG_LENGTH;
		if(Response.bError!=0)
			return CJPP_ERR_SIGN;
		return CJPP_SUCCESS;
}

int cjppInput(HANDLE cjppDevice,unsigned char *key,unsigned char timeout)
{
	CCID_Message Message;
	CCID_Response Response;
	
	Message.bMessageType=PC_TO_RDR_ESCAPE;
	Message.dwLength=2;
	Message.bSlot=0;
	Message.Data.Escape.bFunction=CCID_ESCAPE_INPUT;
	Message.Data.Escape.Data.Input.Timeout=timeout;
	CJPP_TEST(cjppWriteAndRead(cjppDevice,&Message,&Response))
		if(Response.dwLength!=1)
			return CJPP_ERR_WRONG_LENGTH;
		*key=Response.Data.abData[0];
		
		return CJPP_SUCCESS;
}

static int cjppDirectToRAM(HANDLE cjppDevice,unsigned short addr,unsigned char *prg)
{
	CCID_Message Message;
	CCID_Response Response;
	
	Message.bMessageType=PC_TO_RDR_ESCAPE;
	Message.dwLength=131;
	Message.bSlot=0;
	Message.Data.Escape.bFunction=CCID_ESCAPE_DIRECT_TO_RAM;
	Message.Data.Escape.Data.ToRam.wOffset=cjppSWAB_WORD(addr);
	memcpy(Message.Data.Escape.Data.ToRam.bCode,prg,128);
	CJPP_TEST(cjppWriteAndRead(cjppDevice,&Message,&Response))
		if(Response.dwLength!=0)
			return CJPP_ERR_WRONG_LENGTH;
		return CJPP_SUCCESS;
}


static int cjppDirectExecRAM(HANDLE cjppDevice,unsigned char *prg)
{
	CCID_Message Message;
	CCID_Response Response;
	
	Message.bMessageType=PC_TO_RDR_ESCAPE;
	Message.dwLength=129;
	Message.bSlot=0;
	Message.Data.Escape.bFunction=CCID_ESCAPE_EXEC_RAM;
	memcpy(Message.Data.Escape.Data.ExecRam.bIntTable,prg,128);
	CJPP_TEST(cjppWriteAndRead(cjppDevice,&Message,&Response))
		if(Response.dwLength!=0)
			return CJPP_ERR_WRONG_LENGTH;
		return CJPP_SUCCESS;
}

int cjppEasyUpdate(HANDLE cjppDevice,unsigned char *prg)
{
	unsigned short addr;
	for(addr=0x0080;addr<0x8000;addr+=128)
	{
		CJPP_TEST(cjppDirectToRAM(cjppDevice,addr,prg+addr))
	}
	CJPP_TEST(cjppDirectExecRAM(cjppDevice,prg))
		return CJPP_SUCCESS;
}





