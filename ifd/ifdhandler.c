/*
    ifdhandler.c

    Trivial implementation of wrapper functions for the CT-API 
    conforming to MUSCLE PCSC/Lite IFD Handler v2.0 definition
    by David Corcoran.
    
    This file is part of the Unix driver for Towitoko smartcard readers
    Copyright (C) 1998 1999 2000 Carlos Prados (cprados@yahoo.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    This file has been extended by Martin Preuss to make it a v3 IFD handler.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "cjeca32.h"
#include "Debug.h"

//#include "pcscdefines.h"
#include "ifdhandler.h"
#include "part10_l.h"
#include "ctbcs.h"
#include "config_l.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif


#define TAG_IFD_ATR_WIN32 0x90303


#define DEBUGP(Lun,format, ...) \
    rsct_log((((unsigned short) (Lun >> 16)) & 0xffff), \
             DEBUG_MASK_IFD,\
             __FILE__, __LINE__, __FUNCTION__, format , ##__VA_ARGS__ )

#define DEBUGP2(ctn,format, ...) \
    rsct_log(ctn, \
             DEBUG_MASK_IFD,\
             __FILE__, __LINE__, __FUNCTION__, format , ##__VA_ARGS__ )

/*
 * Not exported constants definition
 */

/* Maximum number of readers handled */
#define IFDH_MAX_READERS        32

/* Maximum number of slots per reader handled */
#define IFDH_MAX_SLOTS          1

#define PATHMAX 256


static int rsct_ifd_driver_initialized=0;


/*
 * Not exported data types definition
 */

typedef struct
{
  DEVICE_CAPABILITIES device_capabilities;
  ICC_STATE icc_state;
  DWORD ATR_Length;
  PROTOCOL_OPTIONS protocol_options;
  char deviceName[PATHMAX+1];
  int supportTag51;
}
IFDH_Status;

/*
 * Not exported variables definition
 */

/* Matrix that stores status information of all slots and readers */
static IFDH_Status *ifdh_status[IFDH_MAX_READERS];

#ifdef HAVE_PTHREAD_H
static pthread_mutex_t ifdh_status_mutex[IFDH_MAX_READERS];
#endif



int initDriver() {
  rsct_ifd_driver_initialized++;
  if (rsct_ifd_driver_initialized>1)
    return 0;
  else {
    int i;
#ifdef HAVE_PTHREAD_H
    pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER;
#endif

    for (i=0; i<IFDH_MAX_READERS; i++) {
      ifdh_status[i]=NULL;
#ifdef HAVE_PTHREAD_H
      ifdh_status_mutex[i]=mut;
#endif
    }
    return 0;
  }
}



int deinitDriver() {
  if (rsct_ifd_driver_initialized<1)
    return -1;
  rsct_ifd_driver_initialized--;
  if (rsct_ifd_driver_initialized<1) {
    int i;
#ifdef HAVE_PTHREAD_H
    pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER;
#endif

    for (i=0; i<IFDH_MAX_READERS; i++) {
      if (ifdh_status[i]!=NULL) {
	free(ifdh_status[i]);
	ifdh_status[i]=NULL;
      }
#ifdef HAVE_PTHREAD_H
      ifdh_status_mutex[i]=mut;
#endif
    }
  }

  return 0;
}



/* 
 * Exported functions definition
 */
