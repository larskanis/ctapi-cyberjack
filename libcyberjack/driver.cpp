/***************************************************************************
    begin       : Tue Mar 24 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "driver.hpp"
#include "cyberjack_l.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



namespace Cyberjack {



Reader::Reader(Driver *driver,
               const std::string &name,
	       const std::string &productString,
               const std::string &serial,
               int busType,
               uint32_t vendorId,
               uint32_t productId)
:m_driver(driver)
,m_name(name)
,m_productString(productString)
,m_serial(serial)
,m_busType(busType)
,m_vendorId(vendorId)
,m_productId(productId)
,m_updateCounter(0)
{
  /* clear reader info */
  memset(&m_readerInfo, 0, sizeof(m_readerInfo));
  m_readerInfo.SizeOfStruct=sizeof(m_readerInfo);
}



Reader::~Reader() {
}



int Reader::connect(int object) {
  return ErrorCode_NotSupported;
}



int Reader::disconnect() {
  return ErrorCode_NotSupported;
}



int Reader::connectedObject() {
  return ErrorCode_NotSupported;
}



int Reader::sendApdu(uint8_t *dad,
                     uint8_t *sad,
                     uint16_t cmd_len,
                     const uint8_t *cmd,
                     uint16_t *response_len,
                     uint8_t *response) {
  return ErrorCode_NotSupported;
}




int Reader::_getNumberOfModules() {
  unsigned char dad, sad, rsp[512];
  unsigned short lr;
  unsigned char apdu[256];
  unsigned int alen;
  char ret;

  /* get module count */
  dad=CT_API_AD_DRIVER;
  sad=CT_API_AD_HOST;
  alen=0;
  apdu[alen++]=CJ_SPECIAL_CLA;
  apdu[alen++]=CJ_SPECIAL_INS_GETMODCOUNT;
  apdu[alen++]=0x00; 
  apdu[alen++]=0x00;
  lr=sizeof(rsp);
  ret=sendApdu(&dad, &sad,
	       alen, apdu,
	       &lr, rsp );
  if(ret!=CT_API_RV_OK)
    return -1;

  if (rsp[lr-2]!=0x90) {
    return -1;
  }

  return (int)rsp[0];
}



int Reader::_getModuleInfo(int idx, cj_ModuleInfo *modInfo) {
  unsigned char dad, sad, rsp[512];
  unsigned short lr;
  unsigned char apdu[256];
  unsigned int alen;
  char ret;
  unsigned int l;

  /* get module count */
  dad=CT_API_AD_DRIVER;
  sad=CT_API_AD_HOST;
  alen=0;
  apdu[alen++]=CJ_SPECIAL_CLA;
  apdu[alen++]=CJ_SPECIAL_INS_GETMODINFO;
  apdu[alen++]=(uint8_t)idx;
  apdu[alen++]=0x00;
  lr=sizeof(rsp);
  ret=sendApdu(&dad, &sad,
	       alen, apdu,
	       &lr, rsp );
  if(ret!=CT_API_RV_OK)
    return -1;
  if (rsp[lr-2]!=0x90) {
    return -1;
  }

  if (lr<3) {
    fprintf(stderr, "Too few bytes returned (%d)\n", lr);
    return -1;
  }

  lr-=2;
  l=sizeof(cj_ModuleInfo);
  if (l>lr)
    l=lr;
  memmove(modInfo, rsp, l);

  return ErrorCode_Ok;
}



int Reader::_getReaderInfo(cj_ReaderInfo *readerInfo) {
  unsigned char dad, sad, rsp[1024];
  unsigned short lr;
  unsigned char apdu[256];
  unsigned int alen;
  char ret;
  unsigned int l;

  /* get module count */
  dad=CT_API_AD_DRIVER;
  sad=CT_API_AD_HOST;
  alen=0;
  apdu[alen++]=CJ_SPECIAL_CLA;
  apdu[alen++]=CJ_SPECIAL_INS_GETREADERINFO;
  apdu[alen++]=0x00;
  apdu[alen++]=0x00;
  lr=sizeof(rsp);
  ret=sendApdu(&dad, &sad,
	       alen, apdu,
	       &lr, rsp );
  if(ret!=CT_API_RV_OK)
    return -1;
  if (rsp[lr-2]!=0x90) {
    return -1;
  }

  if (lr<3) {
    fprintf(stderr, "Too few bytes returned (%d)\n", lr);
    return -1;
  }

  lr-=2;
  l=sizeof(cj_ReaderInfo);
  if (l>lr)
    l=lr;
  memmove(readerInfo, rsp, l);

  return ErrorCode_Ok;
}



