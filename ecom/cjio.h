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
 * File: cjio.h
 * CVS: $Id: cjio.h 54 2007-03-13 22:16:21Z martin $
 ***************************************************************************/

#ifndef H_CJ_IO
#define H_CJ_IO

#ifndef H_CJ
#error cj.h needs to be included before this.
#endif

#include "Debug.h"


/* Log file for debugging */
#define CJ_IO_LOGFILE	"/tmp/cj.log"

/* Reader type */
#define CJ_IO_TYPE_USB		1
#define CJ_IO_TYPE_LIBUSB	2

/* Default USB device */
#define CJ_IO_DEV_USB_0	"/dev/ttyUSB0"

/* Max devices */
#define CJ_IO_MAX_DEVICES	127

extern int logging_enabled;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Open device.
 * ci is a pointer to a CJ_INFO struct.
 * Returns CJ_EXIT_*.
 */
int cjIoOpen( CJ_INFO *ci, int reset_before);

/* Close device.
 * ci is a pointer to a CJ_INFO struct.
 * Returns CJ_EXIT_*.
 */
int cjIoClose( CJ_INFO *ci );

/* Send block to reader.
 * ci is a pointer to a CJ_INFO struct.
 * data contains a pointer to the data to transmit.
 * datalen contains the length of the data to transmit.
 * Returns CJ_EXIT_*.
 */
int cjIoSendBlock( CJ_INFO *ci, BYTE *data, int datalen );

/* Receive block from reader.
 * ci is a pointer to a CJ_INFO struct.
 * data contains a pointer to the buffer, which should receive the data.
 * datalen contains a pointer to an int, which must contain on calling the
 * size of the buffer and which returns the length of data received.
 * Returns CJ_EXIT_*.
 */
int cjIoReceiveBlock( CJ_INFO *ci, BYTE *data, int *datalen );

#ifdef __cplusplus
}
#endif /* __cplusplus */


#if 1
#define DEBUGP(format, ...) \
  rsct_log(CT_INVALID_CTN, DEBUG_MASK_CJECOM, __FILE__, __LINE__, __FUNCTION__, format , ##__VA_ARGS__)
#define DEBUGL(text, pData, lData) \
  rsct_log_bytes(CT_INVALID_CTN, DEBUG_MASK_CJECOM, __FILE__, __LINE__, __FUNCTION__, text, lData, pData)
#else
# define DEBUGP(format, args...)
# define DEBUGL(text, pData, lData)
#endif

#endif /* H_CJ_IO */

