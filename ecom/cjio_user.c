/***************************************************************************
 * CT-API library for the REINER SCT cyberJack pinpad/e-com USB.
 * Copyright (C) 2005  REINER SCT
 * Author: Harald Welte
 * Support: support@reiner-sct.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * File: cjio.c
 * CVS: $Id: cjio_user.c 54 2007-03-13 22:16:21Z martin $
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
/* #include <sys/stat.h> */
#include <sys/time.h>
/* #include <sys/types.h> */
#include <time.h>
#include "../ausb/ausb_l.h"

#include "cj.h"
#include "cjio.h"

#include "Debug.h"



/* Open device.
 * ci is a pointer to a CJ_INFO struct.
 * Returns CJ_EXIT_*.
 */
int cjIoOpen_libusb(CJ_INFO *ci, int reset_before){
  BYTE buffer[7];
  int ret, len;
  char name[PATH_MAX+1];

  if (!ci)
    return CJ_EXIT_BAD_PARAM;

  /* Currently only USB supported */
  if (ci->type != CJ_IO_TYPE_LIBUSB)
    return CJ_EXIT_BAD_PARAM;

  DEBUGP("trying to open device\n");
  ci->ll.libusb.dh=ausb_open(ci->ll.libusb.dev, 1);
  if (!ci->ll.libusb.dh) {
    DEBUGP("Error on ausb_open()\n");
    return CJ_EXIT_FILE_ERROR;
  }

  /* detach kernel driver if any */
  ret=ausb_get_kernel_driver_name(ci->ll.libusb.dh, 0, name, PATH_MAX);
  if (ret<0) {
    DEBUGP("Can't determine the current kernel driver using the device, assuming none.\n");
  }
  else {
    if (ret>0) {
      DEBUGP("driver `%s' using the interface\n", name);
      if (strcmp(name, "cyberjack")) {
	fprintf(stderr, "CJECOM: not detaching unknown driver `%s'\n",
		name);
	DEBUGP("Not detaching driver \"%s\"\n", name);
	return CJ_EXIT_FILE_ERROR;
      }
      ret=ausb_detach_kernel_driver(ci->ll.libusb.dh, 0);
      if (ret< 0) {
	DEBUGP("unable to detach kernel driver (%d)\n", ret);
	return CJ_EXIT_FILE_ERROR;
      }
    }
    else {
      DEBUGP("No kernel driver is using the device.\n");
    }
  }

  if (ausb_set_configuration(ci->ll.libusb.dh, 1)) {
    DEBUGP("unable to set usb configuration\n");
    return CJ_EXIT_FILE_ERROR;
  }

  /* try to claim interface */
  DEBUGP("claim interface\n");
  ret=ausb_claim_interface(ci->ll.libusb.dh, 0);
  if (ret<0) {
    DEBUGP("Unable to claim interface (%d), checking for kernel driver\n",
	   ret);
    /* detach kernel driver */
    if (ausb_get_kernel_driver_name(ci->ll.libusb.dh, 0, name, PATH_MAX)>=0) {
      DEBUGP("driver `%s' using the interface\n", name);
      if (strcmp(name, "cyberjack")) {
	fprintf(stderr, "CJECOM: not detaching unknown driver `%s'\n",
		name);
        DEBUGP("Not detaching driver \"%s\"\n", name);
	return CJ_EXIT_FILE_ERROR;
      }
      ret=ausb_detach_kernel_driver(ci->ll.libusb.dh, 0);
      if (ret< 0) {
	DEBUGP("unable to detach kernel driver (%d)\n", ret);
	return CJ_EXIT_FILE_ERROR;
      }
      DEBUGP("claim interface (2nd try)\n");
      ret=ausb_claim_interface(ci->ll.libusb.dh, 0);
      if (ret<0) {
	fprintf(stderr, "CJECOM: Unable to claim interface\n");
	DEBUGP("unable to claim interface (%d)", ret);
	return CJ_EXIT_FILE_ERROR;
      }
    }
    else {
      DEBUGP("No kernel driver is using the device...\n");
      return CJ_EXIT_FILE_ERROR;
    }
  }
  DEBUGP("successfully claimed interface\n");

  ausb_clear_halt(ci->ll.libusb.dh, 0x81);
  ausb_clear_halt(ci->ll.libusb.dh, 0x02);
  ausb_clear_halt(ci->ll.libusb.dh, 0x82);

  if (reset_before) {
    DEBUGP("resetting device upon request\n");
    ausb_reset(ci->ll.libusb.dh);
  }

  //fprintf(stderr, "start interrupt\n");
  if (ausb_start_interrupt(ci->ll.libusb.dh, 0x81)) {
    ausb_close(ci->ll.libusb.dh);
    ci->ll.libusb.dh=NULL;
    fprintf(stderr, "CJECOM: Unable to start interrupt pipe\n");
    DEBUGP("Unable to start interrupt pipe\n");
    return CJ_EXIT_IO_ERROR;
  }

  /* Fill CJ_INFO */
  ci->type = CJ_IO_TYPE_LIBUSB;
  ci->t1.ns = 0;
  ci->t1.nr = 0;
  ci->t1.ifsc = 0xFF;
  ci->t1.ifsd = 0xFF;
  ci->t1.ifsreq = FALSE;
  ci->t1.bwt = 8000000L;
  ci->t1.wtx = 0L;
  ci->t1.cwt = 100000L;	/* Not really CWT */

  usleep(200000);

  ret = cjIoSendBlock(ci, (unsigned char *) "\xE2\xC1\x00\x23", 4);
  if (ret < 0)
    return ret;

  /* Send a S(RESYCH) to resychronise with reader and to test if there
   * is a appropriate reader at the other end of the line.
   */
  ret = cjIoSendBlock(ci, (unsigned char *) "\xE2\xC0\x00\x22", 4);
  if (ret<0) {
    DEBUGP("Unable to send block (%d)\n", ret);
    return ret;
  }

  len=sizeof(buffer);
  ret = cjIoReceiveBlock(ci, buffer, &len);
  if (ret < 0) {
    DEBUGP("Unable to send block (%d)\n", ret);
    return ret;
  }
  if (len != 4) {
    DEBUGP("Unexpected length of response (got %d bytes)\n", len);
    return CJ_EXIT_IO_ERROR;
  }

  /* Correct response? */
  if (memcmp(buffer, "\x2E\xE0\x00\xCE", 4)) {
    DEBUGP("Unexpected response\n");
    return CJ_EXIT_IO_ERROR;
  }

  return CJ_EXIT_OK;
}



