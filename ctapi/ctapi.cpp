/***************************************************************************
    begin       : Mon Apr 23 2007
    copyright   : (C) 2007-2010 by Martin Preuss
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

#include "Platform.h"
#include "ctapi_beep.h"
#include "config_l.h"
#include "version.h"
#include "dialog.h"

#include "Reader.h"
#include "SerialUnix.h"

#include "ctapi-ecom.h"
#include "cjppa.h"

#ifdef ENABLE_NONSERIAL
# include "ausb_l.h"
# include "usbdev_l.h"
#endif

#include <stdarg.h>
#include <list>
#include <string>

#include <sys/stat.h>
#include <pwd.h>
#include <sys/types.h>

#include <stdio.h>

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif


#if 1
# define DEBUGP(ctn, format, args...) \
   rsct_log(ctn, DEBUG_MASK_CTAPI, __FILE__, __LINE__, __FUNCTION__, format, ## args)

# define DEBUGL(ctn, hdr, len, data) \
   rsct_log_bytes(ctn, DEBUG_MASK_CTAPI, __FILE__, __LINE__, __FUNCTION__, hdr, len, data)
#else
# define DEBUGP(ctn, format, args...)
# define DEBUGL(ctn, hdr, len, data)

#endif

struct Ctapi_Context {
  CReader *reader;
  CJ_INFO *oldEcom;
#ifdef ENABLE_NONSERIAL
  CCID_DEVICE ppa;
#endif
  uint16_t ctn;
  uint16_t port;
  CT_KEY_CB keyCallback;
  void *kcb_user_data;
  std::string readerName;

  RSCT_DIALOG *currentDialog;

  /** module pr key data to flash */
  std::string dataToFlash;
  std::string signatureToFlash;

  uint32_t moduleCount;
  cj_ModuleInfo *moduleList;

#ifdef HAVE_PTHREAD_H
  pthread_mutex_t mutex;
#endif


  Ctapi_Context()
    :reader(NULL)
#ifdef ENABLE_NONSERIAL
    ,oldEcom(NULL)
    ,ppa(NULL)
#endif
    ,keyCallback(NULL)
    ,kcb_user_data(NULL)
    ,currentDialog(NULL)
    ,moduleCount(0)
    ,moduleList(NULL)
  {
#ifdef HAVE_PTHREAD_H
    pthread_mutex_init(&mutex, NULL);
#endif
  };

  ~Ctapi_Context() {
    if (reader)
      delete reader;
    if (currentDialog)
      rsct_dialog_free(currentDialog);
#ifdef HAVE_PTHREAD_H
    pthread_mutex_destroy(&mutex);
#endif
  };

  void lock() {
#ifdef HAVE_PTHREAD_H
    pthread_mutex_lock(&mutex);
#endif
  };

  void unlock() {
#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock(&mutex);
#endif
  };

};


typedef enum {
  RSCT_DriverType_Unknown=0,
  RSCT_DriverType_ECA,
  RSCT_DriverType_ECOM_USER,
  RSCT_DriverType_ECOM_KERNEL,
  RSCT_DriverType_PPA,
  RSCT_DriverType_Serial
} RSCT_DRIVER_TYPE;


static int _ctapi_init_count=0;
static struct beep_struct *beepstruct = NULL;
static std::list<Ctapi_Context*> ct_context_list;

#ifdef HAVE_PTHREAD_H
static pthread_mutex_t ctapi_status_mutex=PTHREAD_MUTEX_INITIALIZER;
#endif


/* forward declaration */
static int8_t CT_special(Ctapi_Context *ctx,
                         uint8_t *dad,
                         uint8_t *sad,
                         uint16_t cmd_len,
                         const uint8_t *cmd,
                         uint16_t *response_len,
                         uint8_t *response);


#ifdef ENABLE_NONSERIAL
static void ctapi_LogAusb(ausb_dev_handle *ah,
			  const char *text,
			  const void *pData, uint32_t ulDataLen) {
  rsct_debug_out("<USB>",
		 DEBUG_MASK_COMMUNICATION_IN,
		 (char*)text,
		 (char*)pData, ulDataLen);
}
#endif


static Ctapi_Context *findContextByCtn(uint16_t ctn) {
  std::list<Ctapi_Context*>::iterator it;

  for (it=ct_context_list.begin();
       it!=ct_context_list.end();
       it++) {
    if ((*it)->ctn==ctn) {
      return *it;
    }
  }

  return NULL;
}



static Ctapi_Context *findContextByPort(uint16_t pn) {
  std::list<Ctapi_Context*>::iterator it;

  for (it=ct_context_list.begin();
       it!=ct_context_list.end();
       it++) {
    if ((*it)->port==pn) {
      return *it;
    }
  }

  return NULL;
}



static void addContext(Ctapi_Context *ctx) {
  ct_context_list.push_back(ctx);
}




