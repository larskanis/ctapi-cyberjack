/***************************************************************************
 * CT-API library for the REINER SCT cyberJack pinpad/e-com USB.
 * Copyright (C) 2001  REINER SCT
 * Author: Matthias Bruestle
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
 * File: ctapi.h
 * CVS: $Id: ctapi-ecom.h 20 2006-10-04 21:18:50Z martin $
 ***************************************************************************/

#ifndef H_CTAPI_ECOM
#define H_CTAPI_ECOM

#include "cyberjack_l.h"
#include "cj.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int8_t cjecom_CT_initKernel(const char *filename, CJ_INFO **cjret);
int8_t cjecom_CT_initUser(int ctn, int pn, CJ_INFO **cjret, int reset_before);
int8_t cjecom_CT_initUser2(int ctn, const char *devName,
			   CJ_INFO **cjret,
			   int reset_before);

int8_t cjecom_CT_data(struct cj_info *ci,
		      uint8_t *dad, uint8_t *sad,
		      uint16_t lenc, const uint8_t *command,
		      uint16_t *lenr, uint8_t *response);

int8_t cjecom_CT_close(struct cj_info *ci);

/* Proprietary extension */
int8_t cjecom_CT_keycb(struct cj_info *ci,
		       void (* cb)(struct cj_info *ci, int key) );

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* H_CTAPI */