/* Close device.
 * ci is a pointer to a CJ_INFO struct.
 * Returns CJ_EXIT_*.
 */
int cjIoClose_libusb(CJ_INFO *ci){
  int ret;

  if (!ci)
    return CJ_EXIT_BAD_PARAM;

  ret = cjIoSendBlock(ci, (unsigned char*) "\xE2\xC1\x00\x23", 4);

  ausb_stop_interrupt(ci->ll.libusb.dh);
  ausb_reset(ci->ll.libusb.dh);
  ausb_release_interface(ci->ll.libusb.dh, 0);
  ausb_reattach_kernel_driver(ci->ll.libusb.dh, 0);
  ausb_close(ci->ll.libusb.dh);

  if (ret < 0)
    return ret;

  return CJ_EXIT_OK;
}



/* Send block to reader.
 * ci is a pointer to a CJ_INFO struct.
 * data contains a pointer to the data to transmit.
 * datalen contains the length of the data to transmit.
 * Returns CJ_EXIT_*.
 */
int cjIoSendBlock_libusb(CJ_INFO *ci, BYTE *data, int datalen){
  BYTE buffer[3+3+255+1];
  int ptr, size, ret;
  int timeout = 1000;

  buffer[0] = 0x00;
  /* Quick fix. 1-byte blocks are not possible with T=1. */
  if (datalen == 1) buffer[0]=0xFF;
  buffer[1] = datalen & 0xFF;
  buffer[2] = (datalen>>8) & 0xFF;

  memcpy(buffer+3, data, datalen);

  datalen += 3;

#if 0
  rsct_log_bytes(CT_FLAGS_DEBUG_TRANSFER,
		 __FILE__, __LINE__, __FUNCTION__,
		 "PC->CYBJCK", datalen, data);
#endif

  for (ptr = 0; ptr < datalen; ptr += 64) {
    size = min(64, datalen-ptr);
    DEBUGP("write(buffer+%d,%d)\n", ptr, size);

    ret = ausb_bulk_write(ci->ll.libusb.dh, 0x02, (char *) buffer+ptr,
                          size, timeout);
    if (ret != size) {
      DEBUGP("write(buffer+%d,%d) sent %d bytes\n",
             ptr, size, ret);
      /* return CJ_EXIT_IO_ERROR; */
    }
  }

  return CJ_EXIT_OK;
}