static int showDialogIfRequired(Ctapi_Context *ctx, uint8_t dad, uint8_t sad, uint16_t lenc, const uint8_t *command) {
  if (lenc>4 && dad==1 && sad==2) {
    if (command[0] == 0x20  &&
	(command[1] == 0x18 || /* PERFORM VERIFICATION */
	 command[1] == 0x19)) {
      RSCT_DIALOG *dlg=NULL;
      char customDisplayText1[256];   /* MODIFY VERIFICATION DATA */
      char customDisplayText2[256];
      char customDisplayText3[256];
      int  timeOut=15;
      int  len, i;
      int  variablePINLength=0;
      int  noOfStages=1;
      const uint8_t *ctCmd;
      int rv;

      customDisplayText1[0]=0;
      customDisplayText2[0]=0;
      customDisplayText3[0]=0;

      // check for custom display text & custom timeout
      len=command[4];

      ctCmd = &command[5];
      for ( i=0; i < len; ) {
	uint8_t  cmdLen;

	cmdLen = ctCmd[i+1];
	switch (ctCmd[i]) {
	  case 0x50: {
	    /* custom display message, data is plain ASCII characters */
	    memcpy(customDisplayText1, &ctCmd[i+2], cmdLen);
	    customDisplayText1[cmdLen] = 0;
	    break;
	  }
  
	  case 0x61: {
	    /* custom display message, data is plain ASCII characters */
	    memcpy(customDisplayText2, &ctCmd[i+2], cmdLen);
	    customDisplayText2[cmdLen] = 0;
	    break;
	  }
  
	  case 0x62: {
	    /* custom display message, data is plain ASCII characters */
	    memcpy(customDisplayText3, &ctCmd[i+2], cmdLen);
	    customDisplayText3[cmdLen] = 0;
	    break;
	  }
  
	  case 0x80: {
	    int j;
  
	    /* custom timeout, data is binary encoded */
	    timeOut=0;
	    for (j=0; j<cmdLen; ++j )
	      timeOut=(timeOut<<8)+ctCmd[i+2+j];
	    break;
	  }
  
	  case 0x52: {
	    /* Command-to-perform */
	    /* Control byte specifies PIN length */
	    if ((ctCmd[i+2]&0x80)==0)
	      variablePINLength=1;

	    /* no of stages for PIN input:
	     * if command is MODIFY VERIFICATION, then we need to show 2 or 3 stages,
	     depending on whether this is an initial PIN change or a regular PIN change.
	     The initial PIN change is triggered by passing the same PIN position for old and new PIN */
	    if (command[1] == 0x19) {
	      if (ctCmd[i+3] == ctCmd[i+4])
		noOfStages=2;
              else
		noOfStages=3;
	    }
            else
	      noOfStages=1;
	    break;
	  }
	} /* switch */

	i+=cmdLen+2;
      } /* for */

      dlg=rsct_dialog_new(NULL,
			  noOfStages,
			  command[0],
			  command[1],
			  timeOut,
			  customDisplayText1[0]?customDisplayText1:NULL,
			  customDisplayText2[0]?customDisplayText2:NULL,
			  customDisplayText3[0]?customDisplayText3:NULL);
      rv=rsct_dialog_open(dlg);
      if (rv<0) {
	fprintf(stderr, "RSCT: Could not connect to cyberJack GUI (%d)\n", rv);
        rsct_dialog_free(dlg);
	return rv;
      }
      ctx->currentDialog=dlg;
    } /* if PERFORM VERIFICATION or MODIFICATION */
  } /* if command long enough and with matching SAD/DAD */

  return 0;
}



static void commonKeyCallback(struct Ctapi_Context *ctx, uint8_t transKey) {
  DEBUGP(CT_INVALID_CTN, "Common key callback: Key=%d", transKey);
  if (ctx) {
    if (ctx->currentDialog) {
      switch(transKey) {
      case RSCT_Key_Digit: {
	int n;

	n=rsct_dialog_get_char_num(ctx->currentDialog);
	n++;
	rsct_dialog_set_char_num(ctx->currentDialog, n, 1);
        break;
      }

      case RSCT_Key_CLR:
	rsct_dialog_set_char_num(ctx->currentDialog, 0, 1);
	break;

      case RSCT_Key_Ok: {
	int stage;

	stage=rsct_dialog_get_stage(ctx->currentDialog);
	stage++;
	if (stage<rsct_dialog_get_stages(ctx->currentDialog))
	  rsct_dialog_set_stage(ctx->currentDialog, stage);
	break;
      }

      default:
        break;
      }
    }

    if (ctx->keyCallback) {
      DEBUGP(ctx->ctn, "Calling user-defined callback");
      ctx->keyCallback(ctx->ctn, transKey, ctx->kcb_user_data);
    }
  }
  else {
    DEBUGP(CT_INVALID_CTN, "No user-defined callback, beeping");
    if (beepstruct)
      beep_whatever(beepstruct);
    else {
      DEBUGP(CT_INVALID_CTN, "No beep struct?");
    }
  }
}




#ifdef ENABLE_NONSERIAL

static void oldEcomKeyCallback(struct cj_info *ci, int key) {
  Ctapi_Context *ctx;
  int transKey;

  DEBUGP(CT_INVALID_CTN, "ECOM: Key=%d", key);
  switch(key) {
  case 0:  transKey=RSCT_Key_Digit; break;
  case 2:  transKey=RSCT_Key_Ok; break;
  case 4:  transKey=RSCT_Key_CLR; break;
  default: transKey=RSCT_Key_Unknown; break;
  }

  ctx=findContextByCtn(ci->ctn);
  if (ctx==NULL) {
    DEBUGP(CT_INVALID_CTN, "Context not open");
  }
  else {
    /* TODO: check whether we need to transform the key code */
    commonKeyCallback(ctx, key);
  }
}



static void ppaKeyCallback(CCID_CTX ccid, unsigned char status) {
  Ctapi_Context *ctx;
  int transKey;

  DEBUGP(CT_INVALID_CTN, "PPA: Key=%d", status);
  switch(status) {
  case 0:  transKey=RSCT_Key_Digit; break;
  case 2:  transKey=RSCT_Key_Ok; break;
  case 4:  transKey=RSCT_Key_CLR; break;
  default: transKey=RSCT_Key_Unknown; break;
  }

  ctx=(Ctapi_Context*) ccid;
  if (ctx) {
    /* TODO: check whether we need to transform the key code */
    commonKeyCallback(ctx, transKey);
  }
}
#endif



static void ecaKeyCallback(ctxPtr Context, uint8_t key) {
  struct Ctapi_Context *ctx;
  int transKey;

  ctx=(struct Ctapi_Context*) Context;

  DEBUGP(CT_INVALID_CTN, "ECA: Key=%d", key);
  switch(key) {
  case 0:  transKey=RSCT_Key_Digit; break;
  case 2:  transKey=RSCT_Key_Ok; break;
  case 4:  transKey=RSCT_Key_CLR; break;
  default: transKey=RSCT_Key_Unknown; break;
  }
  commonKeyCallback(ctx, transKey);
}



