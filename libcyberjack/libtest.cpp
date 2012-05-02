
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "driver_ctapi.hpp"
#ifdef HAVE_PCSC
# include "driver_pcsc.hpp"
#endif
#include "cyberjack_l.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


using namespace Cyberjack;




int getNumberOfModules(Reader *r) {
  unsigned char dad, sad, rsp[512];
  unsigned short lr;
  unsigned char apdu[256];
  unsigned int alen;
  char ret;
  //int i;

  /* get module count */
  dad=CT_API_AD_DRIVER;
  sad=CT_API_AD_HOST;
  alen=0;
  apdu[alen++]=CJ_SPECIAL_CLA;
  apdu[alen++]=CJ_SPECIAL_INS_GETMODCOUNT;
  apdu[alen++]=0x00; 
  apdu[alen++]=0x00;
  lr=sizeof(rsp);
  ret=r->sendApdu(&dad, &sad,
                  alen, apdu,
                  &lr, rsp );
#if 0
  printf( "CT_data: %d\n", ret );
#endif
  if(ret!=CT_API_RV_OK)
    return -1;
#if 0
  printf("    sad: %d, dad: %d, rsp:", sad, dad );
  for( i=0; i<lr; i++ ) printf( " %.2X", rsp[i] );
  printf( "\n" );
#endif

  if (rsp[lr-2]!=0x90) {
    return -1;
  }

  return (int)rsp[0];
}



