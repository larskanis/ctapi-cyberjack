/***************************************************************************
 * project      : Cyberjack Diagnoses Tool
    begin       : Fri Jan 26 2007
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *             This file is licensed under the GPL version 2.              *
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "cm_reader.h"
#include "checksuite.h"
#include "driver_pcsc.hpp"
#include "driver_ctapi.hpp"


#include "cyberjack_l.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>



#define CYBERJACK_VENDOR_ID 0xc4b



bool CM_Reader::_checkReaders(Cyberjack::Driver *dr,
			      std::string &xmlString,
			      std::string &reportString,
			      std::string &hintString) {
  int rv;
  std::list<Cyberjack::Reader*>::const_iterator it;
  char numbuf[256];

  rv=dr->open();
  if (rv<0) {
    if (rv==Cyberjack::ErrorCode_NoService) {
      xmlString+="    <result type=\"undetermined\">\n";
      xmlString+=       "Service is not available\n";
      xmlString+="    </result>\n";

      reportString+="Dienst ist nicht verfuegbar.\n";

      return true;
    }
    fprintf(stderr, "Error in open: %d\n", rv);
    return false;
  }

  rv=dr->enumReaders();
  if (rv<0) {
    fprintf(stderr, "Error in enumReaders: %d\n", rv);
    dr->close();
    return false;
  }

  xmlString+="    <result type=\"success\">\n";

  for (it=dr->getReaders().begin(); it!=dr->getReaders().end(); it++) {
    int rv;
    std::string ts;

    reportString+="Leser ";
    reportString+=(*it)->getName();
    reportString+=" (vendorid=\"";
    snprintf(numbuf, sizeof(numbuf)-1, "%04x", (*it)->getVendorId());
    reportString+=numbuf;
    reportString+="\", productid=\"";
    snprintf(numbuf, sizeof(numbuf)-1, "%04x", (*it)->getProductId());
    reportString+=numbuf;
    reportString+="\")\n";

    xmlString+="    <reader name=\"";
    xmlString+=(*it)->getName();
    xmlString+="\" vendorid=\"";
    snprintf(numbuf, sizeof(numbuf)-1, "%04x", (*it)->getVendorId());
    xmlString+=numbuf;
    xmlString+="\" productid=\"";
    snprintf(numbuf, sizeof(numbuf)-1, "%04x", (*it)->getProductId());
    xmlString+=numbuf;
    xmlString+="\">\n";

    if ((*it)->test(ts)!=Cyberjack::ErrorCode_Ok)
      xmlString+="      <readertest result=\"error\">\n";
    else
      xmlString+="      <readertest result=\"success\">\n";
    reportString+="Ergebnis des Lesertests:\n";
    reportString+=ts;
    xmlString+=ts;
    xmlString+="      </readertest>\n";

    rv=(*it)->gatherInfo(true);
    if (rv!=Cyberjack::ErrorCode_Ok) {
      //fprintf(stderr, "Error gathering info: %d\n", rv);
      dr->close();
      return false;
    }

    if (1) {
      const cj_ReaderInfo &ri=(*it)->getReaderInfo();

      xmlString+="      <readerinfo>\n";

      if (ri.ContentsMask & RSCT_READER_MASK_PID) {
	snprintf(numbuf, sizeof(numbuf)-1, "%04x", ri.PID);
	reportString+="  PID        : ";
	reportString+=numbuf;
	reportString+="\n";

	xmlString+="        <pid>";
	xmlString+=numbuf;
	xmlString+="</pid>";
	xmlString+="\n";
      }

      if (ri.ContentsMask & RSCT_READER_MASK_HARDWARE) {
	snprintf(numbuf, sizeof(numbuf)-1, "%08x", ri.HardwareMask);
	reportString+="  HW-Mask    : ";
	reportString+=numbuf;
	reportString+=" (";

	xmlString+="        <hwmask>";
	xmlString+=numbuf;
	xmlString+="</hwmask>";
	xmlString+="\n";

	xmlString+="        <hwmask value=\"";
	xmlString+=numbuf;
	xmlString+="\">";


	if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC1) {
          reportString+=" ICC1";
	  xmlString+=" ICC1";
	}
	if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC2) {
          reportString+=" ICC2";
	  xmlString+=" ICC2";
	}
	if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC3) {
          reportString+=" ICC3";
	  xmlString+=" ICC3";
	}
	if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC4) {
          reportString+=" ICC4";
	  xmlString+=" ICC4";
	}
	if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC5) {
          reportString+=" ICC5";
	  xmlString+=" ICC5";
	}
	if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC6) {
          reportString+=" ICC6";
	  xmlString+=" ICC6";
	}
	if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC7) {
          reportString+=" ICC7";
	  xmlString+=" ICC7";
	}
	if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC8) {
          reportString+=" ICC8";
	  xmlString+=" ICC8";
	}
	if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_KEYPAD) {
          reportString+=" keypad";
	  xmlString+=" keypad";
	}
	if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_DISPLAY) {
	  reportString+=" display";
	  xmlString+=" display";
	}
	if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_BIOMETRIC) {
	  reportString+=" Fingersensor";
	  xmlString+=" Fingersensor";
	}
	if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_UPDATEABLE) {
	  reportString+=" Firmwareupdate";
	  xmlString+=" Firmwareupdate";
	}
	if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_MODULES) {
	  reportString+=" Sicherheitsmodule";
	  xmlString+=" Sicherheitsmodule";
	}

	reportString+=")\n";
	xmlString+="</hwmask>\n";
      }
  
      if (ri.ContentsMask & RSCT_READER_MASK_VERSION) {
	snprintf(numbuf, sizeof(numbuf)-1, "%x", ri.Version);
	reportString+="  Version    : ";
	reportString+=numbuf;
	reportString+="\n";

	xmlString+="        <version>";
	xmlString+=numbuf;
	xmlString+="</version>";
	xmlString+="\n";

      }
  
      if (ri.ContentsMask & RSCT_READER_MASK_HARDWARE_VERSION) {
	snprintf(numbuf, sizeof(numbuf)-1, "%x", ri.HardwareVersion);
	reportString+="  HW-Version : ";
	reportString+=numbuf;
	reportString+="\n";

	xmlString+="        <hwversion>";
	xmlString+=numbuf;
	xmlString+="</hwversion>";
	xmlString+="\n";
      }


      if (ri.ContentsMask & RSCT_READER_MASK_FLASH_SIZE) {
	snprintf(numbuf, sizeof(numbuf)-1, "%d", ri.FlashSize);
	reportString+="  Flashsize  : ";
	reportString+=numbuf;
	reportString+="\n";

	xmlString+="        <flashsize>";
	xmlString+=numbuf;
	xmlString+="</flashsize>";
	xmlString+="\n";
      }
  
      if (ri.ContentsMask & RSCT_READER_MASK_HEAP_SIZE) {
	snprintf(numbuf, sizeof(numbuf)-1, "%d", ri.HeapSize);
	reportString+="  Heapsize   : ";
	reportString+=numbuf;
	reportString+="\n";

	xmlString+="        <heapsize>";
	xmlString+=numbuf;
	xmlString+="</heapsize>";
	xmlString+="\n";
      }

      if (ri.ContentsMask & RSCT_READER_MASK_SERIALNUMBER) {
	reportString+="  Serialnum  : ";
	reportString+=(const char*)(ri.SeriaNumber);
	reportString+="\n";

	xmlString+="          <serialnum>";
	xmlString+=(const char*)(ri.SeriaNumber);
	xmlString+="</serialnum>";
	xmlString+="\n";
      }
  
      if (ri.ContentsMask & RSCT_READER_MASK_VENDOR_STRING) {
	reportString+="  Vendor     : ";
	reportString+=(const char*)(ri.VendorString);
	reportString+="\n";

	xmlString+="        <vendorstring>";
	xmlString+=(const char*)(ri.VendorString);
	xmlString+="</vendorstring>";
	xmlString+="\n";
      }
  
  
      if (ri.ContentsMask & RSCT_READER_MASK_PRODUCT_STRING) {
	reportString+="  Product    : ";
	reportString+=(const char*)(ri.ProductString);
	reportString+="\n";

	xmlString+="        <productstring>";
	xmlString+=(const char*)(ri.ProductString);
	xmlString+="</productstring>";
	xmlString+="\n";
      }
  
      if (ri.ContentsMask & RSCT_READER_MASK_PRODUCTION_DATE) {
	snprintf(numbuf, sizeof(numbuf)-1, "%s %s", ri.ProductionDate, ri.ProductionTime);
	reportString+="  P-Date     : ";
	reportString+=numbuf;
	reportString+="\n";

	xmlString+="        <productiondate>";
	xmlString+=numbuf;
	xmlString+="</productiondate>";
	xmlString+="\n";
      }
  
  
      if (ri.ContentsMask & RSCT_READER_MASK_TEST_DATE) {
	snprintf(numbuf, sizeof(numbuf)-1, "%s %s", ri.TestDate, ri.TestTime);
	reportString+="  T-Date     : ";
	reportString+=numbuf;
	reportString+="\n";

	xmlString+="        <testdate>";
	xmlString+=numbuf;
	xmlString+="</testdate>";
	xmlString+="\n";
      }
  
      if (ri.ContentsMask & RSCT_READER_MASK_COMMISSIONING_DATE) {
	snprintf(numbuf, sizeof(numbuf)-1, "%s %s", ri.CommissioningDate, ri.CommissioningTime);
	reportString+="  C-Date     : ";
	reportString+=numbuf;
	reportString+="\n";

	xmlString+="        <Commissioningdate>";
	xmlString+=numbuf;
	xmlString+="</Commissioningdate>";
	xmlString+="\n";
      }

      if (ri.ContentsMask & RSCT_READER_MASK_COM_TYPE) {
	reportString+="  COM-Type   : ";
	reportString+=(const char*)(ri.CommunicationString);
	reportString+="\n";

	xmlString+="        <communicationstring>";
	xmlString+=(const char*)(ri.CommunicationString);
	xmlString+="</communicationstring>";
	xmlString+="\n";
      }
  
      if (ri.ContentsMask & RSCT_READER_MASK_PORT_ID) {
	snprintf(numbuf, sizeof(numbuf)-1, "%d", ri.PortID);
	reportString+="  PortID     : ";
	reportString+=numbuf;
	reportString+="\n";

	xmlString+="        <portid>";
	xmlString+=numbuf;
	xmlString+="</portid>";
	xmlString+="\n";
      }
  
      if (ri.ContentsMask & RSCT_READER_MASK_IFD_BRIDGE) {
	reportString+="  IFD-Bridge : ";
	reportString+=(const char*)(ri.IFDNameOfIfdBridgeDevice);
	reportString+="\n";

	xmlString+="        <ifdbridge>";
	xmlString+=(const char*)(ri.IFDNameOfIfdBridgeDevice);
	xmlString+="</ifdbridge>";
	xmlString+="\n";
      }

      if (ri.ContentsMask & RSCT_READER_MASK_HW_STRING) {
	reportString+="  HW-String  : ";
	reportString+=(const char*)(ri.HardwareString);
	reportString+="\n";

	xmlString+="        <hwstring>";
	xmlString+=(const char*)(ri.HardwareString);
	xmlString+="</hwstring>";
	xmlString+="\n";
      }
      xmlString+="      </readerinfo>\n";
    } /* if newer reader */

    if ((*it)->getProductId()>=0x400) {
      const std::list<cj_ModuleInfo> &modules=(*it)->getModuleInfos();
      std::list<cj_ModuleInfo>::const_iterator mit;
      int i;

      xmlString+="      <fwmodules>\n";
      for (mit=modules.begin(), i=0; mit!=modules.end(); mit++, i++) {
        const cj_ModuleInfo &mi=*mit;

	reportString+="Module ";
	snprintf(numbuf, sizeof(numbuf)-1, "%d ", i);
	reportString+=numbuf;
	reportString+=":\n";

	xmlString+="        <fwmodule>\n";

	if (mi.ContentsMask & RSCT_MODULE_MASK_DESCRIPTION) {
	  reportString+="  Description: ";
	  reportString+=(const char*)(mi.Description);
	  reportString+="\n";

	  xmlString+="          <description>";
	  xmlString+=(const char*)(mi.Description);
	  xmlString+="</description>";
	  xmlString+="\n";
	}

	if (mi.ContentsMask & RSCT_MODULE_MASK_ID) {
	  snprintf(numbuf, sizeof(numbuf)-1, "%08x", mi.ID);
	  reportString+="  Id         : ";
	  reportString+=numbuf;
	  reportString+="\n";

          xmlString+="          <id>";
	  xmlString+=numbuf;
	  xmlString+="</id>";
	  xmlString+="\n";
	}

	if (mi.ContentsMask & RSCT_MODULE_MASK_VERSION) {
	  snprintf(numbuf, sizeof(numbuf)-1, "%x", mi.Version);
	  reportString+="  Version    : ";
	  reportString+=numbuf;
	  reportString+="\n";

	  xmlString+="          <version>";
	  xmlString+=numbuf;
	  xmlString+="</version>";
	  xmlString+="\n";
	}

	if (mi.ContentsMask & RSCT_MODULE_MASK_REVISION) {
	  snprintf(numbuf, sizeof(numbuf)-1, "%d", mi.Revision);
	  reportString+="  Revision   : ";
	  reportString+=numbuf;
	  reportString+="\n";

	  xmlString+="          <revision>";
	  xmlString+=numbuf;
	  xmlString+="</revision>";
	  xmlString+="\n";
	}

	if (mi.ContentsMask & RSCT_MODULE_MASK_VARIANT) {
	  snprintf(numbuf, sizeof(numbuf)-1, "%d", mi.Variant);
	  reportString+="  Variant    : ";
	  reportString+=numbuf;
	  reportString+="\n";

	  xmlString+="          <variant>";
	  xmlString+=numbuf;
	  xmlString+="</variant>";
	  xmlString+="\n";
	}

	if (mi.ContentsMask & RSCT_MODULE_MASK_DATE) {
	  snprintf(numbuf, sizeof(numbuf)-1, "%12s %6s", mi.Date, mi.Time);
	  reportString+="  Date       : ";
	  reportString+=numbuf;
	  reportString+="\n";

	  xmlString+="          <date>";
	  xmlString+=numbuf;
	  xmlString+="</date>";
	  xmlString+="\n";
	}

	xmlString+="        </fwmodule>\n";
      }
      xmlString+="      </fwmodules>\n";
    } /* if newer reader */
    xmlString+="    </reader>";
  } /* for every reader */

  xmlString+="    </result>\n";

  dr->close();

  return true;
}