static int init() {
  /* init CTAPI configuration */
  if (_ctapi_init_count==0) {
    unsigned int nLevelMask=0;
    const char *s;

    DEBUGP(CT_INVALID_CTN, "config init");
    if (rsct_config_init())
      return -1;

    /* only initialize beep_struct once */
    if (!(rsct_config_get_flags() & CT_FLAGS_NO_BEEP)) {
      DEBUGP(CT_INVALID_CTN, "beep init");
      beepstruct=beep_init();
    }

    /* generic debug */
    if (rsct_config_get_flags() &
	(CT_FLAGS_DEBUG_GENERIC |
	 CT_FLAGS_DEBUG_READER)) {
      nLevelMask|=
	DEBUG_MASK_RESULTS |
	DEBUG_MASK_COMMUNICATION_ERROR;
    }

    /* ECA debugging */
    if (rsct_config_get_flags() & CT_FLAGS_DEBUG_ECA) {
      nLevelMask|=
	DEBUG_MASK_INPUT |
	DEBUG_MASK_OUTPUT |
	DEBUG_MASK_TRANSLATION;
    }

    if (rsct_config_get_flags() & CT_FLAGS_DEBUG_ECOM) {
      nLevelMask|=DEBUG_MASK_CJECOM;
    }

    if (rsct_config_get_flags() & CT_FLAGS_DEBUG_CJPPA) {
      nLevelMask|=DEBUG_MASK_PPA;
    }

    /* USB debug */
    if (rsct_config_get_flags() &
	(CT_FLAGS_DEBUG_AUSB |
	 CT_FLAGS_DEBUG_USB)) {
      nLevelMask|=
	DEBUG_MASK_COMMUNICATION_OUT |
	DEBUG_MASK_COMMUNICATION_IN |
	DEBUG_MASK_COMMUNICATION_ERROR |
	DEBUG_MASK_COMMUNICATION_INT;
    }

    /* misc debug */
    if (rsct_config_get_flags() & CT_FLAGS_DEBUG_CTAPI) {
      nLevelMask|=DEBUG_MASK_CTAPI;
    }
    if (rsct_config_get_flags() & CT_FLAGS_DEBUG_IFD) {
      nLevelMask|=DEBUG_MASK_IFD;
    }

    /* set resulting debug mask */
    Debug.setLevelMask(nLevelMask);

    /* set log file */
    s=rsct_config_get_debug_filename();
    if (s) {
      struct stat st;

      Debug.setLogFileName(s);

      /* check for log file size */
      if (stat(s, &st)==0) {
	if (st.st_size>CT_LOGFILE_LIMIT) {
	  if (truncate(s, 0)==0) {
	    DEBUGP(CT_INVALID_CTN, "Truncated log file");
	  }
	}
      }
    }

#ifdef ENABLE_NONSERIAL
    ausb_set_log_fn(ctapi_LogAusb);

    /* init usbdev interface */
    if (rsct_usbdev_init()<0) {
      DEBUGP(CT_INVALID_CTN, "Error on rsct_usbdev_init, maybe hald is not running?");
      return -1;
    }

    s=rsct_config_get_serial_filename();
    if (s==0) {
      struct passwd *p;

      /* get home dir */
      p=getpwuid(geteuid());
      if (p) {
	char pbuf[256];

	if (strlen(p->pw_dir)<sizeof(pbuf)) {
	  strcpy(pbuf, p->pw_dir);
	  strncat(pbuf, "/cyberjack_serials", sizeof(pbuf)-1);
	  pbuf[sizeof(pbuf)-1]=0;
	  rsct_config_set_serial_filename(pbuf);
	}
      }
      endpwent();
    }
#endif

    if (1) {
      char dbuf[256];

      snprintf(dbuf, sizeof(dbuf)-1,
	       "Initialised CTAPI library (%d.%d.%d.%d/%d.%d.%d.%d)%s%s%s%s",
	       CYBERJACK_VERSION_MAJOR,
	       CYBERJACK_VERSION_MINOR,
	       CYBERJACK_VERSION_PATCHLEVEL,
	       CYBERJACK_VERSION_BUILD,
	       PVER_MAJOR,
	       PVER_MINOR,
	       PVER_PATCHLEVEL,
	       PVER_BUILD,
#ifdef ENABLE_NONSERIAL
	       " nonserial",
#else
	       "",
#endif
#ifdef HAVE_HAL
	       " HAL",
#else
	       "",
#endif
#ifdef USE_USB1
	       " libusb1",
#else
	       " libusb0",
#endif
#ifdef HAVE_PTHREAD_H
	       " multithreading"
#else
	       ""
#endif
	      );
      dbuf[sizeof(dbuf)-1]=0;

      DEBUGP(CT_INVALID_CTN, dbuf);
    }
  }
  _ctapi_init_count++;

  return 0;
}



static void fini() {
  /* fini CTAPI configuration */
  if (_ctapi_init_count>0) {
    _ctapi_init_count--;
    if (_ctapi_init_count==0) {
      DEBUGP(CT_INVALID_CTN, "Deinitializing CTAPI library");

#ifdef ENABLE_NONSERIAL
      rsct_usbdev_fini();
#endif

      if (beepstruct) {
        beep_fini(beepstruct);
        beepstruct=NULL;
      }
      rsct_config_fini();

    }
  }
}



#ifdef ENABLE_NONSERIAL
static rsct_usbdev_t *_findUsbDevByName(rsct_usbdev_t *d,
					const char *s) {
  int vendorId, productId, busId, busPos;

  if (strstr(s, ":libusb:")!=NULL) {
    if (sscanf(s, "usb:%04x/%04x:libusb:%03d:%03d",
	       &vendorId, &productId, &busId, &busPos)!=4) {
      DEBUGP(CT_INVALID_CTN, "Bad device string [%s]", s);
      return NULL;
    }

    while(d) {
      if ((d->busId==(uint32_t)busId) &&
	  (d->busPos==(uint32_t)busPos) &&
	  (d->vendorId==(uint32_t)vendorId) &&
	  (d->productId==(uint32_t)productId))
	break;
      d=d->next;
    }
  }
  else if (strstr(s, ":libhal:")!=NULL) {
    const char *t;

    t=strstr(s, ":libhal:");
    t+=8;

    while(d) {
      if (d->halUDI && strcasecmp(t, d->halUDI)==0)
	break;
      d=d->next;
    }
  }

  return d;
}
#endif



