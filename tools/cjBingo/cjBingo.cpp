
#include "stdafx.h"
#include "Reader.h"
#include "eca_defines.h"

#include <stdio.h>

#include "BingoError.h"
#include "cjBingo.h"

static uint32_t m_ulLastError=0;
static CReader *m_pReader=NULL;


RNDG_RESULT RNDGeneratorInit(uint16_t port)
{
  char *devName;
  int rv;
  
  m_ulLastError=0; 
  devName=CUSBLinux::createDeviceName(port);
  if (devName==NULL)
	  return RNDG_READER_NOT_FOUND;

  m_pReader=new CReader(devName);
  rv=m_pReader->Connect();
  if (rv!=CJ_SUCCESS)
  {
	  delete m_pReader;
	  m_pReader=NULL;
	  return RNDG_READER_BUSY;
  }
  return RNDG_SUCCESS;
}

RNDG_RESULT RNDGeneratorClose(void)
{
   m_ulLastError=0; 
	if(m_pReader)
	{
		m_pReader->Disonnect();
		delete m_pReader;
		m_pReader=NULL;
	}
	return RNDG_SUCCESS;
}

uint32_t RNDGetLastError(void)
{
	return m_ulLastError;
}

static bool IsLittleEndian(void)
{
	if(htons(0x1234)!=0x1234)
		return true;
	return false;
}

static uint16_t InversByteOrderShort(uint16_t Value)
{
	return (Value << 8) | (Value >> 8);
}

static uint32_t InversByteOrderLong(uint32_t Value)
{
	return (((uint32_t)InversByteOrderShort((uint16_t)Value)) << 16) | ((uint32_t)InversByteOrderShort((uint16_t)(Value >> 16)));
}

static uint64_t InversByteOrderLongLong(uint64_t Value)
{
	return (((uint64_t)InversByteOrderLong((uint32_t)Value)) << 32) | ((uint64_t)InversByteOrderShort((uint32_t)(Value >> 32)));
}



static uint16_t HostToReaderShort(uint16_t Value)
{
   return InversByteOrderShort(htons(Value));
}

static uint32_t HostToReaderLong(uint32_t Value)
{
   return InversByteOrderLong(htonl(Value));
}

static uint64_t HostToReaderLongLong(uint64_t Value)
{
	if(IsLittleEndian())
		return Value;
	return InversByteOrderLongLong(Value);
}

static uint16_t ReaderToHostShort(uint16_t Value)
{
   return HostToReaderShort(Value);
}

static uint32_t ReaderToHostLong(uint32_t Value)
{
   return HostToReaderLong(Value);
}

static uint64_t ReaderToHostLongLong(uint64_t Value)
{
   return HostToReaderLongLong(Value);
}



static RNDG_RESULT ExecuteReaderFunction(uint16_t Function,uint8_t *InputData, uint32_t InputLen,uint8_t *ResponseData, uint32_t *ResponseLen)
{
	uint32_t Result=0;
	uint8_t InternalErrorData[4];
	uint32_t InternalErrorLen=sizeof(InternalErrorData);

	if(m_pReader==NULL)
		return RNDG_READER_NOT_CONNECTED;

	if(m_pReader->CtApplicationDataEx(MODULE_ID_BINGO_VOTING,Function,InputData,InputLen,&Result, ResponseData,ResponseLen,InternalErrorData, &InternalErrorLen)!=CJ_SUCCESS)
	{
		RNDGeneratorClose();
		return RNDG_COMMUNICATION_ERROR;
	}
	if(InternalErrorLen!=0 && InternalErrorLen!=4)
	{
		if(ResponseLen)
			*ResponseLen=0;
		RNDGeneratorClose();
		return RNDG_PROTOCOL_ERROR;
	}
	if(InternalErrorLen==4)
	{
		if(ResponseLen)
		{
			if(*ResponseLen!=0)
			{
				*ResponseLen=0;
				return RNDG_PROTOCOL_ERROR;
			}
		}
	}
	if(InternalErrorLen==4)
	{
		if(ResponseLen)
			*ResponseLen=0;
      memcpy(&m_ulLastError,InternalErrorData,sizeof(m_ulLastError)); 
		m_ulLastError=ReaderToHostLong(m_ulLastError);
		return RNDG_EXT_ERROR;
	}
	return RNDG_SUCCESS;
}



RNDG_RESULT RNDGeneratorInternalAutheticate(uint64_t RND_IFD, uint8_t *Signuture)
{
	uint32_t ResponseLen=128;
   m_ulLastError=0; 
   return ExecuteReaderFunction(0,(uint8_t *)&RND_IFD, sizeof(RND_IFD),Signuture,&ResponseLen);
}

RNDG_RESULT RNDGeneratorGenerateRND(int count,uint64_t *RNDs)
{
   RNDG_RESULT Result;
	uint32_t ResponseLen=count*8;
   uint8_t Input=(uint8_t) count;
   m_ulLastError=0; 
   Result=ExecuteReaderFunction(1,&Input,sizeof(Input),(uint8_t *)RNDs,&ResponseLen);
	if(Result==RNDG_SUCCESS)
	{
		while(count)
		{
			count--;
			RNDs[count]=ReaderToHostLongLong(RNDs[count]);
		}
	}
	return Result;
}

RNDG_RESULT RNDGeneratorShowLastRNDs(void)
{
   RNDG_RESULT Result;
   m_ulLastError=0; 

   Result=ExecuteReaderFunction(2,NULL,0,NULL,NULL);
	if(Result==RNDG_EXT_ERROR)
	{
		if(m_ulLastError==ERROR_USER_TIMEOUT)
		{
		   m_ulLastError=0; 
			return RNDG_TIMEOUT;
		}
		if(m_ulLastError==ERROR_USER_ABORT)
		{
		   m_ulLastError=0; 
			return RNDG_ABORT;
		}
	}
	return Result;
}