bool CM_Reader::_checkPcsc(std::string &xmlString,
			   std::string &reportString,
			   std::string &hintString) {
#ifdef HAVE_PCSC
  Cyberjack::Driver *dr=Cyberjack::NewDriverPcsc();
  bool b;

  reportString+="PC/SC Interface\n";
  xmlString+="<api type=\"pcsc\">\n";
  b=_checkReaders(dr, xmlString, reportString, hintString);
  if (!b)
    reportString+="  PC/SC-Interface nicht verfuegbar.\n";
  xmlString+="</api>\n";

  delete dr;
  return b;
#else
  reportString+="PC/SC Interface\n";
  reportString+="  PC/SC-Interface nicht getestet (keine Unterstuetzung eincompiliert).\n";
  xmlString+="<api type=\"pcsc\">\n";
  xmlString+="  <result type=\"untested\">\n";
  xmlString+="  </result>\n";
  xmlString+="</api>\n";
  return true;
#endif
}



bool CM_Reader::_checkCtapi(std::string &xmlString,
			    std::string &reportString,
			    std::string &hintString) {
  Cyberjack::Driver *dr=new Cyberjack::DriverCtapi();
  bool b;

  reportString+="CTAPI Interface\n";
  xmlString+="<api type=\"ctapi\">\n";
  b=_checkReaders(dr, xmlString, reportString, hintString);
  if (!b)
    reportString+="  CTAPI-Interface nicht verfuegbar.\n";
  xmlString+="</api>\n";

  delete dr;
  return b;
}



bool CM_Reader::check(std::string &xmlString,
		      std::string &reportString,
		      std::string &hintString) {
  bool b1;
  bool b2;

  /* test pcsc first, then CTAPI */
  b1=_checkPcsc(xmlString, reportString, hintString);
  b2=_checkCtapi(xmlString, reportString, hintString);

  if (b1 || b2)
    return true;
  return false;
}