extern "C" {


#ifdef ENABLE_NONSERIAL
  static int8_t _init_common1(uint16_t ctn, uint16_t pn,
			      const char *dname){
    Ctapi_Context *ctx;
    std::string devName;
    int rv;
    CReader *r;
    rsct_usbdev_t *devs=NULL;

    if (init()) {
      fprintf(stderr, "Could not init CTAPI driver\n");
      return CT_API_RV_ERR_MEMORY;
    }

    DEBUGP(CT_INVALID_CTN, "ctn=%d, pn=%d, devName=%s, %s",
	   ctn, pn, dname?dname:"none",
#ifdef HAVE_PTHREAD_H
           "multithreading enabled"
#else
	   "multithreading disabled"
#endif
	  );

    /* scan for devices */
    rv=rsct_usbdev_scan(&devs);
    if (rv<0) {
      DEBUGP(CT_INVALID_CTN, "Could not scan (%d)", rv);
      rsct_usbdev_list_free(devs);
      fini();
      return -10;
    }

    if (rsct_enum_serials_with_devs(rsct_config_get_serial_filename(), devs)){
      fprintf(stderr, "RSCT: Could not enumerate readers\n");
    }

    if (ctn==CT_INVALID_CTN) {
      DEBUGP(CT_INVALID_CTN, "Invalid context id");
      rsct_usbdev_list_free(devs);
      fini();
      return -127;
    }

    ctx=findContextByCtn(ctn);
    if (ctx) {
      DEBUGP(CT_INVALID_CTN, "Context %d id already in use", ctn);
      Debug.Out("CTAPI",
		DEBUG_MASK_CTAPI,
		"Context already in use (1)",0,0);
      rsct_usbdev_list_free(devs);
      fini();
      return -127;
    }

    if (pn) {
      if (findContextByPort(pn)) {
	DEBUGP(CT_INVALID_CTN, "Port %d already in use", pn);
	Debug.Out("CTAPI",
		  DEBUG_MASK_CTAPI,
		  "Port already in use (2)",0,0);
	rsct_usbdev_list_free(devs);
	fini();
	return -127;
      }

      if (pn>=0xa000) { /* serial devices */
	char *s;

	s=CSerialUnix::createDeviceName(pn-0xa000);
	if (s) {
	  devName=std::string(s);
	  free(s);
	}
	else {
	  DEBUGP(CT_INVALID_CTN, "Device %d not found", pn);
	  rsct_usbdev_list_free(devs);
	  fini();
	  return -127;
	}
      }
      else if ((pn>=0x9000 && pn<=0x9fff) ||
	       (pn>=1000 && pn<=1999))       { /* select by serial number */
	char serial[128];
	int rv;
        int relpn;

	if (pn>=0x9000)
	  relpn=pn-0x8fff;
        else
	  relpn=pn-999;

	rv=rsct_get_serial_for_port(relpn,
				    rsct_config_get_serial_filename(),
				    serial, sizeof(serial)-1);
	if (rv!=0) {
	  DEBUGP(CT_INVALID_CTN, "Device %d not found", pn);
	  rsct_usbdev_list_free(devs);
	  fini();
	  return -127;
	}
	else {
	  rsct_usbdev_t *d;

	  /* find device with the given serial number */
	  d=devs;
	  while(d) {
	    if (d->vendorId==0xc4b &&
		d->serial[0] &&
		(strcasecmp(serial, d->serial)==0)) {
	      break;
	    }
	    d=d->next;
	  }

	  if (d==NULL) {
	    DEBUGP(CT_INVALID_CTN, "Device %d [%s] not connected",
		   pn, serial);
	    rsct_usbdev_list_free(devs);
	    fini();
	    return -127;
	  }
	  else {
	    char ubuf[128];

	    snprintf(ubuf, sizeof(ubuf),
		     "usb:%04x/%04x:libusb:%03d:%03d",
		     d->vendorId,
		     d->productId,
		     d->busId,
		     d->busPos);

	    devName=std::string(ubuf);
	  }
	}
      }
      else if (pn>=0x8000) { /* /dev/ttyUSBx */
	rsct_usbdev_t *d;

	/* look for the device */
	d=devs;
	while(d) {
	  if (d->vendorId==0xc4b && d->port==pn-0x8000)
	    break;
	  d=d->next;
	}
	if (d==NULL) {
	  DEBUGP(CT_INVALID_CTN,
		 "Device %d [/dev/ttyUSB%d] not connected",
		 pn, pn-0x8000);
	  rsct_usbdev_list_free(devs);
	  fini();
	  return -127;
	}
	else {
	  char numbuf[32];

	  /* device found, create name */
	  snprintf(numbuf, sizeof(numbuf)-1, "%d", pn-0x8000);
	  devName="/dev/ttyUSB";
	  devName+=numbuf;
	}
      }
      else { /* get device with the given index number */
	int i;
	rsct_usbdev_t *d;

	i=0;
	d=devs;
	while(d) {
	  if (d->vendorId==0xc4b) {
	    if (++i==pn)
	      break;
	  }
	  d=d->next;
	}
	if (d==NULL) {
	  DEBUGP(CT_INVALID_CTN,
		 "Device %d not connected", pn);
	  rsct_usbdev_list_free(devs);
	  fini();
	  return -127;
	}
	else {
	  char ubuf[128];

	  if (d->productId==0x100 && d->port!=-1 &&
	      (rsct_config_get_flags() & CT_FLAGS_ECOM_KERNEL)) {
	    /* use kernel module for 0x100 */
	    snprintf(ubuf, sizeof(ubuf)-1, "%d", d->port);
	    devName="/dev/ttyUSB";
	    devName+=ubuf;
	  }
	  else {
	    snprintf(ubuf, sizeof(ubuf),
		     "usb:%04x/%04x:libusb:%03d:%03d",
		     d->vendorId,
		     d->productId,
		     d->busId,
		     d->busPos);

	    devName=std::string(ubuf);
	  }
	}
      }
    } /* if port number */
    else {
      devName=std::string(dname);
    }

    DEBUGP(CT_INVALID_CTN, "Device path is: [%s]", devName.c_str());

    if (strncasecmp(devName.c_str(), "usb:", 4)==0) {
      if (strstr(devName.c_str(), "0c4b/0100")) {
	CJ_INFO *ci=NULL;
	int rv;

	/* old Ecom/Pinpad, check whether we need to use kernel mode */
	if (rsct_config_get_flags() & CT_FLAGS_ECOM_KERNEL) {
	  rsct_usbdev_t *d;

	  /* kernel mode is requested, find device descriptor to get the
	   * /dev/ttyUSBx port */
	  d=_findUsbDevByName(devs, devName.c_str());
	  if (d==NULL) {
	    DEBUGP(ctn, "Device [%s] not found", devName.c_str());
	    rsct_usbdev_list_free(devs);
	    fini();
	    return -127;
	  }
	  else {
	    char dbuf[32];

	    /* descriptor must contain a valid port setting for ttyUSBx */
	    if (d->port==-1) {
	      DEBUGP(ctn, "Device [%s] has no ttyUSB port", devName.c_str());
	      rsct_usbdev_list_free(devs);
	      fini();
	      return -127;
	    }

	    /* change device name */
	    snprintf(dbuf, sizeof(dbuf)-1, "/dev/ttyUSB%d", d->port);
	    devName=std::string(dbuf);

	    /* init device using kernel module */
	    rv=cjecom_CT_initKernel(devName.c_str(), &ci);
	    if (rv) {
	      DEBUGP(ctn, "Unable to init device [%s] (%d)",
		     devName.c_str(), rv);
	      rsct_usbdev_list_free(devs);
	      fini();
	      return rv;
	    }
	  }
	}
	else {
	  /* old cyberjack or ecom */
	  rv=cjecom_CT_initUser2(ctn, devName.c_str(), &ci,
				 (rsct_config_get_flags() &
				  CT_FLAGS_RESET_BEFORE)?1:0);
	  if (rv) {
	    rsct_usbdev_list_free(devs);
	    fini();
	    return rv;
	  }
	}
  
	/* create context */
	ctx=new Ctapi_Context();
	ctx->oldEcom=ci;
	ctx->ctn=ctn;
	ctx->port=pn;
	ctx->keyCallback=NULL;
	ctx->readerName=devName;
	addContext(ctx);
  
	/* set key callback */
	cjecom_CT_keycb(ci, oldEcomKeyCallback);
  
	DEBUGP(ctn, "Ecom/Cyberjack 0x100 detected at %d [%s]",
	       pn, devName.empty()?"":devName.c_str());
      }
      else if (strstr(devName.c_str(), "0c4b/0300")) {
	CCID_DEVICE ccid;
  
	/* pre-create the context to be able to point to it */
	ctx=new Ctapi_Context();
	ctx->ctn=ctn;
	ctx->port=pn;
	ctx->keyCallback=NULL;
	ctx->readerName=devName;
  
	ccid=ctapiInit(devName.c_str(),
		       (CCID_CTX)ctx,
		       NULL, /* status callback */
		       ppaKeyCallback);
	if (ccid==NULL) {
	  fprintf(stderr, "CTAPI: Could not open device at %d\n", pn);
	  delete ctx;
	  rsct_usbdev_list_free(devs);
	  fini();
	  return -127;
	}
	ctx->ppa=ccid;
	addContext(ctx);
  
	DEBUGP(ctn, "Cyberjack pinpad_a detected at %d", pn);
      }
      else {
	/* driven by new driver */
	r=new CReader(devName.c_str());
	rv=r->Connect();
	if (rv!=CJ_SUCCESS) {
	    DEBUGP(CT_INVALID_CTN,
		   "Unable to connect device %d [%s]: %d",
		   pn, devName.c_str(), rv);
	  delete r;
	  rsct_usbdev_list_free(devs);
          fini();
	  return -127;
	}
    
	ctx=new Ctapi_Context();
	ctx->reader=r;
	ctx->ctn=ctn;
	ctx->port=pn;
	ctx->keyCallback=NULL;
	ctx->readerName=devName;
	addContext(ctx);
    
	r->SetKeyInterruptCallback(ecaKeyCallback, (ctxPtr) ctx);
    
	if (0) {
	  CJ_RESULT res;
	  cj_ReaderInfo ri;
    
	  ri.SizeOfStruct=sizeof(ri);
	  res=r->CtGetReaderInfo(&ri);
	  if (res!=SCARD_S_SUCCESS) {
	    DEBUGP(ctn, "Reader info not available (%d)", rv);
	  }
	  else {
	    DEBUGP(ctn, "Reader info available:");
	    if (ri.ContentsMask & RSCT_READER_MASK_VERSION) {
	      DEBUGP(ctn, "Version: %x", ri.Version);
	    }
	    if (ri.ContentsMask & RSCT_READER_MASK_HARDWARE_VERSION) {
	      DEBUGP(ctn, "HW-Version: %d", ri.HardwareVersion);
	    }
	    if (ri.ContentsMask & RSCT_READER_MASK_SERIALNUMBER) {
	      DEBUGP(ctn, "SerialNumber: %s", ri.SeriaNumber);
	    }
	    if (ri.ContentsMask & RSCT_READER_MASK_PRODUCTION_DATE) {
	      DEBUGP(ctn, "Production Date: %s", ri.ProductionDate);
	    }
	    if (ri.ContentsMask & RSCT_READER_MASK_COMMISSIONING_DATE) {
	      DEBUGP(ctn, "Commission Date: %s", ri.CommissioningDate);
	    }
	    if (ri.ContentsMask & RSCT_READER_MASK_TEST_DATE) {
	      DEBUGP(ctn, "Test Date: %s", ri.TestDate);
	    }
	  }
	}
      }
    } /* if "usb:" */
    else {
      if (strncasecmp(devName.c_str(), "/dev/ttyUSB", 11)==0) {
	CJ_INFO *ci;
	int rv;

	/* old cyberjack or ecom in kernel mode */
	rv=cjecom_CT_initKernel(devName.c_str(), &ci);
	if (rv) {
	  DEBUGP(ctn, "Unable to init device [%s] (%d)", devName.c_str(), rv);
	  rsct_usbdev_list_free(devs);
	  fini();
	  return rv;
	}

	/* create context */
	ctx=new Ctapi_Context();
	ctx->oldEcom=ci;
	ctx->ctn=ctn;
	ctx->port=pn;
	ctx->keyCallback=NULL;
	ctx->readerName=devName;
	addContext(ctx);

	/* set key callback */
	cjecom_CT_keycb(ci, oldEcomKeyCallback);

	DEBUGP(ctn, "Ecom/Cyberjack 0x100 detected at [%s] (kernel mode)",
	       devName.c_str());
      }
      else {
	/* serial reader driven by new driver */
	r=new CReader(devName.c_str());
	rv=r->Connect();
	if (rv!=CJ_SUCCESS) {
	  DEBUGP(CT_INVALID_CTN, "Unable to connect device %d", pn);
	  delete r;
	  rsct_usbdev_list_free(devs);
	  fini();
	  return -127;
	}
    
	ctx=new Ctapi_Context();
	ctx->reader=r;
	ctx->ctn=ctn;
	ctx->port=pn;
	ctx->keyCallback=NULL;
	ctx->readerName=devName;
	addContext(ctx);
    
	r->SetKeyInterruptCallback(ecaKeyCallback, (ctxPtr) ctx);

	DEBUGP(ctn, "Ecom A detected at [%s]", devName.c_str());

	if (0) {
	  CJ_RESULT res;
	  cj_ReaderInfo ri;
    
	  ri.SizeOfStruct=sizeof(ri);
	  res=r->CtGetReaderInfo(&ri);
	  if (res!=SCARD_S_SUCCESS) {
	    DEBUGP(ctn, "Reader info not available (%d)", rv);
	  }
	  else {
	    DEBUGP(ctn, "Reader info available:");
	    if (ri.ContentsMask & RSCT_READER_MASK_VERSION) {
	      DEBUGP(ctn, "Version: %x", ri.Version);
	    }
	    if (ri.ContentsMask & RSCT_READER_MASK_HARDWARE_VERSION) {
	      DEBUGP(ctn, "HW-Version: %d", ri.HardwareVersion);
	    }
	    if (ri.ContentsMask & RSCT_READER_MASK_SERIALNUMBER) {
	      DEBUGP(ctn, "SerialNumber: %s", ri.SeriaNumber);
	    }
	    if (ri.ContentsMask & RSCT_READER_MASK_PRODUCTION_DATE) {
	      DEBUGP(ctn, "Production Date: %s", ri.ProductionDate);
	    }
	    if (ri.ContentsMask & RSCT_READER_MASK_COMMISSIONING_DATE) {
	      DEBUGP(ctn, "Commission Date: %s", ri.CommissioningDate);
	    }
	    if (ri.ContentsMask & RSCT_READER_MASK_TEST_DATE) {
	      DEBUGP(ctn, "Test Date: %s", ri.TestDate);
	    }
	  }
	}
      }
    }
    rsct_usbdev_list_free(devs);
    return 0;
  }

#else

  static int8_t _init_common1(uint16_t ctn, uint16_t pn,
			      const char *dname){
    Ctapi_Context *ctx;
    std::string devName;
    int rv;
    CReader *r;

    if (init()) {
      fprintf(stderr, "Could not init CTAPI driver\n");
      return CT_API_RV_ERR_MEMORY;
    }

    DEBUGP(CT_INVALID_CTN, "ctn=%d, pn=%d, devName=%s, %s",
	   ctn, pn, dname?dname:"none",
#ifdef HAVE_PTHREAD_H
           "multithreading enabled"
#else
	   "multithreading disabled"
#endif
	  );

    if (ctn==CT_INVALID_CTN) {
      DEBUGP(CT_INVALID_CTN, "Invalid context id");
      fini();
      return -127;
    }

    ctx=findContextByCtn(ctn);
    if (ctx) {
      DEBUGP(CT_INVALID_CTN, "Context %d id already in use", ctn);
      Debug.Out("CTAPI",
		DEBUG_MASK_CTAPI,
		"Context already in use (3)",0,0);
      fini();
      return -127;
    }

    if (pn) {
      if (findContextByPort(pn)) {
	DEBUGP(CT_INVALID_CTN, "Port %d already in use", pn);
	Debug.Out("CTAPI",
		  DEBUG_MASK_CTAPI,
		  "Port already in use",0,0);
	fini();
	return -127;
      }

      if (pn>=0xa000) { /* serial devices */
	char *s;

	s=CSerialUnix::createDeviceName(pn-0xa000);
	if (s) {
	  devName=std::string(s);
	  free(s);
	}
	else {
	  DEBUGP(CT_INVALID_CTN, "Device %d not found", pn);
	  fini();
	  return -127;
	}
      }
      else {
	DEBUGP(CT_INVALID_CTN,
	       "Serial-only driver, only accepts ports >0xa000");
	fini();
	return -127;
      }
    }
    else {
      DEBUGP(CT_INVALID_CTN,
	     "Serial-only driver, only accepts ports >0xa000");
    }

    DEBUGP(CT_INVALID_CTN, "Device path is: [%s]", devName.c_str());
    /* serial reader driven by new driver */
    r=new CReader(devName.c_str());
    rv=r->Connect();
    if (rv!=CJ_SUCCESS) {
      DEBUGP(CT_INVALID_CTN, "Unable to connect device %d", pn);
      delete r;
      fini();
      return -127;
    }

    ctx=new Ctapi_Context();
    ctx->reader=r;
    ctx->ctn=ctn;
    ctx->port=pn;
    ctx->keyCallback=NULL;
    ctx->readerName=devName;
    addContext(ctx);

    r->SetKeyInterruptCallback(ecaKeyCallback, (ctxPtr) ctx);

    DEBUGP(ctn, "Ecom A detected at [%s]", devName.c_str());
    if (0) {
      CJ_RESULT res;
      cj_ReaderInfo ri;

      ri.SizeOfStruct=sizeof(ri);
      res=r->CtGetReaderInfo(&ri);
      if (res!=SCARD_S_SUCCESS) {
	DEBUGP(ctn, "Reader info not available (%d)", rv);
      }
      else {
	DEBUGP(ctn, "Reader info available:");
	if (ri.ContentsMask & RSCT_READER_MASK_VERSION) {
	  DEBUGP(ctn, "Version: %x", ri.Version);
	}
	if (ri.ContentsMask & RSCT_READER_MASK_HARDWARE_VERSION) {
	  DEBUGP(ctn, "HW-Version: %d", ri.HardwareVersion);
	}
	if (ri.ContentsMask & RSCT_READER_MASK_SERIALNUMBER) {
	  DEBUGP(ctn, "SerialNumber: %s", ri.SeriaNumber);
	}
	if (ri.ContentsMask & RSCT_READER_MASK_PRODUCTION_DATE) {
	  DEBUGP(ctn, "Production Date: %s", ri.ProductionDate);
	}
	if (ri.ContentsMask & RSCT_READER_MASK_COMMISSIONING_DATE) {
	  DEBUGP(ctn, "Commission Date: %s", ri.CommissioningDate);
	}
	if (ri.ContentsMask & RSCT_READER_MASK_TEST_DATE) {
	  DEBUGP(ctn, "Test Date: %s", ri.TestDate);
	}
      }
    }

    return 0;
  }
#endif



  static int8_t _init_common2(uint16_t ctn, uint16_t pn,
			      const char *dname){
    int8_t rv;

    rv=_init_common1(ctn, pn, dname);
    if (rv==0) {
      uint8_t apdu[]={0x20, 0x13, 0x00, 0x46, 0x00};
      uint8_t responseBuffer[300];
      uint16_t lr;
      uint8_t sad, dad;

      lr=sizeof(responseBuffer);
      sad=CT_API_AD_HOST;
      dad=CT_API_AD_CT;
      rv=CT_data(ctn, &dad, &sad, 5, apdu, &lr, responseBuffer);
      if (rv) {
	DEBUGP(ctn, "Error retrieving reader information");
	CT_close(ctn);
        return rv;
      }
    }

    return rv;
  }



  int8_t CT_init(uint16_t ctn, uint16_t pn){
    int8_t res;

#if 0
    fprintf(stderr,
	    "Reiner SCT cyberJack Driver v%s initialising (%u, %u)\n",
	    CYBERJACK_VERSION_FULL_STRING,
	    (unsigned int)ctn, (unsigned int) pn);
#endif
    if (pn==0) {
      DEBUGP(CT_INVALID_CTN, "Invalid port");
      return -127;
    }
    if (ctn==CT_INVALID_CTN) {
      DEBUGP(CT_INVALID_CTN, "Invalid context id");
      return -127;
    }

#ifdef HAVE_PTHREAD_H
    pthread_mutex_lock(&ctapi_status_mutex);
#endif
    res=_init_common2(ctn, pn, NULL);
#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock(&ctapi_status_mutex);
#endif

    return res;
  }



  static int8_t CT_internal(Ctapi_Context *ctx,
                            uint8_t *dad,
                            uint8_t *sad,
                            uint16_t cmd_len,
                            const uint8_t *cmd,
                            uint16_t *response_len,
                            uint8_t *response){
    char res;

    if (ctx->reader!=NULL) {
      DEBUGL(ctx->ctn, "Sending", cmd_len, cmd);
      res=ctx->reader->CtData(dad, sad, cmd_len, cmd, response_len, response);
      DEBUGP(ctx->ctn, "Result: %d", res);
      if (res==0) {
	DEBUGL(ctx->ctn, "Received", *response_len, response);
      }
    }
#ifdef ENABLE_NONSERIAL
    else if (ctx->oldEcom!=NULL) {
      DEBUGL(ctx->ctn, "Sending", cmd_len, cmd);
      res=cjecom_CT_data(ctx->oldEcom,
			 dad, sad,
			 cmd_len, cmd,
			 response_len, response);
      DEBUGP(ctx->ctn, "Result: %d", res);
      if (res==0) {
        DEBUGL(ctx->ctn, "Received", *response_len, response);
      }
    }
    else if (ctx->ppa!=NULL) {
      DEBUGL(ctx->ctn, "Sending", cmd_len, cmd);
      res=ctapiData(ctx->ppa,
		    dad, sad,
		    cmd_len, cmd,
		    response_len, response);
      DEBUGP(ctx->ctn, "Result: %d", res);
      if (res==0) {
	DEBUGL(ctx->ctn, "Received", *response_len, response);
      }
    }
#endif
    else {
      /* device lost */
      DEBUGP(ctx->ctn, "Device lost");
      return -127;
    }

    switch(res){
    case 0:
    case -1:
    case -11:
      break;
    default:
      DEBUGP(ctx->ctn, "Device lost (rv=%d)", res);
      if (ctx->reader) {
	delete ctx->reader;
	ctx->reader=NULL;
      }
#ifdef ENABLE_NONSERIAL
      else if (ctx->oldEcom) {
	cjecom_CT_close(ctx->oldEcom);
	free(ctx->oldEcom);
	ctx->oldEcom=NULL;
      }
      else if (ctx->ppa) {
	ctapiClose(ctx->ppa);
	ctx->ppa=NULL;
      }
#endif
    }

    return res;
  }


  int8_t CT_data(uint16_t ctn,
		 uint8_t *dad,
		 uint8_t *sad,
		 uint16_t cmd_len,
		 const uint8_t *cmd,
		 uint16_t *response_len,
                 uint8_t *response){
    Ctapi_Context *ctx;
    int8_t res;

    ctx=findContextByCtn(ctn);
    if (ctx==NULL) {
      DEBUGP(CT_INVALID_CTN, "Context %d not open", ctn);
      return -128;
    }

    if (cmd_len<4) {
      DEBUGP(ctx->ctn, "APDU too short");
      return CT_API_RV_ERR_INVALID;
    }

    if (*response_len<2) {
      DEBUGP(ctx->ctn, "Response buffer too short");
      return CT_API_RV_ERR_MEMORY;
    }

    ctx->lock();

    if (ctx->currentDialog==NULL)
      showDialogIfRequired(ctx, *dad, *sad, cmd_len, cmd);

    if (*dad==CT_API_AD_DRIVER)
      res=CT_special(ctx, dad, sad, cmd_len, cmd, response_len, response);
    else
      res=CT_internal(ctx, dad, sad, cmd_len, cmd, response_len, response);

    if (ctx->currentDialog) {
      rsct_dialog_close(ctx->currentDialog);
      rsct_dialog_free(ctx->currentDialog);
      ctx->currentDialog=NULL;
    }

    ctx->unlock();

    return res;
  }


  /* non-threadsafe version of CT_close() */
  int8_t nt_close(uint16_t ctn){
    Ctapi_Context *ctx;

    DEBUGP(CT_INVALID_CTN, "Closing device %d", ctn);
    ctx=findContextByCtn(ctn);
    if (ctx==NULL) {
      return -128;
    }

    if (ctx->reader!=NULL) {
      ctx->reader->Disonnect();
      ct_context_list.remove(ctx);
      delete ctx;
      fini();
      return 0;
    }
#ifdef ENABLE_NONSERIAL
    else if (ctx->oldEcom!=NULL) {
      cjecom_CT_close(ctx->oldEcom);
      free(ctx->oldEcom);
      ctx->oldEcom=NULL;
      ct_context_list.remove(ctx);
      delete ctx;
      fini();
      return 0;
    }
    else if (ctx->ppa!=NULL) {
      ctapiClose(ctx->ppa); /* gets freed there */
      ctx->oldEcom=NULL;
      ct_context_list.remove(ctx);
      delete ctx;
      fini();
      return 0;
    }
#endif
    else {
      /* device lost */
      ct_context_list.remove(ctx);
      delete ctx;
      fini();
      return -127;
    }

  }


  int8_t CT_close(uint16_t ctn){
    int8_t res;

#ifdef HAVE_PTHREAD_H
    pthread_mutex_lock(&ctapi_status_mutex);
#endif
    res=nt_close(ctn);
#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock(&ctapi_status_mutex);
#endif
    return res;
  }


  int8_t rsct_setkeycb(uint16_t ctn, CT_KEY_CB cb, void *user_data) {
    Ctapi_Context *ctx;

    ctx=findContextByCtn(ctn);
    if (ctx==NULL) {
      Debug.Out("CTAPI",
		DEBUG_MASK_CTAPI,
		"Context not open",0,0);
      return -128;
    }

    if (ctx->reader==NULL
#ifdef ENABLE_NONSERIAL
	&& ctx->oldEcom==NULL
	&& ctx->ppa==NULL
#endif
       ) {
      /* device lost */
      Debug.Out("CTAPI",
		DEBUG_MASK_CTAPI,
		"Device lost",0,0);
      return -127;
    }

    ctx->keyCallback=cb;
    ctx->kcb_user_data=user_data;

    return CT_API_RV_OK;
  }



  int8_t rsct_init_name(uint16_t ctn, const char *devName) {
    int8_t res;

    DEBUGP(CT_INVALID_CTN, "Init device [%s]",
	   devName?devName:"<empty>");
    if (devName==NULL) {
      DEBUGP(CT_INVALID_CTN, "No device name given");
      return -127;
    }
    if (ctn==CT_INVALID_CTN) {
      DEBUGP(CT_INVALID_CTN, "Invalid context id");
      return -127;
    }

#ifdef HAVE_PTHREAD_H
    pthread_mutex_lock(&ctapi_status_mutex);
#endif
    res=_init_common2(ctn, 0, devName);
#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock(&ctapi_status_mutex);
#endif
    return res;
  }



  void rsct_version(uint8_t *vmajor,
		    uint8_t *vminor,
		    uint8_t *vpatchlevel,
		    uint16_t *vbuild) {
    if (vmajor)
      *vmajor=CYBERJACK_VERSION_MAJOR;
    if (vminor)
      *vminor=CYBERJACK_VERSION_MINOR;
    if (vpatchlevel)
      *vpatchlevel=CYBERJACK_VERSION_PATCHLEVEL;
    if (vbuild)
      *vbuild=CYBERJACK_VERSION_BUILD;
  }



  void rsct_log(uint16_t ctn,
		unsigned int what,
		const char *file, int line, const char *function,
		const char *format, ...) {
    char tbuf[512];
    char *p;
    unsigned int len;
    va_list ap;
    unsigned int i;

    /* prefix */
    snprintf(tbuf, sizeof(tbuf)-1,
	     "%s:%s:%d:",
	     file, function, line);
    len=strlen(tbuf);
    p=tbuf+len;
    len=sizeof(tbuf)-len;

    /* real message */
    va_start(ap, format);
    vsnprintf(p, len-1, format, ap);
    va_end(ap);
    tbuf[sizeof(tbuf)-1]=0;

    len=strlen(tbuf);
    /* remove newline chars */
    for (i=0; i<len; i++) {
      if (tbuf[i]=='\n')
	tbuf[i]=' ';
    }

    /* output message */
    if (ctn!=CT_INVALID_CTN) {
      Ctapi_Context *ctx;

      ctx=findContextByCtn(ctn);
      if (ctx && ctx->reader)
	ctx->reader->DebugLeveled(what, tbuf, NULL, 0);
      else
	Debug.Out("LOG",
		  what,
		  tbuf, NULL, 0);
    }
    else
      Debug.Out("LOG",
                what,
		tbuf, NULL, 0);
  }



  void rsct_log_bytes(uint16_t ctn,
		      unsigned int what,
		      const char *file, int line,
		      const char *function,
		      const char *hdr,
		      int datalen, const uint8_t *data) {
    char tbuf[512];
    unsigned int i;
    unsigned int len;

    /* prefix */
    snprintf(tbuf, sizeof(tbuf)-1,
	     "%s:%s:%d:%s",
	     file, function, line, hdr);
    len=strlen(tbuf);

    /* remove newline chars */
    for (i=0; i<len; i++) {
      if (tbuf[i]=='\n')
	tbuf[i]=' ';
    }

    /* output message */
    if (ctn!=CT_INVALID_CTN) {
      Ctapi_Context *ctx;

      ctx=findContextByCtn(ctn);
      if (ctx)
	Debug.Out((char*)ctx->readerName.c_str(),
		  what, tbuf, (char*)data, datalen);
      else
	Debug.Out("LOG",
		  what, tbuf, (char*)data, datalen);
    }
    else
      Debug.Out("LOG",
		what, tbuf, (char*)data, datalen);
  }


  uint32_t rsct_ifd_ioctl(uint16_t ctn,
			  uint32_t IoCtrlCode,
			  uint8_t *Input,
			  uint32_t InputLength,
			  uint8_t *Output,
			  uint32_t *OutputLength) {
    Ctapi_Context *ctx;
    RSCT_IFD_RESULT res;

    ctx=findContextByCtn(ctn);
    if (ctx==NULL) {
      DEBUGP(CT_INVALID_CTN, "Context %d not open", ctn);
      return 612; // IFD_COMMUNICATION_ERROR
    }

    if (ctx->reader==NULL) {
      DEBUGP(ctn, "Reader is not driven by cjeca32");
      return 612; // IFD_COMMUNICATION_ERROR
    }

    res=ctx->reader->IfdIoControl(IoCtrlCode,
				  Input,
				  InputLength,
				  Output,
				  OutputLength);
    if (res!=0) {
      DEBUGP(ctn, "Error on IfdIoControl: %d", (int) res);
    }
    return res;
  }


} /* extern C */


/* include handling for special APDUs */
#include "ctapi_special.cpp"