CJECA32_EXPORT
RESPONSECODE IFDHCreateChannel (DWORD Lun, DWORD Channel){
  char ret;
  unsigned short ctn, pn, slot;
  RESPONSECODE rv;

  DEBUGP(Lun, "IFDHCreateChannel: Lun %X, Channel %d\n", (int)Lun, (int)Channel);

  ctn=((unsigned short) (Lun >> 16)) & 0xffff;
  slot=((unsigned short) (Lun & 0x0000FFFF)) % IFDH_MAX_SLOTS;
  if (ctn>=IFDH_MAX_READERS || slot>=IFDH_MAX_SLOTS) {
    DEBUGP(Lun, "Invalid LUN %X\n", Lun);
    return IFD_COMMUNICATION_ERROR;
  }

#ifdef HAVE_PTHREAD_H
  pthread_mutex_lock (&ifdh_status_mutex[ctn]);
#endif

  if (initDriver()) {
    DEBUGP(Lun, "Could not init driver\n");
    rv=IFD_COMMUNICATION_ERROR;
  }
  else {
    if (ifdh_status[ctn] == NULL) {
      pn=(unsigned short) Channel+1;
      ret=CT_init (ctn, pn);
      DEBUGP(Lun, "%d=CT_init(%d,%d)\n",(int)ret,(int)ctn,(int)pn);
  
      if (ret==0){
	ifdh_status[ctn]=(IFDH_Status*) malloc(sizeof(IFDH_Status));
	if (ifdh_status[ctn]!=NULL) {
	  memset(ifdh_status[ctn], 0, sizeof(IFDH_Status));
	  rv=IFD_SUCCESS;
	}
	else {
	  DEBUGP(Lun, "Could not allocate memory");
	  rv=IFD_COMMUNICATION_ERROR;
	}
      }
      else {
	rv=IFD_COMMUNICATION_ERROR;
      }
    }
    else
      rv=IFD_SUCCESS;
  
    if (rv!=IFD_SUCCESS)
      deinitDriver();

  }

#ifdef HAVE_PTHREAD_H
  pthread_mutex_unlock(&ifdh_status_mutex[ctn]);
#endif

  return rv;
}



CJECA32_EXPORT
RESPONSECODE IFDHCreateChannelByName(DWORD Lun, char *devName){
  char ret;
  unsigned short ctn, slot;
  RESPONSECODE rv;

  DEBUGP(Lun, "IFDHCreateChannelByName: Lun %X, Device %s\n", (int)Lun, devName);

  ctn=((unsigned short) (Lun >> 16)) & 0xffff;
  slot=((unsigned short) (Lun & 0x0000FFFF)) % IFDH_MAX_SLOTS;
  if (ctn>=IFDH_MAX_READERS || slot>=IFDH_MAX_SLOTS) {
    DEBUGP(Lun, "Invalid LUN %X\n", Lun);
    return IFD_COMMUNICATION_ERROR;
  }

#ifdef HAVE_PTHREAD_H
  pthread_mutex_lock (&ifdh_status_mutex[ctn]);
#endif

  if (initDriver()) {
    DEBUGP(Lun, "Could not init driver\n");
    rv=IFD_COMMUNICATION_ERROR;
  }
  else {
    if (ifdh_status[ctn] == NULL) {
#ifdef OS_DARWIN
      ret=CT_init(ctn, slot+1);
      DEBUGP(Lun, "%d=CT_init(%d,%d)\n",(int)ret,(int)ctn,(int)(slot+1));
  
      if (ret==0){
	ifdh_status[ctn]=(IFDH_Status*) malloc(sizeof(IFDH_Status));
	if (ifdh_status[ctn]!=NULL) {
	  memset(ifdh_status[ctn], 0, sizeof(IFDH_Status));
	  rv=IFD_SUCCESS;
	}
	else {
	  DEBUGP(Lun, "Could not allocate memory");
	  rv=IFD_COMMUNICATION_ERROR;
	}
      }
      else {
	rv=IFD_COMMUNICATION_ERROR;
      }
#else
      ret=rsct_init_name(ctn, devName);
      DEBUGP(Lun, "%d=CT_init_name(%d,%s)\n",(int)ret,(int)ctn, devName);
      if (ret == 0) {
	ifdh_status[ctn]=(IFDH_Status*) malloc(sizeof(IFDH_Status));
	if (ifdh_status[ctn]!=NULL) {
	  memset(ifdh_status[ctn], 0, sizeof(IFDH_Status));
	  strncpy(ifdh_status[ctn]->deviceName, devName, PATHMAX);
	  rv=IFD_SUCCESS;
	}
	else {
	  DEBUGP(Lun, "Could not allocate memory");
	  rv=IFD_COMMUNICATION_ERROR;
	}
      }
      else {
	rv=IFD_COMMUNICATION_ERROR;
      }
#endif /* darwin */
    }
    else {
      if (ifdh_status[ctn]->deviceName[0] &&
	  strcmp(ifdh_status[ctn]->deviceName, devName)!=0) {
	DEBUGP(Lun, "ERROR: The LUN %X is already in use!\n", Lun, devName);
	rv=IFD_COMMUNICATION_ERROR;
      }
      else
	rv=IFD_SUCCESS;
    }

    if (rv!=IFD_SUCCESS)
      deinitDriver();
  }

#ifdef HAVE_PTHREAD_H
  pthread_mutex_unlock (&ifdh_status_mutex[ctn]);
#endif

  return rv;
}