int Reader::gatherInfo(bool doConnect) {
  int rv;

  /* reset */
  m_moduleInfos.clear();
  memset(&m_readerInfo, 0, sizeof(m_readerInfo));
  m_readerInfo.SizeOfStruct=sizeof(m_readerInfo);

  if (doConnect) {
    rv=connect(Object_Reader);
    if (rv!=ErrorCode_Ok) {
      //fprintf(stderr, "Error connecting reader: %d\n", rv);
      return rv;
    }
  }

  /* get reader info */
  rv=_getReaderInfo(&m_readerInfo);
  if (rv<0) {
    //fprintf(stderr, "Error getting reader info: %d\n", rv);
    if (doConnect)
      disconnect();
    return rv;
  }

  if (getProductId()>=0x400) {
    int n;
    int i;

    /* get number of modules */
    rv=_getNumberOfModules();
    if (rv<0) {
      //fprintf(stderr, "Error getting module count: %d\n", rv);
      if (doConnect)
        disconnect();
      return rv;
    }
    n=rv;

    /* read module infos */
    for (i=0; i<n; i++) {
      cj_ModuleInfo *mi;

      m_moduleInfos.push_back(cj_ModuleInfo());
      mi=&(m_moduleInfos.back());
      mi->SizeOfStruct=sizeof(cj_ModuleInfo);
      rv=_getModuleInfo(i, mi);
      if (rv!=ErrorCode_Ok) {
        m_moduleInfos.pop_back();
        //fprintf(stderr, "Error getting module info: %d\n", rv);
        if (doConnect)
          disconnect();
        return rv;
      }
    }
  }

  if (doConnect) {
    rv=disconnect();
    if (rv!=ErrorCode_Ok) {
      //fprintf(stderr, "Error disconnecting reader: %d\n", rv);
      return rv;
    }
  }

  return ErrorCode_Ok;
}



int Reader::test(std::string &result) {
  result+="Test ist nicht implementiert\n";
  return ErrorCode_Ok;
}



int Reader::_sendData(uint8_t ins, const uint8_t *pdata, uint32_t ldata) {
  unsigned char dad, sad, rsp[512];
  unsigned short lr;
  unsigned char apdu[256];
  unsigned int alen;
  char ret;
  int first=1;

  /* send module */
  while(ldata) {
    uint32_t l;

    l=ldata;
    if (l>240)
      l=240;
    dad=CT_API_AD_DRIVER;
    sad=CT_API_AD_HOST;
    alen=0;
    apdu[alen++]=CJ_SPECIAL_CLA;
    apdu[alen++]=ins;
    if (first)
      apdu[alen++]=0x20; /* first data block */
    else
      apdu[alen++]=0x00;
    apdu[alen++]=0x00;
    apdu[alen++]=l & 0xff;
    memmove(apdu+alen, pdata, l);
    alen+=l;

    lr=sizeof(rsp);
    ret=sendApdu(&dad, &sad,
                 alen, apdu,
                 &lr, rsp );
    if(ret!=CT_API_RV_OK)
      return -1;

    if (rsp[lr-2]!=0x90)
      return ErrorCode_Generic;
    ldata-=l;
    pdata+=l;
    first=0;
  } /* while ldata */

  return ErrorCode_Ok;
}



int Reader::sendModuleToFlash(const uint8_t *pdata, uint32_t ldata) {
  return _sendData(CJ_SPECIAL_INS_UPLOADMOD, pdata, ldata);
}



int Reader::sendSignatureToFlash(const uint8_t *pdata, uint32_t ldata) {
  return _sendData(CJ_SPECIAL_INS_UPLOADSIG, pdata, ldata);
}



int Reader::flash() {
  unsigned char dad, sad, rsp[512];
  unsigned short lr;
  unsigned char apdu[256];
  unsigned int alen;
  char ret;

  /* get module count */
  dad=CT_API_AD_DRIVER;
  sad=CT_API_AD_HOST;
  alen=0;
  apdu[alen++]=CJ_SPECIAL_CLA;
  apdu[alen++]=CJ_SPECIAL_INS_UPLOADFLASH;
  apdu[alen++]=0x00; 
  apdu[alen++]=0x00;
  lr=sizeof(rsp);
  ret=sendApdu(&dad, &sad,
	       alen, apdu,
	       &lr, rsp );
  if(ret!=CT_API_RV_OK)
    return ErrorCode_IO;

  if (rsp[lr-2]!=0x90) {
    return ErrorCode_Generic;
  }

  return ErrorCode_Ok;
}



