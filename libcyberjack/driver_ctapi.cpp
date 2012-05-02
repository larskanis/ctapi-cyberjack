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

#ifdef ENABLE_NONSERIAL

/*#define USE_KERNEL_DRIVER_FOR_0X100*/

#include "driver_ctapi.hpp"
#include "cyberjack_l.h"
#include "Debug.h"

#include "usbdev_l.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>
#include <unistd.h>

#include <sys/types.h>
#include <grp.h>
#include <pwd.h>


#define DEBUGP(ctn, format, args...) \
  rsct_log(ctn, DEBUG_MASK_CTAPI, __FILE__, __LINE__, __FUNCTION__, format, ## args)

#define DEBUGL(ctn, hdr, len, data) \
  rsct_log_bytes(ctn, DEBUG_MASK_CTAPI, __FILE__, __LINE__, __FUNCTION__, hdr, len, data)



namespace Cyberjack {

  class ReaderCtapi: public Reader {
  public:

    ReaderCtapi(Driver *driver,
                const std::string &name,
                const std::string &productString,
                const std::string &serial,
                int busType,
                uint32_t vendorId,
                uint32_t productId);
    virtual ~ReaderCtapi();

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
    int m_ctn;
    uint32_t m_devicePos;
    uint32_t m_busId;
    uint32_t m_deviceId;
    std::string m_path;
    std::string m_busName;
    std::string m_deviceName;
    std::string m_halPath;
  };



  ReaderCtapi::ReaderCtapi(Driver *driver,
                           const std::string &name,
                           const std::string &productString,
                           const std::string &serial,
                           int busType,
                           uint32_t vendorId,
                           uint32_t productId)
  : Reader(driver, name, productString, serial, busType, vendorId, productId)
  , m_connectedObject(Object_None)
  , m_ctn(-1)
  , m_devicePos(0)
  , m_busId(0)
  , m_deviceId(0) {
  }



  ReaderCtapi::~ReaderCtapi() {
  }



  int ReaderCtapi::connect(int object) {
    int8_t rv;
    std::string devName;

    if (m_connectedObject==object)
      return ErrorCode_Ok;

    if (m_ctn==-1)
      m_ctn=m_devicePos+1;
    if (m_busType==BusType_UsbRaw) {
      char nbuffer[256];

      if (!m_halPath.empty()) {
	devName=m_halPath;
      }
      else {
        snprintf(nbuffer, sizeof(nbuffer)-1, "usb:%04x/%04x:libusb:%03d:%03d",
                 m_vendorId, m_productId, m_busId, m_deviceId);
        nbuffer[sizeof(nbuffer)-1]=0;
        devName=std::string(nbuffer);
      }
    }
    else
      devName=std::string(m_path);

#if USE_KERNEL_DRIVER_FOR_0X100
    if (m_productId==0x100) {
      rv=CT_init(m_ctn, 0x8000+m_deviceId);
      if (rv!=0) {
	DEBUGP(CT_INVALID_CTN, "Unable to init device [%s] at %04x: %d\n",
	       devName.c_str(), 0x8000+m_deviceId, rv);
        return ErrorCode_IO;
      }
    }
    else {
#endif
      rv=rsct_init_name(m_ctn, devName.c_str());
      if (rv!=0) {
        DEBUGP(CT_INVALID_CTN, "Unable to init device [%s]: %d\n",
               devName.c_str(), rv);
        return ErrorCode_IO;
      }
#if USE_KERNEL_DRIVER_FOR_0X100
    }
#endif
    if (object!=Object_Reader) {
      /* connect something else, do it */
    }

    m_connectedObject=object;
    return ErrorCode_Ok;
  }



  int ReaderCtapi::disconnect() {
    if (m_connectedObject!=Object_None) {
      int8_t rv;

      m_connectedObject=Object_None;
      rv=CT_close(m_ctn);
      if (rv!=0) {
        DEBUGP(CT_INVALID_CTN, "Unable to close device %d: %d",
               m_ctn, rv);
        return ErrorCode_IO;
      }
    }

    return ErrorCode_Ok;
  }



  int ReaderCtapi::connectedObject() {
    return m_connectedObject;
  }



  int ReaderCtapi::sendApdu(uint8_t *dad,
                            uint8_t *sad,
                            uint16_t cmd_len,
                            const uint8_t *cmd,
                            uint16_t *response_len,
                            uint8_t *response) {
    if (m_connectedObject!=Object_None) {
      int8_t rv;

      rv=CT_data(m_ctn, dad, sad, cmd_len, cmd, response_len, response);
      if (rv<0) {
        DEBUGP(m_ctn, "Error on CT_data: %d", rv);
        switch(rv) {
        case CT_API_RV_ERR_TRANS:
          return ErrorCode_IO;

        case CT_API_RV_ERR_INVALID:
          return ErrorCode_Invalid;

        case CT_API_RV_ERR_CT:
        case CT_API_RV_ERR_MEMORY:
        case CT_API_RV_ERR_HOST:
        case CT_API_RV_ERR_HTSI:
        default:
          return ErrorCode_Generic;
        }
      }

      return ErrorCode_Ok;
    }
    else {
      DEBUGP(CT_INVALID_CTN, "Device not connected");
      return ErrorCode_Invalid;
    }
  }


  int ReaderCtapi::test(std::string &result) {
    int errorCode;

    errorCode=Cyberjack::ErrorCode_Ok;

    result+="- Geraetedatei ist: ";
    result+=m_path+"\n";
    if (access(m_path.c_str(), F_OK)==0) {
      struct stat st;

      result+="- die Geraetedate existiert\n";
      if (stat(m_path.c_str(), &st)) {
        result+="- konnte stat() nicht ausfuehren (";
        result+=strerror(errno);
        result+=")\n";
        errorCode=ErrorCode_IO;
      }
      else {
        char tmpbuf[256];
        struct passwd *pw=NULL;
        struct group *gr;

        /* get owner and group of the file */
        snprintf(tmpbuf, sizeof(tmpbuf)-1, "Rechte=%o, Besitzer=%d, Gruppe=%d",
                 (st.st_mode & 0777), st.st_uid, st.st_gid);
        tmpbuf[sizeof(tmpbuf)-1]=0;
        result+="- Dateirechte: ";
        result+=tmpbuf;
        result+="\n";

        pw=getpwuid(st.st_uid);
        if (pw==NULL) {
          result+="- Fehler bei getpwuid() (";
          result+=strerror(errno);
          result+=")\n";
          errorCode=ErrorCode_IO;
        }
        else {
          result+="- Dateibesitzer: ";
          result+=pw->pw_name;
          result+="\n";
        }

        gr=getgrgid(st.st_gid);
        if (gr==NULL) {
          result+="- Fehler bei getgrgid() (";
          result+=strerror(errno);
          result+=")\n";
          errorCode=ErrorCode_IO;
        }
        else {
          result+="- Dateigruppe: ";
          result+=gr->gr_name;
          result+="\n";
        }

        if (access(m_path.c_str(), R_OK | W_OK)==0) {
          result+="- der ausfuehrende Benutzer hat alle noetigen Rechte\n";
        }
        else {
          if (access(m_path.c_str(), R_OK)==0) {
            result+="- der ausfuehrende Benutzer hat Leserechte\n";
          }
          else {
            result+="- der ausfuehrende Benutzer hat keine Leserechte (";
            result+=strerror(errno);
            result+=")\n";
            errorCode=ErrorCode_IO;
          }
          if (access(m_path.c_str(), W_OK)==0) {
            result+="- der ausfuehrende Benutzer hat Schreibrechte\n";
          }
          else {
            result+="- der ausfuehrende Benutzer hat keine Schreibrechte (";
            result+=strerror(errno);
            result+=")\n";
            errorCode=ErrorCode_IO;
          }
        }
      }
    }
    else {
      result+="- die Geraetedate existiert nicht (";
      result+=strerror(errno);
      result+=")\n";
      errorCode=ErrorCode_IO;
    }

    return errorCode;
  }







  DriverCtapi::DriverCtapi()
  : Driver() {
  }



  DriverCtapi::~DriverCtapi() {
  }


  int DriverCtapi::open() {
    return ErrorCode_Ok;
  }



  int DriverCtapi::close() {
    return ErrorCode_Ok;
  }


  static ReaderCtapi *findReader(int busType,
                                 uint32_t busId,
                                 uint32_t deviceId,
                                 std::list<Reader*> &rl) {
    std::list<Reader*>::iterator it;

    for (it=rl.begin(); it!=rl.end(); it++) {
      ReaderCtapi *r=dynamic_cast<ReaderCtapi*>(*it);
      if (r) {
        if (r->getBusType()==busType && r->m_busId==busId && r->m_deviceId==deviceId)
          return r;
      }
    }

    return NULL;
  }



  int DriverCtapi::enumReaders() {
    int count=0;
    std::list<Reader*>::iterator rit;
    rsct_usbdev_t *devList=NULL;
    rsct_usbdev_t *dev;
    int rv;

    /* set previous update counter for all devices which have a counter of 0.
     * this means all devices added in the last round now get a real counter value */
    for (rit=m_readerList.begin(); rit!=m_readerList.end(); rit++) {
      Reader *r=*rit;
      if (r->getUpdateCounter()==0)
	r->setUpdateCounter(m_updateCounter);
    }

    /* start next round */
    m_updateCounter++;

    rv=rsct_usbdev_scan(&devList);
    if (rv) {
      DEBUGP(CT_INVALID_CTN, "Could not scan devices");
      return ErrorCode_IO;
    }

    dev=devList;
    while(dev) {
      ReaderCtapi *r;

      r=findReader(BusType_UsbRaw, dev->busId, dev->busPos, m_readerList);
      if (r) {
	r->setUpdateCounter(m_updateCounter);
      }
      else {
	char pbuff[256];
	std::string rname;

	/* determine reader name */
	snprintf(pbuff, sizeof(pbuff),
		 "%s an %03d:%03d",
		 (dev->productName[0]!=0)?(dev->productName):"Leser",
		 dev->busId, dev->busPos);
	rname=std::string(pbuff);

	/* create reader */
	r=new ReaderCtapi(this,
			  rname,
			  std::string(dev->productName),
			  std::string(dev->serial),
			  BusType_UsbRaw,
			  dev->vendorId,
			  dev->productId);
	r->m_devicePos=count++;
	r->m_busId=dev->busId;
	r->m_deviceId=dev->busPos;
	r->m_path=std::string(dev->deviceNodePath);
	r->m_halPath=std::string(dev->halPath);
	r->setUpdateCounter(0); /* 0 means "just created" */

	/* add reader */
	m_readerList.push_back(r);
      } /* if reader new */

      dev=dev->next;
    }

    return 0;
  }












} /* namespace */



#endif