/* Receive block from reader.
 * ci is a pointer to a CJ_INFO struct.
 * data contains a pointer to the buffer, which should receive the data.
 * datalen contains a pointer to an int, which must contain on calling the
 * size of the buffer and which returns the length of data received.
 * Returns CJ_EXIT_*.
 * CAVE: This function does not return all the data read from the reader!
 * The first 3 bytes have a special meaning and are skipped:
 * - 0: unknown
 * - 1: block size (low byte)
 * - 2: block size (hig byte)
 * The rest after these 3 bytes is returned. It did cost me some time to
 * figure out what the caller expected here... (M.Preuss)
 */
int cjIoReceiveBlock_libusb(CJ_INFO *ci, BYTE *data, int *datalen){
  int timeout;
  int ret, len=0;
  LONG tmpbwt;
  BYTE ubuf[64];
  int blockSize=-1;

  DEBUGP("Reading %d bytes\n", *datalen);

  if (!ci || !data || !datalen)
    return CJ_EXIT_BAD_PARAM;

  usleep(20);

  if (ci->t1.wtx)
    tmpbwt = ci->t1.bwt*ci->t1.wtx;
  else
    tmpbwt = ci->t1.bwt;
  ci->t1.wtx = 0;

  /* Wait for first data with BWT */
  timeout = tmpbwt/1000;
  DEBUGP("timeout=%d\n", timeout);

  /* read header to get block size */
  len=0;
  while(len<3) {
    int size=min(64, sizeof(ubuf)-len);

    DEBUGP("Reading header (up to %d bytes)", size);
    ret=ausb_bulk_read(ci->ll.libusb.dh, 0x82,
		       (char *) ubuf+len,
		       size, timeout);
    DEBUGP("ausb_bulk_read=%d", ret);
    if (ret==0) {
      DEBUGP("Received 0 bytes, that can't be good...");
      return CJ_EXIT_PROTOCOL_ERROR;
    }

    /* Wait for next data with CWT (Is not really CWT.) */
    timeout = ci->t1.cwt/1000;
    len+=ret;
  }

  /* get block size */
  blockSize=((short)ubuf[2]<<8)+ubuf[1];
  DEBUGP("Block size is %d bytes", blockSize);
  len-=3;
  if (len>0) {
    /* we already got the first bytes of the body, copy them */
    DEBUGP("We already got %d bytes", len);
    if (len<=*datalen) {
      /* only copy if there is still enough room in caller's buffer */
      memmove(data, ubuf+3, len);
    }
    else {
      DEBUGP("Outside buffer boundaries, not copying data (%d>%d)",
	     len, *datalen);
    }
  }

  /* read rest of the body (if any) */
  while(len<blockSize) {
    int size=min(64, blockSize-len);
    //int size=64;

    DEBUGP("Reading body (up to %d bytes)", size);
    ret=ausb_bulk_read(ci->ll.libusb.dh, 0x82,
		       (char *) ubuf,
		       size, timeout);
    if (ret==0)
      break;
    if ((len+ret)>*datalen) {
      DEBUGP("Outside buffer boundaries, not copying data");
    }
    else {
      /* only copy if there is still enough room in caller's buffer */
      memmove(data+len, ubuf, ret);
    }

    len+=ret;
  }

  /* check number of received bytes */
  if (len>*datalen) {
    DEBUGP("Buffer too small (%d<%d)", *datalen, len);
    return CJ_EXIT_BAD_PARAM;
  }
  else {
    *datalen=len;
    DEBUGP("Received %d bytes", len);
    DEBUGL("Read:", data, len);
    return 0;
  }

  return CJ_EXIT_OK;
}