CJECA32_EXPORT
RESPONSECODE IFDHCloseChannel(DWORD Lun){
  char ret;
  unsigned short ctn, slot;
  RESPONSECODE rv;

  DEBUGP(Lun, "IFDHCloseChannel: Lun %X\n", (int)Lun);

  ctn=((unsigned short) (Lun >> 16)) & 0xffff;
  slot=((unsigned short) (Lun & 0x0000FFFF)) % IFDH_MAX_SLOTS;
  if (ctn>=IFDH_MAX_READERS || slot>=IFDH_MAX_SLOTS) {
    DEBUGP(Lun, "Invalid LUN %X\n", Lun);
    return IFD_COMMUNICATION_ERROR;
  }

#ifdef HAVE_PTHREAD_H
  pthread_mutex_lock (&ifdh_status_mutex[ctn]);
#endif

  if (ifdh_status[ctn]!=NULL) {
    ret=CT_close(ctn);
    if (ret == 0){
      if (ifdh_status[ctn]!=NULL) {
	free (ifdh_status[ctn]);
	ifdh_status[ctn]=NULL;
      }
      rv=IFD_SUCCESS;
    }
    else
      rv=IFD_COMMUNICATION_ERROR;

    deinitDriver();
  }
  else {
    DEBUGP(Lun, "Reader LUN %X is not open\n", Lun);
    rv=IFD_COMMUNICATION_ERROR;
  }

#ifdef HAVE_PTHREAD_H
  pthread_mutex_unlock (&ifdh_status_mutex[ctn]);
#endif

  return rv;
}



CJECA32_EXPORT
RESPONSECODE IFDHGetCapabilities(DWORD Lun, DWORD Tag, PDWORD Length,
				 PUCHAR Value){
  unsigned short ctn, slot;
  RESPONSECODE rv;

  DEBUGP(Lun, "IFDHGetCapabilities: Lun %X, Tag %X, Length %d\n",
	 (int)Lun, (int)Tag, (int)*Length);

  ctn=((unsigned short) (Lun >> 16)) & 0xffff;
  slot=((unsigned short) (Lun & 0x0000FFFF)) % IFDH_MAX_SLOTS;
  if (ctn>=IFDH_MAX_READERS || slot>=IFDH_MAX_SLOTS) {
    DEBUGP(Lun, "Invalid LUN %X\n", Lun);
    return IFD_COMMUNICATION_ERROR;
  }

#ifdef HAVE_PTHREAD_H
  pthread_mutex_lock (&ifdh_status_mutex[ctn]);
#endif

  if (ifdh_status[ctn] != NULL){
    switch(Tag) {
    case TAG_IFD_ATR:
    case TAG_IFD_ATR_WIN32:
      if ((*Length)>=ifdh_status[ctn]->ATR_Length && Value) {
	(*Length)=ifdh_status[ctn]->ATR_Length;
	memcpy (Value, ifdh_status[ctn]->icc_state.ATR, (*Length));
	rv=IFD_SUCCESS;
      }
      else
	rv=IFD_ERROR_TAG;
      break;

#ifdef HAVE_PTHREAD_H
    case TAG_IFD_SIMULTANEOUS_ACCESS:
      if (*Length>=1 && Value) {
	*Length=1;
	*Value=IFDH_MAX_READERS;
	rv=IFD_SUCCESS;
      }
      else
	rv=IFD_ERROR_TAG;
      break;

    case TAG_IFD_THREAD_SAFE:
      if (*Length>=1){
	*Length=1;
	*Value=1; /* allow mutliple readers at the same time */
	rv=IFD_SUCCESS;
      }
      else
	rv=IFD_ERROR_TAG;
      break;
#endif

    case TAG_IFD_SLOTS_NUMBER:
      if (*Length>=1 && Value) {
	*Length=1;
	*Value=1;
	rv=IFD_SUCCESS;
      }
      else
	rv=IFD_ERROR_TAG;
      break;

    case TAG_IFD_SLOT_THREAD_SAFE:
      if (*Length>=1 && Value){
	*Length=1;
	*Value=0; /* Can NOT talk to multiple slots at the same time */
	rv=IFD_SUCCESS;
      }
      else
	rv=IFD_ERROR_TAG;
      break;

    case SCARD_ATTR_VENDOR_IFD_VERSION:
      /* Vendor-supplied interface device version (DWORD in the form
       * 0xMMmmbbbb where MM = major version, mm = minor version, and
       * bbbb = build number). */
      if (*Length>=1 && Value){
	DWORD v;

	v=(CYBERJACK_VERSION_MAJOR<<24) |
	  (CYBERJACK_VERSION_MINOR<<16) |
	  (CYBERJACK_VERSION_BUILD & 0xffff);
	*Length=sizeof(DWORD);
	*(DWORD*)Value=v;
	rv=IFD_SUCCESS;
      }
      else
	rv=IFD_ERROR_TAG;
      break;

    case SCARD_ATTR_VENDOR_NAME:
#define VENDOR_NAME "Reiner SCT"
      if (*Length>=sizeof(VENDOR_NAME) && Value){
	*Length=sizeof(VENDOR_NAME);
	memcpy(Value, VENDOR_NAME, sizeof(VENDOR_NAME));
	rv=IFD_SUCCESS;
      }
      else
	rv=IFD_ERROR_TAG;
#undef VENDOR_NAME
      break;

    default:
      rv=IFD_ERROR_TAG;
    } /* switch */
  }
  else
    rv=IFD_ICC_NOT_PRESENT;

#ifdef HAVE_PTHREAD_H
  pthread_mutex_unlock (&ifdh_status_mutex[ctn]);
#endif

  return rv;
}