int getModuleInfo(Reader *r, int idx, cj_ModuleInfo *modInfo) {
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
  ret=r->sendApdu(&dad, &sad,
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



int getReaderInfo(Reader *r, cj_ReaderInfo *readerInfo) {
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
  ret=r->sendApdu(&dad, &sad,
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



int main(int argc, char **argv) {
#ifdef HAVE_PCSC
  Driver *dr=NewDriverPcsc();
#else
  Driver *dr=new DriverCtapi();
#endif
  int rv;
  std::list<Reader*>::const_iterator it;

  rv=dr->open();
  if (rv<0) {
    fprintf(stderr, "Error in open: %d\n", rv);
    return 2;
  }

  rv=dr->enumReaders();
  if (rv<0) {
    fprintf(stderr, "Error in enumReaders: %d\n", rv);
    return 2;
  }

  for (it=dr->getReaders().begin(); it!=dr->getReaders().end(); it++) {
    int rv;

    fprintf(stderr, " - %s (%04x, %04x: %s)\n",
            (*it)->getName().c_str(),
            (*it)->getVendorId(),
            (*it)->getProductId(),
            (*it)->getSerial().c_str());

    rv=(*it)->gatherInfo(true);
    if (rv!=ErrorCode_Ok) {
      fprintf(stderr, "Error gathering info: %d\n", rv);
      return 2;
    }

    if (1 /*(*it)->getProductId()>=0x400 ||*/) {
      const cj_ReaderInfo &ri=(*it)->getReaderInfo();

      if (ri.ContentsMask & RSCT_READER_MASK_PID)
        fprintf(stderr, "  PID               : %x\n", ri.PID);
  
      if (ri.ContentsMask & RSCT_READER_MASK_HARDWARE) {
        fprintf(stderr, "  Hardware-Mask     : %x\n",ri.HardwareMask);
        fprintf(stderr, "  Reader supports   :");
        if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC1)
          fprintf(stderr, " ICC1");
        if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC2)
          fprintf(stderr, " ICC2");
        if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC3)
          fprintf(stderr, " ICC3");
        if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC4)
          fprintf(stderr, " ICC4");
        if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC5)
          fprintf(stderr, " ICC5");
        if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC6)
          fprintf(stderr, " ICC6");
        if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC7)
          fprintf(stderr, " ICC7");
        if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_ICC8)
          fprintf(stderr, " ICC8");
        if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_KEYPAD)
          fprintf(stderr, " Keypad");
        if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_DISPLAY)
          fprintf(stderr, " Display");
        if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_BIOMETRIC)
          fprintf(stderr, " Fingersensor");
        if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_UPDATEABLE)
          fprintf(stderr, " Firmwareupdate");
        if (ri.HardwareMask & RSCT_READER_HARDWARE_MASK_MODULES)
          fprintf(stderr, " Sicherheitsmodule");
      }
      fprintf(stderr, "\n");
  
      if (ri.ContentsMask & RSCT_READER_MASK_VERSION)
        fprintf(stderr, "  Version           : %x\n", ri.Version);
  
  
      if (ri.ContentsMask & RSCT_READER_MASK_HARDWARE_VERSION)
        fprintf(stderr, "  Hardware-Version  : %x\n", ri.HardwareVersion);
  
  
      if (ri.ContentsMask & RSCT_READER_MASK_FLASH_SIZE)
        fprintf(stderr, "  Flashgroesse      : %d\n", ri.FlashSize);
  
  
      if (ri.ContentsMask & RSCT_READER_MASK_HEAP_SIZE)
        fprintf(stderr, "  Heapgroesse       : %d\n", ri.HeapSize);
  
  
      if (ri.ContentsMask & RSCT_READER_MASK_SERIALNUMBER)
        fprintf(stderr, "  Seriennummer      : %s\n", ri.SeriaNumber);
  
      if (ri.ContentsMask & RSCT_READER_MASK_VENDOR_STRING)
        fprintf(stderr, "  Hersteller        : %s\n", ri.VendorString);
  
  
      if (ri.ContentsMask & RSCT_READER_MASK_PRODUCT_STRING)
        fprintf(stderr, "  Produkt           : %s\n", ri.ProductString);
  
  
      if (ri.ContentsMask & RSCT_READER_MASK_PRODUCTION_DATE)
        fprintf(stderr, "  Herstellungsdatum : %s %s\n", ri.ProductionDate, ri.ProductionTime);
  
  
      if (ri.ContentsMask & RSCT_READER_MASK_TEST_DATE)
        fprintf(stderr, "  Testdatum         : %s %s\n", ri.TestDate, ri.TestTime);
  
  
      if (ri.ContentsMask & RSCT_READER_MASK_COMMISSIONING_DATE)
        fprintf(stderr, "  Ausgabedatum      : %s %s\n", ri.CommissioningDate, ri.CommissioningTime);
  
  
      if (ri.ContentsMask & RSCT_READER_MASK_COM_TYPE)
        fprintf(stderr, "  Anschlussart      : %s\n", ri.CommunicationString);
  
  
      if (ri.ContentsMask & RSCT_READER_MASK_PORT_ID)
        fprintf(stderr, "  Anschlussnummer   : %d\n", ri.PortID);
  
  
      if (ri.ContentsMask & RSCT_READER_MASK_IFD_BRIDGE)
        fprintf(stderr, "  IFD-Bridge        : %s\n", ri.IFDNameOfIfdBridgeDevice);
  
      if (ri.ContentsMask & RSCT_READER_MASK_HW_STRING)
        fprintf(stderr, "  Hardwarestring    : %s\n", ri.HardwareString);

    } /* if newer reader */

    if ((*it)->getProductId()>=0x400) {
      const std::list<cj_ModuleInfo> &modules=(*it)->getModuleInfos();
      std::list<cj_ModuleInfo>::const_iterator mit;

      for (mit=modules.begin(); mit!=modules.end(); mit++) {
        const cj_ModuleInfo &mi=*mit;

        fprintf(stderr, "   Module information\n");
        if (mi.ContentsMask & RSCT_MODULE_MASK_ID)
          fprintf(stderr, "    ID:          %08x\n", mi.ID);
        if (mi.ContentsMask & RSCT_MODULE_MASK_VERSION)
          fprintf(stderr, "    Version:     %08x\n", mi.Version);
        if (mi.ContentsMask & RSCT_MODULE_MASK_REVISION)
          fprintf(stderr, "    Revision:    %08x\n", mi.Revision);
        if (mi.ContentsMask & RSCT_MODULE_MASK_VARIANT)
          fprintf(stderr, "    Variant:     %08x\n", mi.Variant);
        if (mi.ContentsMask & RSCT_MODULE_MASK_DESCRIPTION)
          fprintf(stderr, "    Description: %17s\n", mi.Description);
        if (mi.ContentsMask & RSCT_MODULE_MASK_DATE)
          fprintf(stderr, "    Date:        %12s %6s\n", mi.Date, mi.Time);
      }
    } /* if newer reader */
  }

  rv=dr->close();
  if (rv<0) {
    fprintf(stderr, "Error in close: %d\n", rv);
    return 2;
  }

  return 1;
}


