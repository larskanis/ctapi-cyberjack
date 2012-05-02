#include "Platform.h"
#include "BaseCommunication.h"
#include <string.h>
#include "Debug.h"


CBaseCommunication::CBaseCommunication(const char *cDeviceName,CReader *Owner)
{
	m_cDeviceName=strdup(cDeviceName);
	m_Owner=Owner;
	m_Reader=NULL;
	m_InterruptPipeState=UnInit;
}


CBaseCommunication::~CBaseCommunication(void)
{
	free(m_cDeviceName);
}

void CBaseCommunication::Close()
{}



int CBaseCommunication::Write(void *Message,uint32_t len)
{
	if(IsConnected())
   {
      Debug.Out(m_cDeviceName,DEBUG_MASK_COMMUNICATION_OUT,"CCID OUT:",Message,len);
	}
      
	return ((!IsConnected())?CJ_ERR_DEVICE_LOST:CJ_SUCCESS);
}

int CBaseCommunication::Read(void *Response,uint32_t *len)
{
   if(IsConnected())
   {
      Debug.Out(m_cDeviceName,DEBUG_MASK_COMMUNICATION_IN,"CCID IN:",Response,*len);
	}
      
	return ((!IsConnected())?CJ_ERR_DEVICE_LOST:CJ_SUCCESS);
}

void CBaseCommunication::FreeIFDHandlerDeviceName(char *DeviceName)
{
	if(DeviceName!=NULL)
	   delete DeviceName;
}