CJECA32_EXPORT RESPONSECODE
IFDHSetCapabilities (DWORD Lun, DWORD Tag, DWORD Length, PUCHAR Value){
  DEBUGP(Lun, "IFDHSetCapabilities: Lun %X, Tag %d, Length %d\n",
	 (int)Lun, (int)Tag, (int)Length);

  return IFD_NOT_SUPPORTED;
}



CJECA32_EXPORT RESPONSECODE
IFDHSetProtocolParameters (DWORD Lun, DWORD Protocol,
                           UCHAR Flags, UCHAR PTS1, UCHAR PTS2, UCHAR PTS3){
  char ret;
  unsigned short ctn, slot, lc, lr;
  UCHAR cmd[16], rsp[256], sad, dad;
  RESPONSECODE rv;

  DEBUGP(Lun, "IFDHSetProtocolParameters: Lun %X, Protocol %d, Flags %d\n",
	 (int)Lun, (int)Protocol, (int)Flags);

  ctn=((unsigned short) (Lun >> 16)) & 0xffff;
  slot=((unsigned short) (Lun & 0x0000FFFF)) % IFDH_MAX_SLOTS;
  if (ctn>=IFDH_MAX_READERS || slot>=IFDH_MAX_SLOTS) {
    DEBUGP(Lun, "Invalid LUN %X\n", Lun);
    return IFD_COMMUNICATION_ERROR;
  }

#ifdef HAVE_PTHREAD_H
  pthread_mutex_lock (&ifdh_status_mutex[ctn]);
#endif

  if (ifdh_status[ctn] != NULL &&
      (ifdh_status[ctn]->icc_state.ATR[0]==0x3b ||
       ifdh_status[ctn]->icc_state.ATR[0]==0x3f)) {
    lc=0;
    cmd[lc++]=0x80;
    cmd[lc++]=0x60;
    cmd[lc++]=0x01;
    cmd[lc++]=0x00;
    cmd[lc++]=0x03;
    cmd[lc++]=0x22;
    cmd[lc++]=0x01;

    if (Protocol==SCARD_PROTOCOL_T0)
      cmd[lc++]=0x01;
    else if (Protocol==SCARD_PROTOCOL_T1)
      cmd[lc++]=0x02;
    else {
      DEBUGP(Lun, "IFDHSetProtocolParameters: Protocol %d not supported\n", Protocol);
#ifdef HAVE_PTHREAD_H
      pthread_mutex_unlock (&ifdh_status_mutex[ctn]);
#endif
      return IFD_PROTOCOL_NOT_SUPPORTED;
    }

    DEBUGP(Lun, "IFDHSetProtocolParameters: Setting protocol %d\n", Protocol);
    dad = 0x01;
    sad = 0x02;
    lr = sizeof(rsp);

    ret = CT_data (ctn, &dad, &sad, lc, cmd, &lr, rsp);
    if ((ret == 0) && (lr >= 2)){
      if (rsp[lr-2]==0x90) {
	rv=IFD_SUCCESS;
      }
      else {
	DEBUGP(Lun, "IFDHSetProtocolParameters: Error setting protocol %d (%02x, %02x)\n",
	       (int) Protocol, rsp[lr-2], rsp[lr-1]);
	rv=IFD_COMMUNICATION_ERROR;
      }
    }
    else {
      DEBUGP(Lun, "IFDHSetProtocolParameters: Error setting protocol %d (%d)\n",
	     (int) Protocol, (int) ret);
      rv=IFD_COMMUNICATION_ERROR;
    }
  }
  else {
    if (ifdh_status[ctn] == NULL) {
      DEBUGP(Lun, "IFDHSetProtocolParameters: No status.\n");
    }
    else {
      DEBUGP(Lun, "IFDHSetProtocolParameters: Unexpected ATR byte 0 (%02x).\n", ifdh_status[ctn]->icc_state.ATR[0]);
    }
    rv=IFD_PROTOCOL_NOT_SUPPORTED;
  }

#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock (&ifdh_status_mutex[ctn]);
#endif

  return rv;
}



