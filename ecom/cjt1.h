/***************************************************************************
 * CT-API library for the REINER SCT cyberJack pinpad/e-com USB.
 * Copyright (C) 2001  Matthias Bruestle <m@mbsks.franken.de>
 * Modified for REINER SCT
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
 * File: cjt1.h
 * CVS: $Id: cjt1.h 20 2006-10-04 21:18:50Z martin $
 ***************************************************************************/

#ifndef CJ_T1_H
#define CJ_T1_H

#ifndef H_CJ
#error cj.h needs to be included before this.
#endif

#define CJ_T1_MAX_BLKLEN	3+255+1
#define CJ_T1_MAX_SBLKLEN	3+1+1

/* S-Block parameter */

#define	CJ_T1_S_RESYNCH		0x00
#define	CJ_T1_S_IFS			0x01
#define	CJ_T1_S_ABORT		0x02
#define	CJ_T1_S_WTX			0x03
/* Reader specific S-Blocks */
#define	CJ_T1_S_KEY			0x04
#define	CJ_T1_S_CARD		0x05

#define	CJ_T1_S_REQUEST		0x00
#define	CJ_T1_S_RESPONSE	0x01

#define CJ_T1_S_IFS_MAX		0xFF

#define CJ_T1_S_NAD_VALUE	0xE2

/* R-Block parameter */

#define	CJ_T1_R_OK				0x00
#define	CJ_T1_R_EDC_ERROR		0x01
#define	CJ_T1_R_OTHER_ERROR		0x02

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Returns LRC of data */
BYTE cjT1Lrc( const BYTE *data, int datalen);

/* Appends RC */
int cjT1AppendRc( CJ_T1_INFO *t1, BYTE *data, int *datalen );

/* Checks RC. */
BOOLEAN cjT1CheckRc( CJ_T1_INFO *t1, const BYTE *data, int datalen );

/* Builds S-Block */
int cjT1SBlock( CJ_T1_INFO *t1, int type, int dir, int param, BYTE *block,
	int *len );

/* Builds R-Block */
int cjT1RBlock( CJ_T1_INFO *t1, int type, BYTE *block, int *len );

/* Builds I-Block */
int cjT1IBlock( CJ_T1_INFO *t1, BOOLEAN more, const BYTE *data, int datalen,
	BYTE *block, int *blocklen );

/* Returns N(R) or N(S) from R/I-Block. */
int cjT1GetN( const BYTE *block );

/* Transmit APDU with protocol T=1 */
int cjT1SendCmd( CJ_INFO *ci, CJ_APDU *apdu );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CJ_T1_H */

