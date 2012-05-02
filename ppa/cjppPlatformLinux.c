
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define CJPPA_DEBUG_TRANSFER

#define WANT_CJDEBUG 1
#include "cjppa.h"
#include "cjpp.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "../ausb/ausb_l.h"
#include <netinet/in.h>

#include "cjppa_linux.h"
#include "Debug.h"


#if 0
# define debug_out(format, args...) \
   rsct_log(CT_FLAGS_DEBUG_CJPPA, __FILE__, __LINE__, __FUNCTION__, format, ## args)
#else
# define debug_out(format, args...)
#endif


cjccidHANDLE AllHandles[512];


/* These functions must be implemented for different Platforms*/


static void handle_interrupt(const uint8_t *data,
                             uint32_t dlen,
			     void *userdata){
  cjccidHANDLE hDevice = userdata;

#if 0
  if (dlen < sizeof(CCID_Interrupt)) {
    debug_out("received short interrupt packet (%u bytes)\n",
	      uurb->actual_length);
    return;
  }
#endif

  HandleCyberJackInterruptData(hDevice,
			       (CCID_Interrupt*)data);

}



int cjppLinux_SetInterruptEventNotificationProc(cjccidHANDLE cjccid,
						ausb_dev_handle *ah){
  if (ausb_register_callback(ah, handle_interrupt, cjccid)) {
    debug_out("unable to register interrupt callback\n");
    return -1;
  }

  return 0;
}



HANDLE cjppCreate(const char *cDeviceName){
  rsct_usbdev_t *dev;
  ausb_dev_handle *hdl;

  if (rsct_config_get_flags() & CT_FLAGS_DEBUG_CJPPA)
    cjppDebugSetLevel(1);

  dev=rsct_usbdev_getDevByName(cDeviceName);
  if (!dev) {
    debug_out("unable to find cyberjack usb device\n");
    return NULL;
  }

  hdl=ausb_open(dev, 1); /* use implementation 1 (with interrupt pipe) */
  if (!hdl) {
    debug_out("unable to open usb device: %s\n", strerror(errno));
    return NULL;
  }

  if (ausb_set_configuration(hdl, 1)) {
    debug_out("unable to set usb configuration\n");
    return NULL;
  }

  if (ausb_claim_interface(hdl, 0)) {
    debug_out("unable to claim usb device\n");
    return NULL;
  }

  ausb_clear_halt(hdl, 0x81);
  ausb_clear_halt(hdl, 0x04);
  ausb_clear_halt(hdl, 0x85);

  if (ausb_start_interrupt(hdl, 0x81)) {
    debug_out("unable to start interrupts\n");
    return NULL;
  }


  return (HANDLE) hdl;
}




unsigned long cjppGetLocalInfo(void){
  /* FIXME */
  return 0xffffffff;
}



void cjppFillDevice(cjppHANDLE hcjppDevice){
  /* FIXME */
}


void cjccidClose(HANDLE cjppDevice){
  ausb_dev_handle *hdl;

  hdl = (ausb_dev_handle *) ((cjppHANDLE)cjppDevice)->hDevice;

  debug_out("releasing and closing ausb interface\n");
  ausb_stop_interrupt(hdl);
  /*ausb_clear_halt(hdl, 0x04);*/
  /*ausb_clear_halt(hdl, 0x85);*/
  /*ausb_reset(hdl); * don't reset the device, only old cyberJacks need that */
  ausb_release_interface(hdl, 0);
  ausb_close(hdl);

  /* cjccidClose frees the handle, so we don't */
}



unsigned short cjppSWAB_WORD(unsigned short Value){
  return htons(Value);
}



unsigned long cjppSWAB_DWORD(unsigned long Value){
  return htonl(Value);
}



static unsigned short cjppSWAP_ALWAYS(unsigned short Value){
  unsigned char b;
  b = (unsigned char)(Value>>8);
  Value <<= 8;
  return (unsigned short)(Value+b);
}



unsigned short cjppSWAB_WORD_2(unsigned short Value){
  Value = htons(Value);
  return cjppSWAP_ALWAYS(Value);
}



unsigned long cjppSWAB_DWORD_2(unsigned long Value){
  unsigned short w;
  Value = htonl(Value);
  w = cjppSWAP_ALWAYS((unsigned short)(Value>>16));
  Value = cjppSWAP_ALWAYS((unsigned short)Value);
  Value <<= 16;
  return Value+w;
}



void cjppSleep(unsigned long Value){
  usleep(Value*1000);
}




int cjppRead(HANDLE cjppDevice,CCID_Response *Response){
  int32_t bytesRead;
  ausb_dev_handle *ah;

  ah=(ausb_dev_handle *)((cjppHANDLE)cjppDevice)->hDevice;

  memset(Response, 0, sizeof(*Response));

  bytesRead = 0;
  while (bytesRead == 0 || memcmp(Response,"\x00\x00\x00\x00", 4) == 0) {
    debug_out("reading from bulk in pipe\n");
    bytesRead = ausb_bulk_read(ah, 0x85, (char *)Response,
			       sizeof(CCID_Response), USB_READ_TIMEOUT);
    if ( bytesRead <= 0 ) {
      debug_out("bytesRead == %d\n", bytesRead);
      debug_out("%s: returning ERR_DEVICE_LOST\n",
                __FUNCTION__);
      return CJPP_ERR_DEVICE_LOST;
      //return CJPP_SUCCESS;
    }
  }

#if 0
  rsct_log_bytes(CT_FLAGS_DEBUG_AUSB,
		 __FILE__, __LINE__, __FUNCTION__,
		 "CYBJCK->PC", bytesRead,
		 (unsigned char*) Response);
#endif

  debug_out("%s: returning SUCCESS (%u bytes)\n", __FUNCTION__,
	    bytesRead);
  return CJPP_SUCCESS;
}



int cjppWrite(HANDLE cjppDevice,CCID_Message *Message){
  unsigned char	buffer[274];
  int32_t	bytesToWrite, bytesWritten;

  memcpy(buffer,Message,270);

  bytesToWrite = 10+cjppSWAB_DWORD_2(Message->dwLength);
  debug_out("%s: writing %u bytes to bulk out pipe:\n", __FUNCTION__,
	    bytesToWrite);

#if 0
  rsct_log_bytes(CT_FLAGS_DEBUG_AUSB,
		 __FILE__, __LINE__, __FUNCTION__,
		 "PC->CYBJCK", bytesToWrite,
		 (unsigned char*) Message);
#endif

  bytesWritten = ausb_bulk_write((ausb_dev_handle *)((cjppHANDLE)cjppDevice)->hDevice, 0x04, (char *)buffer, bytesToWrite, USB_WRITE_TIMEOUT);
  if ( bytesWritten == -ENODEV) {
    debug_out("%s: returning ERR_DEVICE_LOST\n", __FUNCTION__);
    return CJPP_ERR_DEVICE_LOST;
  }
  if ( bytesWritten < bytesToWrite ) {
    debug_out("%s: wrote only %d of %u bytes\n", __FUNCTION__,
	      bytesWritten, bytesToWrite);
    debug_out("%s: returning ERR_WRITE_DEVICE\n", __FUNCTION__);
    return CJPP_ERR_WRITE_DEVICE;
  }

  debug_out("%s: returning SUCCESS\n", __FUNCTION__);
  return CJPP_SUCCESS;
}



int cjppTransfer(HANDLE cjppDevice, CCID_Message *Message,
		 CCID_Response *Response){
#ifdef CJPPA_DEBUG_TRANSFER
  unsigned char dad, sad;
  unsigned short lenc = 20+cjppSWAB_DWORD_2(Message->dwLength);
  unsigned char *cmd = (unsigned char *) Message;
  unsigned short lenr = sizeof(CCID_Response);
  unsigned char *response = (unsigned char *) Response;

  cjppDebugCommand(NULL, &sad, &dad, lenc, cmd, &lenr, response);
#endif

  CJPP_TEST(cjppWrite(cjppDevice,Message))
  CJPP_TEST(cjppRead(cjppDevice,Response))
#ifdef CJPPA_DEBUG_TRANSFER
  cjppDebugResponse(NULL, &dad, &sad, lenc, cmd, &lenr, response,
		    CJPP_SUCCESS);
#endif
  return CJPP_SUCCESS;
}



HANDLE cjppCreateThread(void (*ThreadRoutine)(void *),void *Params){
  // еее
  /*
   * DWORD ThreadId;
   return CreateThread(NULL,0,(  DWORD (WINAPI *)( LPVOID ))ThreadRoutine,Params,0,&ThreadId);
   */
  debug_out("cjppCreateThread - not implemented!\n");
  return NULL;
}



void cjppTerminateThread(HANDLE thread){
  // еее
  /*
   TerminateThread(thread,0);
   */
  debug_out("cjppTerminateThread - not implemented!\n");
}



unsigned long cjppGetUniqueID(void){
  /* FIXME */
  unsigned long Result;
  debug_out("cjppTerminateThread - not implemented!\n");
#if 0
  CFUUIDRef uuid;
  unsigned long Help;
  CFUUIDBytes		uuidBytes;

  uuid = CFUUIDCreate(NULL);
  if ( uuid == NULL ) {
    debug_out("cjppGetUniqueID: CFUUIDCreate failed!\n");
    return 0;
  }
  uuidBytes = CFUUIDGetUUIDBytes(uuid);
  CFRelease(uuid);
  memcpy(&Result,&uuidBytes.byte0,4);
  memcpy(&Help,&uuidBytes.byte4,4);
  Result^=Help;
  memcpy(&Help,&uuidBytes.byte8,4);
  Result^=Help;
  memcpy(&Help,&uuidBytes.byte12,4);
  Result^=Help;
#else
  Result=0;
#endif
  return Result;
}



