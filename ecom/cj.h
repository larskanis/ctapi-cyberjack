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
 * File: cj.h
 * CVS: $Id: cj.h 20 2006-10-04 21:18:50Z martin $
 ***************************************************************************/

#ifndef H_CJ
#define H_CJ

#include "Platform.h"
#include <limits.h>

#include "config_l.h"

/* Return codes */
#define CJ_EXIT_OK			0
#define CJ_EXIT_UNDEF_ERROR		-1
#define CJ_EXIT_BAD_PARAM		-2
#define CJ_EXIT_IO_ERROR		-3
#define CJ_EXIT_FILE_NOT_EXIST		-4
#define CJ_EXIT_FILE_ERROR		-5
#define CJ_EXIT_PROTOCOL_ERROR		-6
#define CJ_EXIT_TIMEOUT			-7

typedef int BOOLEAN;
//typedef unsigned char BYTE;
typedef unsigned short SHORT;
//typedef unsigned long LONG;

#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE    !0
#endif

#ifndef max
#define max( a, b )   ( ( ( a ) > ( b ) ) ? ( ( int ) a ) : ( ( int ) b ) )
#endif /* !max */

#ifndef min
#define min( a, b )   ( ( ( a ) < ( b ) ) ? ( ( int ) a ) : ( ( int ) b ) )
#endif /* !min */

typedef struct cj_t1_info {
	BYTE	nad;		/* NAD */
	BYTE	ns;		/* N(S) */
	BYTE	nr;		/* N(R) */
	BYTE	ifsc;		/* Information Field Size Card */
	BYTE	ifsd;		/* Information Field Size Device */
	BOOLEAN	ifsreq;		/* S(IFS Req) already sent? */
	LONG	cwt;		/* Character Waiting Time in etu -11 etu */
	LONG	wtx;		/* Wait actually cwt*wtx */
	LONG	bwt;		/* Block Waiting Time in us */
} CJ_T1_INFO;

/* Struct, which contains all neccessary information for IO and
 * Protocol.
 */

typedef struct cj_info {
  /* Type */
  int type;	/* Type of reader */
  /* Portnumber */
  int pn;
  union {
    struct {
      /* Device name */
      char	device[PATH_MAX+1];
      /* File descriptor (USB) */
      int	fd;
    } kernel;
    struct {
      struct rsct_usbdev_t *dev;
      struct ausb_dev_handle *dh;
      char buffer[280];
      unsigned int rdtodo;
    } libusb;

  } ll;
  /* Key pressed callback */
  void	(* keycb)(struct cj_info *ci, int key);
  /* T=1PC */
  CJ_T1_INFO	t1;	/* T=1 protocol state */
  int ctn;
} CJ_INFO;

typedef struct cj_apdu {
	BYTE	nad;		/* Address byte */
	BYTE	*cmd;		/* C-APDU */
	int	cmdlen;		/* length of C-APDU */
	BYTE	*rsp;		/* R-APDU */
	int	rsplen;		/* length of R-APDU */
	int	maxRspLen;    	/* expected maximum length of R-APDU */
} CJ_APDU;

#endif /* H_CJ */