int Reader::deleteAllModules(bool doConnect) {
  unsigned char dad, sad, rsp[512];
  unsigned short lr;
  unsigned char apdu[256];
  unsigned int alen;
  char ret;
  int rv;

  if (doConnect) {
    rv=connect(Object_Reader);
    if (rv!=ErrorCode_Ok) {
      //fprintf(stderr, "Error connecting reader: %d\n", rv);
      return rv;
    }
  }

  /* get module count */
  dad=CT_API_AD_DRIVER;
  sad=CT_API_AD_HOST;
  alen=0;
  apdu[alen++]=CJ_SPECIAL_CLA;
  apdu[alen++]=CJ_SPECIAL_INS_DELETEALLMODS;
  apdu[alen++]=0x00; 
  apdu[alen++]=0x00;
  lr=sizeof(rsp);
  ret=sendApdu(&dad, &sad,
	       alen, apdu,
	       &lr, rsp );
  if(ret!=CT_API_RV_OK) {
    if (doConnect)
      disconnect();
    return ErrorCode_IO;
  }

  if (rsp[lr-2]!=0x90) {
    if (doConnect)
      disconnect();
    return ErrorCode_Generic;
  }

  /* reload module info */
  rv=gatherInfo(false);
  if (rv!=ErrorCode_Ok) {
    if (doConnect)
      disconnect();
    return rv;
  }

  if (doConnect) {
    rv=disconnect();
    if (rv!=ErrorCode_Ok) {
      //fprintf(stderr, "Error disconnecting reader: %d\n", rv);
      return rv;
    }
  }

  return ErrorCode_Ok;
}



int Reader::updateKeys(const uint8_t *pdata, uint32_t ldata) {
  unsigned char dad, sad, rsp[512];
  unsigned short lr;
  unsigned char apdu[256];
  unsigned int alen;
  char ret;
  int first=1;

  /* send module */
  while(ldata) {
    uint32_t l;
    uint8_t p1=0;

    if (first)
      p1|=0x20;                  /* first data block */
    l=ldata;
    if (l>240)
      l=240;
    if ((ldata-l)<1)
      p1|=0x80;                  /* last data block */
    dad=CT_API_AD_DRIVER;
    sad=CT_API_AD_HOST;
    alen=0;
    apdu[alen++]=CJ_SPECIAL_CLA;
    apdu[alen++]=CJ_SPECIAL_INS_KEYUPDATE;
    apdu[alen++]=p1;
    apdu[alen++]=0x00;
    apdu[alen++]=l & 0xff;
    memmove(apdu+alen, pdata, l);
    alen+=l;

    lr=sizeof(rsp);
    ret=sendApdu(&dad, &sad,
                 alen, apdu,
                 &lr, rsp );
    if(ret!=CT_API_RV_OK)
      return -1;

    if (rsp[lr-2]!=0x90)
      return ErrorCode_Generic;
    ldata-=l;
    pdata+=l;
    first=0;
  } /* while ldata */

  return ErrorCode_Ok;
}








Driver::Driver()
:m_updateCounter(0) {
}



Driver::~Driver() {
  clearReaderList();
}



int Driver::open() {
  return ErrorCode_NotSupported;
}



int Driver::enumReaders() {
  return ErrorCode_NotSupported;
}



int Driver::close() {
  return ErrorCode_NotSupported;
}



void Driver::clearReaderList() {
  std::list<Reader*>::iterator it;

  for (it=m_readerList.begin(); it!=m_readerList.end(); it++)
    delete *it;
  m_readerList.clear();
}



void Driver::removeOldReaders() {
  bool stop;

  stop=false;
  while(!stop) {
    std::list<Reader*>::iterator it;

    stop=true;
    for (it=m_readerList.begin(); it!=m_readerList.end(); it++) {
      if ((*it)->getUpdateCounter()>0 &&
          (*it)->getUpdateCounter()<m_updateCounter) {
        /* old, delete and remove from list */
        delete *it;
        m_readerList.erase(it);
        stop=false;
        break;
      }
    }
  }
}





} /* namespace */




