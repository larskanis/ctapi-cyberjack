/***************************************************************************
 * CT-API library for the REINER SCT cyberJack pinpad/e-com USB.
 * Copyright (C) 2001  REINER SCT
 * Author: Matthias Bruestle, Harald Welte
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
 * File: cjctapi.c
 * CVS: $Id: cjctapi.c 54 2007-03-13 22:16:21Z martin $
 ***************************************************************************/

#include "Platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cj.h"
#include "cjctapi.h"
#include "cjio.h"
#include "cjt1.h"

#include "ausb_l.h"

#include "ctapi-ecom.h"




int8_t cjCtapiRV( int ret ) {
  switch( ret ) {
  case CJ_EXIT_OK:
    return CT_API_RV_OK;
  case CJ_EXIT_UNDEF_ERROR:
    return CT_API_RV_ERR_TRANS;
  case CJ_EXIT_BAD_PARAM:
    return CT_API_RV_ERR_INVALID;
  case CJ_EXIT_IO_ERROR:
    return CT_API_RV_ERR_TRANS;
  case CJ_EXIT_FILE_NOT_EXIST:
    return CT_API_RV_ERR_HOST;
  case CJ_EXIT_FILE_ERROR:
    return CT_API_RV_ERR_HOST;
  case CJ_EXIT_PROTOCOL_ERROR:
    return CT_API_RV_ERR_TRANS;
  case CJ_EXIT_TIMEOUT:
    return CT_API_RV_ERR_TRANS;
  default:
    return CT_API_RV_ERR_TRANS;
  }
}



static int8_t cjecom_CT_initCommon(CJ_INFO *ci, int reset_before){
  int ret;
  /* For RESET CT */
  int8_t ctret;
  uint8_t dad = CT_API_AD_CT, sad = CT_API_AD_HOST, response[2];
  uint16_t lenr = sizeof(response);

  /* Set empty callback */
  ci->keycb = NULL;

  /* Open reader */
  ret = cjIoOpen(ci, reset_before);
  if (ret != CJ_EXIT_OK) {
    free(ci);
    return cjCtapiRV(ret);
  }

  /* RESET CT */
  ctret = cjecom_CT_data(ci, &dad, &sad, 4,
                         (unsigned char *)"\x20\x11\x00\x00", &lenr,
                         response);
  if ((ret!=CT_API_RV_OK) || (sad!=CT_API_AD_CT) ||
      (dad!=CT_API_AD_HOST) || (lenr!=2) || (response[0]!=0x90) ||
      (response[1]!=0x00) ) {
    cjecom_CT_close(ci);
    free(ci);
    if (ret != CT_API_RV_OK)
      return ret;
    return CT_API_RV_ERR_CT;
  }

  return CT_API_RV_OK;
}



int8_t cjecom_CT_initKernel(const char *filename,
			    CJ_INFO **cjret) {
  CJ_INFO *ci = malloc(sizeof(*ci));
  int ret;

  DEBUGP("using kernel module interface\n");

  if (!ci)
    return CT_API_RV_ERR_HOST;

  memset(ci, 0, sizeof(*ci));

  /* Set ci->type */
  ci->type = CJ_IO_TYPE_USB;
  /* Assemble device name */
  strncpy(ci->ll.kernel.device, filename, PATH_MAX);

  ret=cjecom_CT_initCommon(ci, 0);
  if (ret==0)
    *cjret=ci;

  return ret;
}



int8_t cjecom_CT_initUser(int ctn, int pn, CJ_INFO **cjret,
			  int reset_before) {
  rsct_usbdev_t *udev;
  CJ_INFO *ci;
  int ret;

  DEBUGP("using userspace interface\n");

  udev=rsct_usbdev_getDevByIdx(pn);
  if (udev==NULL) {
    fprintf(stderr, "CJECOM: Device %d not found\n", pn);
    return -127;
  }

  ci=malloc(sizeof(*ci));
  if (!ci)
    return CT_API_RV_ERR_HOST;
  memset(ci, 0, sizeof(*ci));

  /* Set ci->type */
  ci->type = CJ_IO_TYPE_LIBUSB;
  ci->ll.libusb.dev = udev;
  ci->pn=pn;
  ci->ctn=ctn;

  ret=cjecom_CT_initCommon(ci, reset_before);
  if (ret==0)
    *cjret=ci;

  return ret;
}