CJECA32_EXPORT RESPONSECODE
IFDHPowerICC (DWORD Lun, DWORD Action, PUCHAR Atr, PDWORD AtrLength){
  char ret;
  unsigned short ctn, slot, lc, lr;
  UCHAR cmd[5], rsp[256], sad, dad;
  RESPONSECODE rv;

  DEBUGP(Lun, "IFDHPowerICC: Lun %X, Action %d, ATR Length %d\n",
	 (int)Lun, (int)Action, (int)*AtrLength);

  ctn=((unsigned short) (Lun >> 16)) & 0xffff;
  slot=((unsigned short) (Lun & 0x0000FFFF)) % IFDH_MAX_SLOTS;
  if (ctn>=IFDH_MAX_READERS || slot>=IFDH_MAX_SLOTS) {
    DEBUGP(Lun, "Invalid LUN %X\n", Lun);
    return IFD_COMMUNICATION_ERROR;
  }

#ifdef HAVE_PTHREAD_H
  pthread_mutex_lock (&ifdh_status_mutex[ctn]);
#endif

  if (ifdh_status[ctn] != NULL) {
    if (Action == IFD_POWER_UP){
      cmd[0] = CTBCS_CLA;
      cmd[1] = CTBCS_INS_REQUEST;
      cmd[2] = (UCHAR) (slot + 1);
      cmd[3] = CTBCS_P2_REQUEST_GET_ATR;
      cmd[4] = 0x00;

      dad = 0x01;
      sad = 0x02;
      lr = 256;
      lc = 5;

      ret = CT_data (ctn, &dad, &sad, 5, cmd, &lr, rsp);

      if ((ret == 0) && (lr >= 2)){
	if (rsp[lr-2]==0x90) {
	  int alen;
	  int clen;

	  alen=(DWORD) lr-2;

	  clen=alen;
	  if (clen>sizeof(ifdh_status[ctn]->icc_state.ATR))
	    clen=sizeof(ifdh_status[ctn]->icc_state.ATR);
	  ifdh_status[ctn]->ATR_Length = (DWORD) clen;
	  if (clen)
	    memcpy(ifdh_status[ctn]->icc_state.ATR, rsp, clen);

	  clen=alen;
	  if (clen>*AtrLength)
	    clen=*AtrLength;
	  (*AtrLength)=clen;
	  if (clen)
	    memcpy (Atr, rsp, clen);
	  rv=IFD_SUCCESS;
	}
        else
	  rv = IFD_ERROR_POWER_ACTION;
      }
      else
	rv=IFD_COMMUNICATION_ERROR;
    }
    else if (Action == IFD_POWER_DOWN){
      cmd[0] = CTBCS_CLA;
      cmd[1] = CTBCS_INS_EJECT;
      cmd[2] = (UCHAR) (slot + 1);
      cmd[3] = 0x00;
      /*cmd[4] = 0x00;*/

      dad = 0x01;
      sad = 0x02;
      lr = 256;
      lc = 4;

      ifdh_status[ctn]->ATR_Length = 0;
      ret=CT_data(ctn, &dad, &sad, 4, cmd, &lr, rsp);

      if ((ret == 0) && (lr >= 2)){
	rv=IFD_SUCCESS;
      }
      else
	rv=IFD_COMMUNICATION_ERROR;
    }
    else if (Action == IFD_RESET){
      cmd[0] = CTBCS_CLA;
      cmd[1] = CTBCS_INS_RESET;
      cmd[2] = (UCHAR) (slot + 1);
      cmd[3] = CTBCS_P2_RESET_GET_ATR;
      cmd[4] = 0x00;

      dad = 0x01;
      sad = 0x02;
      lr = 256;
      lc = 5;

      ret = CT_data (ctn, &dad, &sad, 5, cmd, &lr, rsp);

      if ((ret == 0) && (lr >= 2)){
	if (rsp[lr-2]==0x90) {
	  int alen;
	  int clen;

	  alen=(DWORD) lr-2;

	  clen=alen;
	  if (clen>sizeof(ifdh_status[ctn]->icc_state.ATR))
	    clen=sizeof(ifdh_status[ctn]->icc_state.ATR);
	  ifdh_status[ctn]->ATR_Length = (DWORD) clen;
	  if (clen)
	    memcpy(ifdh_status[ctn]->icc_state.ATR, rsp, clen);

	  clen=alen;
	  if (clen>*AtrLength)
	    clen=*AtrLength;
	  (*AtrLength)=clen;
	  if (clen)
	    memcpy (Atr, rsp, clen);
	  rv=IFD_SUCCESS;
	}
	else
	  rv=IFD_ICC_NOT_PRESENT;
      }
      else
	rv=IFD_COMMUNICATION_ERROR;

    }
    else
      rv=IFD_NOT_SUPPORTED;
  }
  else
    rv=IFD_ICC_NOT_PRESENT;

#ifdef HAVE_PTHREAD_H
  pthread_mutex_unlock (&ifdh_status_mutex[ctn]);
#endif

  return rv;
}



