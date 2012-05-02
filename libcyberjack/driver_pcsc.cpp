/***************************************************************************
    begin       : Thu Mar 26 2009
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

#ifdef HAVE_PCSC


#include "driver_pcsc_p.hpp"
#include "driver_pcsc.hpp"
#include "cyberjack_l.h"
#include "Debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>
#include <unistd.h>


#define DEBUGP(ctn, format, args...) \
  rsct_log(ctn, DEBUG_MASK_CTAPI, __FILE__, __LINE__, __FUNCTION__, format, ## args)

#define DEBUGL(ctn, hdr, len, data) \
  rsct_log_bytes(ctn, DEBUG_MASK_CTAPI, __FILE__, __LINE__, __FUNCTION__, hdr, len, data)



namespace Cyberjack {

  class ReaderPcsc: public Reader {
  public:

    ReaderPcsc(Driver *driver,
	       const std::string &name);
    virtual ~ReaderPcsc();

    virtual int connect(int object);
    virtual int disconnect();
    virtual int connectedObject();

    virtual int sendApdu(uint8_t *dad,
                         uint8_t *sad,
                         uint16_t cmd_len,
                         const uint8_t *cmd,
                         uint16_t *response_len,
                         uint8_t *response);

    virtual int test(std::string &result);

  public:
    int m_connectedObject;
    SCARDHANDLE m_card;
    DWORD m_verify_ctrl;
    DWORD m_modify_ctrl;
    DWORD m_universal_ctrl;

  };


  class DriverPcsc: public Driver {
  public:
    DriverPcsc();
    virtual ~DriverPcsc();

    virtual int open();
    virtual int enumReaders();
    virtual int close();

    SCARDCONTEXT m_context;

  };





  ReaderPcsc::ReaderPcsc(Driver *driver,
			 const std::string &name)
  : Reader(driver, name, "", "", BusType_Pcsc, 0, 0)
  , m_connectedObject(Object_None) {
  }



  ReaderPcsc::~ReaderPcsc() {
  }


  int ReaderPcsc::connect(int object) {
    std::string devName;
    DriverPcsc *d=dynamic_cast<DriverPcsc*>(m_driver);

    if (m_connectedObject==object)
      return ErrorCode_Ok;

    if (object!=Object_None) {
      LONG rv;
      DWORD length;
      DWORD i;
      DWORD dwActiveProtocol;
      unsigned char bRecvBuffer[300];
      PCSC_TLV_STRUCTURE *tlv;

      /* connect reader */
      dwActiveProtocol=-1;
      if (object==Object_Reader)
        rv=SCardConnect(d->m_context, m_name.c_str(), SCARD_SHARE_DIRECT,
                        SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &m_card, &dwActiveProtocol);
      else
        rv=SCardConnect(d->m_context, m_name.c_str(), SCARD_SHARE_EXCLUSIVE,
                        SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &m_card, &dwActiveProtocol);
      if (rv!=SCARD_S_SUCCESS) {
        fprintf(stderr, "SCardConnect: %s (%lX)\n",
                pcsc_stringify_error(rv), rv);
        return ErrorCode_IO;
      }

      /* get control codes */
      m_verify_ctrl=0;
      m_modify_ctrl=0;
      m_universal_ctrl=0;

      rv=SCardControl(m_card, CM_IOCTL_GET_FEATURE_REQUEST, NULL, 0,
                      bRecvBuffer, sizeof(bRecvBuffer), &length);
      if (rv!=SCARD_S_SUCCESS) {
        fprintf(stderr, "SCardControl: %s (%lX)\n",
                pcsc_stringify_error(rv), rv);
        SCardDisconnect(m_card, SCARD_UNPOWER_CARD);
        return ErrorCode_IO;
      }

      if (length % sizeof(PCSC_TLV_STRUCTURE)) {
        fprintf(stderr, "Inconsistent result! Bad TLV values!\n");
        SCardDisconnect(m_card, SCARD_UNPOWER_CARD);
        return ErrorCode_IO;
      }

      length /= sizeof(PCSC_TLV_STRUCTURE);
      tlv = (PCSC_TLV_STRUCTURE*) bRecvBuffer;
      for (i=0; i<length; i++) {
        if (tlv[i].tag == FEATURE_VERIFY_PIN_DIRECT)
          m_verify_ctrl = ntohl(tlv[i].value);
        if (tlv[i].tag == FEATURE_MODIFY_PIN_DIRECT)
          m_modify_ctrl = ntohl(tlv[i].value);
        if (tlv[i].tag == FEATURE_MCT_UNIVERSAL)
          m_universal_ctrl=ntohl(tlv[i].value);
      }
      if (m_universal_ctrl==0) {
	fprintf(stderr, "Driver for [%s] doesn't know FEATURE_MKT_UNIVERSAL\n",
		m_name.c_str());
        SCardDisconnect(m_card, SCARD_UNPOWER_CARD);
        return ErrorCode_IO;
      }
    }

    m_connectedObject=object;
    return ErrorCode_Ok;
  }



  int ReaderPcsc::disconnect() {
    if (m_connectedObject!=Object_None) {
      LONG rv;

      m_connectedObject=Object_None;
      rv=SCardDisconnect(m_card, SCARD_UNPOWER_CARD);
      if (rv!=SCARD_S_SUCCESS) {
        fprintf(stderr, "SCardDisconnect: %s (%lX)\n",
                pcsc_stringify_error(rv), rv);
        return ErrorCode_IO;
      }
    }

    return ErrorCode_Ok;
  }



  int ReaderPcsc::connectedObject() {
    return m_connectedObject;
  }



  int ReaderPcsc::sendApdu(uint8_t *dad,
                            uint8_t *sad,
                            uint16_t cmd_len,
                            const uint8_t *cmd,
                            uint16_t *response_len,
                            uint8_t *response) {
    if (m_connectedObject!=Object_None) {
      uint8_t *wbuf;
      MCTUniversal_t *usent;
      MCTUniversal_t *urecvd;
      DWORD rlength;
      DWORD wlength;
      LONG rv;

      wlength=sizeof(MCTUniversal_t)+cmd_len-1;
      wbuf=(uint8_t*) malloc(wlength);
      if (wbuf==NULL) {
        /* error */
      }
      memset(wbuf, 0, wlength);
      usent=(MCTUniversal_t*)wbuf;
      usent->SAD=*sad;
      usent->DAD=*dad;
      usent->BufferLength=cmd_len;
      memmove(&usent->buffer, cmd, cmd_len);

      rlength=(*response_len)-sizeof(MCTUniversal_t);
      urecvd=(MCTUniversal_t*)response;

      rv=SCardControl(m_card, m_universal_ctrl, wbuf, wlength,
                      response, rlength, &rlength);
      free(wbuf);
      if (rv!=SCARD_S_SUCCESS) {
        fprintf(stderr, "SCardControl: %s (%lX)\n",
                pcsc_stringify_error(rv), rv);
        return ErrorCode_IO;
      }
      *sad=urecvd->SAD;
      *dad=urecvd->DAD;
      rlength=urecvd->BufferLength;
      memmove(response, &urecvd->buffer, rlength);
      *response_len=rlength;

      return ErrorCode_Ok;
    }
    else {
      DEBUGP(CT_INVALID_CTN, "Device not connected");
      return ErrorCode_Invalid;
    }
  }


  int ReaderPcsc::test(std::string &result) {
    return ErrorCode_Ok;
  }







  DriverPcsc::DriverPcsc()
  : Driver() {
  }



  DriverPcsc::~DriverPcsc() {
  }


  int DriverPcsc::open() {
    LONG rv;
    SCARDCONTEXT hContext;

    rv=SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
    if (rv!=SCARD_S_SUCCESS){
      if (rv==SCARD_E_NO_SERVICE) {
	//fprintf(stderr, "No SCard service (pcscd not running)\n");
	return ErrorCode_NoService;
      }
      fprintf(stderr, "SCardEstablishContext: Cannot Connect to Resource Manager: %s (%lX)\n",
	      pcsc_stringify_error(rv), rv);
      return ErrorCode_IO;
    }
    m_context=hContext;

    return ErrorCode_Ok;
  }



  int DriverPcsc::close() {
    LONG rv;

    rv=SCardReleaseContext(m_context);
    if (rv!=SCARD_S_SUCCESS)
      fprintf(stderr, "SCardReleaseContext: %s (0x%lX)\n",
	      pcsc_stringify_error(rv), rv);

    return ErrorCode_Ok;
  }


  static ReaderPcsc *findReader(const char *name,
				std::list<Reader*> &rl) {
    std::list<Reader*>::iterator it;

    for (it=rl.begin(); it!=rl.end(); it++) {
      ReaderPcsc *r=dynamic_cast<ReaderPcsc*>(*it);
      if (r) {
        if (strcasecmp(r->getName().c_str(), name)==0)
	  return r;
      }
    }

    return NULL;
  }



  int DriverPcsc::enumReaders() {
    LONG rv;
    DWORD dwReaders;
    LPSTR mszReaders;
    char *ptr;
    std::list<Reader*>::iterator rit;

    /* set previous update counter for all devices which have a counter of 0.
     * this means all deviced added in the last round now get a real counter value */
    for (rit=m_readerList.begin(); rit!=m_readerList.end(); rit++) {
      Reader *r=*rit;
      if (r->getUpdateCounter()==0)
        r->setUpdateCounter(m_updateCounter);
    }

    /* start next round */
    m_updateCounter++;

    /* Retrieve the available readers list */
    rv=SCardListReaders(m_context, NULL, NULL, &dwReaders);
    if (rv!=SCARD_S_SUCCESS) {
      fprintf(stderr, "SCardListReader: %s (%lX)\n",
	      pcsc_stringify_error(rv), rv);
      return ErrorCode_IO;
    }

    mszReaders=(LPSTR) malloc(sizeof(char)*dwReaders);
    if (mszReaders==NULL){
      fprintf(stderr, "malloc: not enough memory\n");
      return ErrorCode_IO;
    }

    rv=SCardListReaders(m_context, NULL, mszReaders, &dwReaders);
    if (rv!=SCARD_S_SUCCESS)
      fprintf(stderr, "SCardListReader: %s (%lX)\n",
	      pcsc_stringify_error(rv), rv);

    /* create readers */
    ptr=mszReaders;
    while (*ptr != '\0'){
      //fprintf(stderr, "Found device: %s\n", ptr);
      if (strncasecmp(ptr, "REINER SCT", 10)==0) {
        ReaderPcsc *r;

        r=findReader(ptr, m_readerList);
        if (r) {
          //fprintf(stderr, "Found reader\n");
          r->setUpdateCounter(m_updateCounter);
        }
        else {
          int res;

          /* create new reader */
          r=new ReaderPcsc(this, ptr);
          r->setVendorId(0x0c4b);
          res=r->gatherInfo(true);
          if (res!=ErrorCode_Ok) {
            //fprintf(stderr, "Unable to connect reader\n");
          }
          else {
            const cj_ReaderInfo &ri=r->getReaderInfo();
            if (ri.ContentsMask & RSCT_READER_MASK_PID) {
              r->setProductId(ri.PID);
            }
            if (ri.ContentsMask & RSCT_READER_MASK_SERIALNUMBER) {
              r->setSerialNumber(std::string((const char*)ri.SeriaNumber));
            }
            if (ri.ContentsMask & RSCT_READER_MASK_PRODUCT_STRING) {
              r->setProductString(std::string((const char*)ri.ProductString));
            }
          }

          r->setUpdateCounter(0); /* 0 means "just created" */
          m_readerList.push_back(r);
        }
      }
      ptr+=strlen(ptr)+1;
    }

    return 0;
  }




  Driver *NewDriverPcsc() {
    return new DriverPcsc();
  }











} /* namespace */



#endif