int8_t cjecom_CT_initUser2(int ctn, const char *devName,
			   CJ_INFO **cjret, int reset_before) {
  rsct_usbdev_t *udev;
  CJ_INFO *ci;
  int ret;

  DEBUGP("using userspace interface\n");

  udev=rsct_usbdev_getDevByName(devName);
  if (udev==NULL) {
    fprintf(stderr, "CJECOM: Device [%s] not found\n", devName);
    return -127;
  }

  ci=malloc(sizeof(*ci));
  if (!ci)
    return CT_API_RV_ERR_HOST;
  memset(ci, 0, sizeof(*ci));

  /* Set ci->type */
  ci->type = CJ_IO_TYPE_LIBUSB;
  ci->ll.libusb.dev = udev;
  ci->pn=0;
  ci->ctn=ctn;

  ret=cjecom_CT_initCommon(ci, reset_before);
  if (ret==0)
    *cjret=ci;

  return ret;
}



int8_t cjecom_CT_data(CJ_INFO *ci,
		      uint8_t *dad, uint8_t *sad,
		      uint16_t lenc, const uint8_t *command,
		      uint16_t *lenr, uint8_t *response){
  BYTE cmd[CJ_CTAPI_MAX_LENC];
  CJ_APDU apdu={ 0, cmd, 0, response, 0, *lenr };
  int ret;

  DEBUGP("APDU: %d bytes out, max %d bytes in)\n", lenc, *lenr);
#if 0
  /* Test sad/dad */
  if( (*sad!=CT_API_AD_HOST) || ((*dad!=CT_API_AD_CT) &&
				 (*dad!=CT_API_AD_ICC1)) )
    return CT_API_RV_ERR_INVALID;
#endif

  /* Test pointers */
  if( (command==NULL) || (response==NULL) ) {
    DEBUGP("Bad buffer pointers");
    return CT_API_RV_ERR_INVALID;
  }

  /* Test lengths */
  if( lenc>CJ_CTAPI_MAX_LENC ) {
    DEBUGP("Command too long (%d>%d)", lenc, CJ_CTAPI_MAX_LENC);
    return CT_API_RV_ERR_INVALID;
  }

  /* Assemble structures */
  memcpy( cmd, command, lenc );
  apdu.cmdlen = lenc;
  apdu.nad = ((*dad<<4)&0xF0) | (*sad&0x0F);

  /* Send command to reader */
  DEBUGP("Sending command...");
  ret = cjT1SendCmd(ci, &apdu );
  DEBUGP("Sending command... done (%d).", ret);
  if( ret!=CJ_EXIT_OK ) {
    return cjCtapiRV( ret );
  }

  /* Copy response */
  DEBUGP("Received %d bytes", apdu.rsplen);
  if( apdu.rsplen>*lenr ) {
    DEBUGP("Buffer too small (%d>%d)", apdu.rsplen, *lenr);
    return CT_API_RV_ERR_MEMORY;
  }
  *lenr = apdu.rsplen;
  *dad = (apdu.nad>>4) & 0x0F;
  *sad = apdu.nad & 0x0F;

  DEBUGP("Done");
  return CT_API_RV_OK;
}



int8_t cjecom_CT_close(CJ_INFO *ci){
  /* For EJECT ICC */
  uint8_t dad=CT_API_AD_CT, sad=CT_API_AD_HOST, response[2];
  uint16_t lenr=sizeof(response);

  /* EJECT ICC */
  cjecom_CT_data(ci, &dad, &sad, 4,
		 (unsigned char *)"\x20\x15\x01\x07", &lenr, response);

  cjIoClose(ci);

  return CT_API_RV_OK;
}



int8_t cjecom_CT_keycb(CJ_INFO *ci, void (*cb)(CJ_INFO *ci, int key) ){
  ci->keycb = cb;

  return CT_API_RV_OK;
}