CJECA32_EXPORT RESPONSECODE
IFDHTransmitToICC(DWORD Lun, SCARD_IO_HEADER SendPci,
		  PUCHAR TxBuffer, DWORD TxLength,
		  PUCHAR RxBuffer, PDWORD RxLength, PSCARD_IO_HEADER RecvPci){
  char ret;
  unsigned short ctn, slot, lc, lr;
  UCHAR sad, dad;
  RESPONSECODE rv;

  DEBUGP(Lun, "IFDHTransmitToICC: Lun %X, TxLength %d\n", (int)Lun, (int)TxLength);

  ctn=((unsigned short) (Lun >> 16)) & 0xffff;
  slot=((unsigned short) (Lun & 0x0000FFFF)) % IFDH_MAX_SLOTS;
  if (ctn>=IFDH_MAX_READERS || slot>=IFDH_MAX_SLOTS) {
    DEBUGP(Lun, "Invalid LUN %X\n", Lun);
    return IFD_COMMUNICATION_ERROR;
  }

#ifdef HAVE_PTHREAD_H
  pthread_mutex_lock (&ifdh_status_mutex[ctn]);
#endif

  if (ifdh_status[ctn] != NULL) {
#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock (&ifdh_status_mutex[ctn]);
#endif
    if (TxLength>0 && TxBuffer[0]==0x20)
      dad = 0x01;
    else
      dad = (UCHAR) ((slot == 0) ? 0x00 : slot + 1);
    sad = 0x02;
    if (*RxLength>65535)
      lr=65535;
    else
      lr = (unsigned short) (*RxLength);
    lc = (unsigned short) TxLength;

    ret = CT_data (ctn, &dad, &sad, lc, TxBuffer, &lr, RxBuffer);

    if (ret == 0) {
      (*RxLength) = lr;
      rv = IFD_SUCCESS;
    }
    else {
      (*RxLength) = 0;
      rv = IFD_COMMUNICATION_ERROR;
    }
  }
  else {
#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock (&ifdh_status_mutex[ctn]);
#endif
    rv = IFD_ICC_NOT_PRESENT;
  }

  return rv;
}



CJECA32_EXPORT RESPONSECODE
IFDHControl(DWORD Lun,
	    DWORD controlCode,
	    PUCHAR TxBuffer,
	    DWORD TxLength,
	    PUCHAR RxBuffer,
	    DWORD RxLength,
	    PDWORD RxReturned){
  char ret;
  unsigned short ctn, slot, lc, lr;
  UCHAR sad, dad;
  RESPONSECODE rv;

  DEBUGP(Lun, "IFDHControl: Lun %X, Code %X, TxLength %d\n",
	 (int)Lun, (int)controlCode, (int)TxLength);

  ctn=((unsigned short) (Lun >> 16)) & 0xffff;
  slot=((unsigned short) (Lun & 0x0000FFFF)) % IFDH_MAX_SLOTS;
  if (ctn>=IFDH_MAX_READERS || slot>=IFDH_MAX_SLOTS) {
    DEBUGP(Lun, "Invalid LUN %X\n", Lun);
    return IFD_COMMUNICATION_ERROR;
  }

  if (controlCode!=0)
    return Part10Control(ctn, slot,
			 controlCode,
			 TxBuffer, TxLength,
			 RxBuffer, RxLength,
                         RxReturned);

#ifdef HAVE_PTHREAD_H
  pthread_mutex_lock (&ifdh_status_mutex[ctn]);
#endif

  if (ifdh_status[ctn] != NULL) {
#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock (&ifdh_status_mutex[ctn]);
#endif
    dad = 0x01;
    sad = 0x02;
    if (RxLength>65535)
      lr=65535;
    else
      lr = (unsigned short) RxLength;
    lc = (unsigned short) TxLength;

    ret = CT_data(ctn, &dad, &sad, lc, TxBuffer, &lr, RxBuffer);

    if (ret == 0) {
      (*RxReturned) = lr;
      rv = IFD_SUCCESS;
    }
    else {
      (*RxReturned) = 0;
      rv = IFD_COMMUNICATION_ERROR;
    }
  }
  else {
#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock (&ifdh_status_mutex[ctn]);
#endif
    rv = IFD_ICC_NOT_PRESENT;
  }

  return rv;
}


CJECA32_EXPORT RESPONSECODE
IFDHICCPresence (DWORD Lun){
  char ret;
  unsigned short ctn, slot, lc, lr;
  UCHAR cmd[5], rsp[256], sad, dad;
  RESPONSECODE rv;

  ctn=((unsigned short) (Lun >> 16)) & 0xffff;
  slot=((unsigned short) (Lun & 0x0000FFFF)) % IFDH_MAX_SLOTS;
  if (ctn>=IFDH_MAX_READERS || slot>=IFDH_MAX_SLOTS) {
    DEBUGP(Lun, "Invalid LUN %X\n", Lun);
    return IFD_COMMUNICATION_ERROR;
  }

  cmd[0] = CTBCS_CLA;
  cmd[1] = CTBCS_INS_STATUS;
  cmd[2] = CTBCS_P1_CT_KERNEL;
  cmd[3] = CTBCS_P2_STATUS_ICC;
  cmd[4] = 0x00;

  dad = 0x01;
  sad = 0x02;
  lc = 5;
  lr = 256;

  ret = CT_data (ctn, &dad, &sad, lc, cmd, &lr, rsp);
  DEBUGP(Lun, "Status: ret=%d, lr=%d", ret, lr);

  if (ret == 0) {
    if (slot < lr - 2) {
      if (rsp[2+slot]==CTBCS_DATA_STATUS_NOCARD) {
	DEBUGP(Lun, "IFDHPresence: Slot %d: no card (%02x)\n",
	       (int)Lun, rsp[2+slot]);
	rv=IFD_ICC_NOT_PRESENT;
      }
      else
	rv=IFD_ICC_PRESENT;
    }
    else {
        DEBUGP(Lun, "IFDHPresence: Lun %X: To few bytes received\n", (int)Lun);
        rv=IFD_ICC_NOT_PRESENT;
    }
  }
  else {
    DEBUGP(Lun, "Communication error (%d)", ret);
    rv=IFD_COMMUNICATION_ERROR;
  }

  DEBUGP(Lun, "IFDHPresence: Lun %X (%d)\n", (int)Lun, rv);

  return rv;
}


#include "part10.c"


