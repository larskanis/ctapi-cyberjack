/***************************************************************************
 * CT-API library for the REINER SCT cyberJack pinpad/e-com USB.
 * Copyright (C) 2001  Matthias Bruestle <m@mbsks.franken.de
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
 * File: cjt1.c
 * CVS: $Id: cjt1.c 54 2007-03-13 22:16:21Z martin $
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include <stdio.h>
#include <string.h>

#if defined(WINDOWS) && !defined(__BORLANDC__)
#include <memory.h>
#endif

#include "cj.h"
#include "cjctapi.h"
#include "cjio.h"
#include "cjt1.h"



/* Returns LRC of data */

BYTE cjT1Lrc( const BYTE *data, int datalen){
  BYTE lrc=0x00;
  int i;

  for( i=0; i<datalen; i++ ) lrc^=data[i];

  return lrc;
}



/* Appends RC */
int cjT1AppendRc( CJ_T1_INFO *t1, BYTE *data, int *datalen ){
  data[*datalen]=cjT1Lrc( data, *datalen );
  *datalen+=1;
  return( CJ_EXIT_OK );

  //return( CJ_EXIT_BAD_PARAM );
}



/* Checks RC. */
BOOLEAN cjT1CheckRc( CJ_T1_INFO *t1, const BYTE *data, int datalen ){
  BYTE rc[2];
  BYTE cmp[2];

  /* Check LEN. */
  if( (data[2]+3+1)!=datalen) {
    DEBUGP("Bad response: Length reported does not match (%d!=%d)\n",
	   data[2]+3+1, datalen);
    return( FALSE );
  }

  rc[1]=data[datalen-1];
  cmp[1] = cjT1Lrc( data, datalen-1 );
  if( rc[1]==cmp[1] ) {
    DEBUGP("T1 checksum ok");
    return( TRUE );
  }

  DEBUGP("T1 checksum not ok");
  return( FALSE );
}



/* Builds S-Block */
int cjT1SBlock( CJ_T1_INFO *t1, int type, int dir, int param, BYTE *block,
               int *len ){
  int ret;

  block[0]=CJ_T1_S_NAD_VALUE;

  switch( type ) {
  case CJ_T1_S_RESYNCH:
    if( dir==CJ_T1_S_REQUEST ) block[1]=0xC0;
    else block[1]=0xE0;
    block[2]=0x00;
    *len=3;
    if( (ret=cjT1AppendRc( t1, block, len )) ) return( ret );
    break;

  case CJ_T1_S_IFS:
    if( dir==CJ_T1_S_REQUEST ) block[1]=0xC1;
    else block[1]=0xE1;
    block[2]=0x01;
    block[3]=(BYTE) param;
    *len=4;
    if( (ret=cjT1AppendRc( t1, block, len )) ) return( ret );
    break;

  case CJ_T1_S_ABORT:
    if( dir==CJ_T1_S_REQUEST ) block[1]=0xC2;
    else block[1]=0xE2;
    block[2]=0x00;
    *len=3;
    if( (ret=cjT1AppendRc( t1, block, len )) ) return( ret );
    break;

  case CJ_T1_S_WTX:
    if( dir==CJ_T1_S_REQUEST ) block[1]=0xC3;
    else block[1]=0xE3;
    if( param ) {
      block[2]=0x01;
      block[3]=(BYTE) param;
      *len=4;
    }
    else {
      /* Special reader S-Block */
      block[2]=0x00;
      *len=3;
    }
    if( (ret=cjT1AppendRc( t1, block, len )) ) return( ret );
    break;

  case CJ_T1_S_KEY:
    if( dir==CJ_T1_S_REQUEST ) block[1]=0xC4;
    else block[1]=0xE4;
    block[2]=0x00;
    *len=3;
    if( (ret=cjT1AppendRc( t1, block, len )) ) return( ret );
    break;

  case CJ_T1_S_CARD:
    if( dir==CJ_T1_S_REQUEST ) block[1]=0xC5;
    else block[1]=0xD5;
    block[2]=0x00;
    *len=3;
    if( (ret=cjT1AppendRc( t1, block, len )) ) return( ret );
    break;

  default:
    return( CJ_EXIT_BAD_PARAM );
  }

  return( CJ_EXIT_OK );
}



/* Builds R-Block */
int cjT1RBlock( CJ_T1_INFO *t1, int type, BYTE *block, int *len ){
  int ret;

  block[0]=t1->nad;
  block[2]=0x00;

  switch( type ) {
  case CJ_T1_R_OK:
    /* Bug workaround */
    /*
     * if( t1->nr ) block[1]=0x90;
     else block[1]=0x80;
     */
    block[1]=0x80;
    break;

  case CJ_T1_R_EDC_ERROR:
    if( t1->nr ) block[1]=0x91;
    else block[1]=0x81;
    break;

  case CJ_T1_R_OTHER_ERROR:
    if( t1->nr ) block[1]=0x92;
    else block[1]=0x82;
    break;

  default:
    return( CJ_EXIT_BAD_PARAM );
  }

  *len=3;
  if( (ret=cjT1AppendRc( t1, block, len )) ) return( ret );

  return( CJ_EXIT_OK );
}



/* Builds I-Block */
int cjT1IBlock( CJ_T1_INFO *t1, BOOLEAN more, const BYTE *data, int datalen,
	BYTE *block, int *blocklen ){
  int ret;

  block[0]=t1->nad;

  block[1]=0x00;
  if( t1->ns ) block[1]|=0x40;
  if( more ) block[1]|=0x20;

  if( datalen>t1->ifsc ) return( CJ_EXIT_BAD_PARAM );
  block[2]=(BYTE) datalen;

  memcpy( block+3, data, datalen );

  *blocklen=datalen+3;
  if( (ret=cjT1AppendRc( t1, block, blocklen )) ) return( ret );

  return( CJ_EXIT_OK );
}



/* Returns N(R) or N(S) from R/I-Block. */
int cjT1GetN( const BYTE *block ){
  /* R-Block */
  if( (block[1]&0xC0)==0x80 ) {
    return( (block[1]>>4)&0x01 );
  }

  /* I-Block */
  if( (block[1]&0x80)==0x00 ) {
    return( (block[1]>>6)&0x01 );
  }

  return( 0 );
}



/* Change IFSD. */
int cjT1ChangeIFSD( CJ_INFO *ci, BYTE ifsd ){
  CJ_T1_INFO *t1;

  BYTE block[ CJ_T1_MAX_SBLKLEN ];
  int blocklen;

  BYTE rblock[ CJ_T1_MAX_SBLKLEN ];
  int rblocklen;

  BOOLEAN success=FALSE;
  int errors=0;
  int ret;

  t1=&ci->t1;

  if( (ret = cjT1SBlock( t1, CJ_T1_S_IFS, CJ_T1_S_REQUEST, ifsd, block,
                        &blocklen )) != CJ_EXIT_OK ) return( ret );

  while( !success ) {
    if( (ret=cjIoSendBlock( ci, block, blocklen )) != CJ_EXIT_OK )
      return( ret );

    rblocklen=sizeof(rblock);
    if( (ret=cjIoReceiveBlock( ci, rblock, &rblocklen )) != CJ_EXIT_OK )
      return( ret );

    if( (rblocklen==blocklen) && (rblock[1]==0xE1) &&
       cjT1CheckRc( t1, rblock, rblocklen ) ) {
      t1->ifsreq=TRUE;
      t1->ifsd=rblock[3];
      success=TRUE;
    }
    else {
      errors++;
    }

    if( errors>2 ) {
      t1->ifsreq=TRUE;
      /* Easy exit. If there is more wrong the following
       * communication will show it.
       */
      success=TRUE;
    }
  }

  return( CJ_EXIT_OK );
}



/* Transmit APDU with protocol T=1 */
int cjT1SendCmd( CJ_INFO *ci, CJ_APDU *apdu ){
  CJ_T1_INFO *t1;

  int sendptr=0;	/* Points to begining of unsent data. */
  int sendlen;

  BYTE block[ CJ_T1_MAX_BLKLEN ];
  int blocklen;

  BYTE block2[ CJ_T1_MAX_BLKLEN ];
  int block2len;

  BYTE rblock[ CJ_T1_MAX_BLKLEN ];
  int rblocklen;

  int rsplen=0;

  BOOLEAN more=TRUE;		/* More data to send. */
  BOOLEAN lastiicc=FALSE;	/* It's ICCs turn to send I-Blocks. */

  int ret;
  int wtxcntr=0;
  int errcntr=0;
  int rerrcntr=0;

  if( (ci==NULL) || (apdu==NULL) ) return( CJ_EXIT_BAD_PARAM );

  t1=&ci->t1;

  ci->t1.nad = apdu->nad;

  apdu->rsplen=0;
  DEBUGP("Sending command to %2x (%d bytes out, max %d bytes in)\n",
         apdu->nad, apdu->cmdlen, apdu->maxRspLen);

#if 0
  /* Change IFSD if not allready changed. */
  if( !t1->ifsreq )
    if( (ret=cjT1ChangeIFSD( ci, 0xFE ))!=CJ_EXIT_OK )
      return( ret );
#endif

  sendlen=min(apdu->cmdlen-sendptr, t1->ifsc);
  if (sendlen==(apdu->cmdlen-sendptr))
    more=FALSE;
  if( (ret=cjT1IBlock(t1, more, apdu->cmd, sendlen, block,
		      &blocklen)) != CJ_EXIT_OK ) {
    DEBUGP("Error creating I-block (%d)", ret);
    return( ret );
  }
  sendptr+=sendlen;

  DEBUGP("Sending R-block");
  if( (ret=cjIoSendBlock( ci, block, blocklen )) != CJ_EXIT_OK ) {
    DEBUGP("Error sending block (%d)", ret);
    return( ret );
  }
  DEBUGP("Block is on its way\n");

  while( TRUE ) {
    rblocklen=sizeof(rblock);
    DEBUGP("Receiving block");
    if( (ret=cjIoReceiveBlock( ci, rblock, &rblocklen )) != CJ_EXIT_OK ) {
      DEBUGP("Error on block receiption (%d), resending", ret);
      if (ret==CJ_EXIT_BAD_PARAM) {
	DEBUGP("Internal error (memory too small)\n");
	return (ret);
      }
      if( (ret=cjT1RBlock( t1, CJ_T1_R_OTHER_ERROR, block2,
                          &block2len ) ) !=CJ_EXIT_OK ) {
	DEBUGP("Error creating R-block (%d)", ret);
        return( ret );
      }

      errcntr++;

      if( errcntr>3 ) {
	DEBUGP("Too many errors, giving up");
	return( CJ_EXIT_IO_ERROR );
      }

      DEBUGP("Sending R-block");
      if( (ret=cjIoSendBlock( ci, block2, block2len )) !=CJ_EXIT_OK ) {
	DEBUGP("Error sending block (%d)", ret);
	return( ret );
      }
      DEBUGP("Block is on its way\n");

      continue;
    }

    /* Wrong length or RC error. */
    if( !cjT1CheckRc( t1, rblock, rblocklen ) ) {
      DEBUGP("Bad T1-block received\n");

      errcntr++;

      if( errcntr>3 ) {
	DEBUGP("Too many errors, giving up");
	return( CJ_EXIT_IO_ERROR );
      }

      if( (ret=cjT1RBlock( t1, CJ_T1_R_EDC_ERROR, block2, &block2len ) )
         !=CJ_EXIT_OK ) {
	DEBUGP("Error creating R-block (%d)", ret);
        return( ret );
      }

      DEBUGP("Resending R-block");
      if( (ret=cjIoSendBlock( ci, block2, block2len ))!=CJ_EXIT_OK ) {
	DEBUGP("Error sending R-block (%d)", ret);
	return( ret );
      }
      DEBUGP("Block is on its way\n");

      continue;
    }

    /* Reset errcntr, because when it is here it had an errorfree
     * transmission.
     */

    errcntr=0;

    /* R-Block */
    if( (rblock[1]&0xC0)==0x80 ) {
      DEBUGP("[R]\n");
      rerrcntr++;

      if( rerrcntr>3 ) {
	DEBUGP("Too many errors, giving up");
	return( CJ_EXIT_IO_ERROR );
      }

      if( lastiicc ) {
        /* Card is sending I-Blocks, so send R-Block. */
        if( (ret=cjT1RBlock( t1, CJ_T1_R_OK, block2,
                            &block2len ) ) != CJ_EXIT_OK ) {
	  DEBUGP("Error creating R-block (%d)", ret);
	  return( ret );
	}

	DEBUGP("Resending R block");
	if( (ret=cjIoSendBlock( ci, block2, block2len ))!=CJ_EXIT_OK ) {
	  DEBUGP("Error sending block (%d)", ret);
          return( ret );
        }
	DEBUGP("Block is on its way\n");
      }
      else {
        /* Bug workaround */
        if( /*cjT1GetN( rblock )==t1->ns*/ (rblock[1]&0x0F)!=0x00 ) {
          /* Update PCB in case ns has changed */
          block[1]&=~0x40;
	  if( t1->ns )
	    block[1]|=0x40;
	  /* N(R) is old N(S), so resend I-Block. */
	  DEBUGP("Resending R block");
	  if( (ret=cjIoSendBlock( ci, block, blocklen ))!= CJ_EXIT_OK ) {
	    DEBUGP("Error sending block (%d)", ret);
	    return( ret );
	  }
	  DEBUGP("Block is on its way\n");
        }
        else {
          /* N(R) is next N(S), so make next I-Block and send it. */

          /* Check if data available. */
          if( more==FALSE ) {
	    DEBUGP("Nothing more to send");
	    return( CJ_EXIT_PROTOCOL_ERROR );
	  }

          /* Change N(S) to new value. */
          t1->ns^=1;

          /* Make next I-Block. */
          sendlen=min( apdu->cmdlen-sendptr, t1->ifsc );
          if( sendlen==(apdu->cmdlen-sendptr) ) more=FALSE;
          if( (ret = cjT1IBlock( t1, more, apdu->cmd+sendptr,
                                sendlen, block, &blocklen )) != CJ_EXIT_OK ) {
	    DEBUGP("Error creating I-block (%d)", ret);
            return( ret );
          }
          sendptr+=sendlen;

	  /* Send I-Block. */
          DEBUGP("Sending I-block");
	  if( (ret=cjIoSendBlock( ci, block, blocklen ))!= CJ_EXIT_OK ) {
	    DEBUGP("Error sending block (%d)", ret);
	    return( ret );
          }
	  DEBUGP("Block is on its way\n");

          /* Reset rerrcntr, because R-block was an ACK. */
          rerrcntr=0;
	}
      }

      continue;
    }

    /* Reset rerrcntr, because when it is here it had not received an
     * R-Block.
     */

    rerrcntr=0;

    /* I-Block */
    if( (rblock[1]&0x80)==0x00 ) {

      DEBUGP("[I]\n");

      if( !lastiicc ) {
        /* Change N(S) to new value. */
        t1->ns^=1;
        /* Save NAD value to return. */
        apdu->nad=rblock[0];
      }

      lastiicc=TRUE;

      if( cjT1GetN( rblock )!=t1->nr ) {
        /* Card is sending wrong I-Block, so send R-Block. */
        if( (ret=cjT1RBlock( t1, CJ_T1_R_OTHER_ERROR, block2,
                            &block2len ) ) != CJ_EXIT_OK ) {
	  DEBUGP("Error creating R-block (%d)", ret);
          return( ret );
        }

        DEBUGP("Resending R-block");
        if( (ret=cjIoSendBlock( ci, block2, block2len ))!= CJ_EXIT_OK ) {
	  DEBUGP("Error sending block (%d)", ret);
          return( ret );
        }
	DEBUGP("Block is on its way\n");

	continue;
      }

      /* Copy data. */
#if 0
      if( rblock[2]>(CJ_CTAPI_MAX_LENR-rsplen) ) {
	return( CJ_EXIT_PROTOCOL_ERROR );
      }
#endif

      /* Copy response to application buffer */
      if (rsplen+rblock[2]<=apdu->maxRspLen) {
	/* memory buffer ok, continue */
        DEBUGP("Copying % d bytes to offset %d", rblock[2], rsplen);
	memcpy(apdu->rsp+rsplen, rblock+3, rblock[2]);
      }
      rsplen+=rblock[2];

      if( (rblock[1]>>5) & 1 ) {
        /* More data available. */

        /* Change N(R) to new value. */
        t1->nr^=1;

        /* Send R-Block. */
        if( (ret=cjT1RBlock( t1, CJ_T1_R_OK, block2,
                            &block2len ) ) != CJ_EXIT_OK ) {
	  DEBUGP("Error creating R-block (%d)", ret);
	  return( ret );
	}

        DEBUGP("Sending next block");
	if( (ret=cjIoSendBlock( ci, block2, block2len ))!=CJ_EXIT_OK) {
	  DEBUGP("Error sending block (%d)", ret);
	  return( ret );
        }
	DEBUGP("Block is on its way\n");
      }
      else {
        /* Last block. */

        /* Change N(R) to new value. */
        t1->nr^=1;

	if( rsplen<2 ) {
          DEBUGP("Too few bytes received (%d<2)", rsplen);
          return( CJ_EXIT_PROTOCOL_ERROR );
        }

        /* complete operation */
	if (rsplen>apdu->maxRspLen) {
	  /* application buffer was too small,
	   could not store full response */
	  DEBUGP("Buffer too small (%d<%d)", apdu->maxRspLen, rsplen);
	  return CJ_EXIT_BAD_PARAM;
	}

	/* otherwise just store number of bytes read and be done */
	apdu->rsplen=rsplen;
        DEBUGP("Returning %d bytes", rsplen);
        return( CJ_EXIT_OK );
      }

      continue;
    }

    /* S-Block IFS Request */
    if( rblock[1]==0xC1 ) {
      DEBUGP("[F]\n");

      if( (ret=cjT1SBlock( t1, CJ_T1_S_IFS, CJ_T1_S_RESPONSE, rblock[3],
			  block2, &block2len ) ) !=CJ_EXIT_OK ) {
	DEBUGP("Error creating S-block (%d)", ret);
	return( ret );
      }

      DEBUGP("Sending S-block");
      if( (ret=cjIoSendBlock( ci, block2, block2len ))!=CJ_EXIT_OK ) {
	DEBUGP("Error sending block (%d)", ret);
        return( ret );
      }
      DEBUGP("Block is on its way\n");

      t1->ifsc=rblock[3];

      continue;
    }

    /* S-Block ABORT Request */
    if( rblock[1]==0xC2 ) {
      if (rsct_config_get_flags() & CT_FLAGS_DEBUG_READER) {
        DEBUGP("[A]\n");
      }

      if( (ret=cjT1SBlock( t1, CJ_T1_S_ABORT, CJ_T1_S_RESPONSE, 0x00,
                          block2, &block2len ) ) !=CJ_EXIT_OK ) {
	DEBUGP("Error creating S-block (%d)", ret);
        return( ret );
      }

      DEBUGP("Sending S-block");
      if( (ret=cjIoSendBlock( ci, block2, block2len ))!=CJ_EXIT_OK ) {
	DEBUGP("Error sending S-block (%d)", ret);
	return( ret );
      }
      DEBUGP("Block is on its way\n");

      /* Get next block to get sending rights back. (This only helps,
       * when the card sends an R(X)) Better to return to the main loop?
       * I really don't know, why a card sends an ABORT block.
       * I would have made sense, when a card sends after an ABORT
       * of IFS I-Blocks a SW in an I-Block, but in all examples of
       * ISO7816-4 Amd.1 the sender of the aborted chain begins with
       * sending a new (chain of) I-Block(s). So when the ICC ABORTS
       * the IFD it just aborts this command without any status?
       * And when the ICC aborts itself it begins another time with
       * its response?
       */

      rblocklen=sizeof(rblock);
      DEBUGP("Receiving next block");
      if( (ret=cjIoReceiveBlock( ci, rblock, &rblocklen ))!=CJ_EXIT_OK ) {
	DEBUGP("Error receiving next block (%d)", ret);
	return( ret );
      }

      DEBUGP("Aborted by card\n");
      return( CJ_EXIT_UNDEF_ERROR );
    }

    /* S-Block WTX Request */
    if( rblock[1]==0xC3 ) {
      DEBUGP("[W]\n");
      /* Special WTX Request from reader with Len=0 */
      if( rblock[2]==0 ) {
        wtxcntr+=1;

        if( wtxcntr>200 ) {
          /* 2000*BWT has to be enough. This is normally over
           * 2 hours. */
          if( !lastiicc ) {
            /* Change N(S) to new value. This is probably more
             * reliable. */
            t1->ns^=1;
	  }
          DEBUGP("WTX too high");
	  return( CJ_EXIT_PROTOCOL_ERROR );
	}

        if( (ret=cjT1SBlock( t1, CJ_T1_S_WTX, CJ_T1_S_RESPONSE, 0x00,
                            block2, &block2len ) ) !=CJ_EXIT_OK ) {
	  DEBUGP("Error creating S-block (%d)", ret);
          return( ret );
        }

	DEBUGP("Sending S-block");
        if( (ret=cjIoSendBlock( ci, block2, block2len ))!=CJ_EXIT_OK ) {
	  DEBUGP("Error sending block (%d)", ret);
          return( ret );
        }
	DEBUGP("Block is on its way\n");

        /* Wait 1 BWT. */
        t1->wtx=1;
      }
      else {
        wtxcntr+=rblock[3];

        if( wtxcntr>200 ) {
          /* 200*BWT has to be enough. This is normally over 5 minutes. */
          return( CJ_EXIT_PROTOCOL_ERROR );
        }

        if( (ret=cjT1SBlock( t1, CJ_T1_S_WTX, CJ_T1_S_RESPONSE, rblock[3],
                            block2, &block2len ) ) !=CJ_EXIT_OK ) {
	  DEBUGP("Error creating S-block (%d)", ret);
          return( ret );
        }

	DEBUGP("Sending S-block");
        if( (ret=cjIoSendBlock( ci, block2, block2len ))!=CJ_EXIT_OK ) {
	  DEBUGP("Error sending block (%d)", ret);
          return( ret );
        }
	DEBUGP("Block is on its way\n");

        /* Wait (WTX-1)*BWT. Rest is waited at begin of while loop. */
        t1->wtx=rblock[3];
      }

      continue;
    }

    /****************************/
    /* Reader specific S-Blocks */
    /****************************/

    /* S-Block Resynch Request */
    if( rblock[1]==0xC0 ) {
      DEBUGP("[Sy]\n");

      if( (ret=cjT1SBlock( t1, CJ_T1_S_RESYNCH, CJ_T1_S_RESPONSE, 0x00,
			  block2, &block2len ) ) !=CJ_EXIT_OK ) {
	DEBUGP("Error creating S-block (%d)", ret);
	return( ret );
      }

      DEBUGP("Sending S-block");
      if( (ret=cjIoSendBlock( ci, block2, block2len ))!=CJ_EXIT_OK ) {
	DEBUGP("Error sending block (%d)", ret);
        return( ret );
      }
      DEBUGP("Block is on its way\n");

      t1->nr=0;
      t1->ns=0;

      continue;
    }

    /* S-Block Key Pressed */
    if( rblock[1]==0xC4 || rblock[1]==0xF4 ) {
      DEBUGP("[K(%s)]\n", (rblock[1]==0xC4)?"num":"special");

      /* Call key pressed callback */
      if( ci->keycb!=NULL ) {
	/* TODO: Set correct values here */
	ci->keycb(ci, (rblock[1]==0xf4)?CJ_KEY_DIGIT:CJ_KEY_CLEAR);
      }

      if( (ret=cjT1SBlock( t1, CJ_T1_S_KEY, CJ_T1_S_RESPONSE, 0x00,
                          block2, &block2len ) ) !=CJ_EXIT_OK ) {
	DEBUGP("Error creating S-block (%d)", ret);
        return( ret );
      }

      DEBUGP("Sending S-block");
      if( (ret=cjIoSendBlock( ci, block2, block2len ))!=CJ_EXIT_OK ) {
	DEBUGP("Error sending block (%d)", ret);
        return( ret );
      }
      DEBUGP("Block is on its way\n");

      /* Insert callback here */

      continue;
    }

    /* S-Block to throw away */
    if( (rblock[1]==0xE6) || (rblock[1]==0xF6) ) {
      DEBUGP("[F]\n");

      continue;
    }

    /* S-Block Card (not) present */
    if( (rblock[1]==0xE5) || (rblock[1]==0xF5) ) {
      DEBUGP("[C]\n");

      /* This is not really correct, but makes resync faster. */
      if( (ret=cjT1SBlock( t1, CJ_T1_S_CARD, CJ_T1_S_RESPONSE, 0x00,
                          block2, &block2len ) ) !=CJ_EXIT_OK ) {
	DEBUGP("Error creating S-block (%d)", ret);
        return( ret );
      }

      DEBUGP("Sending S-block");
      if( (ret=cjIoSendBlock( ci, block2, block2len ))!= CJ_EXIT_OK ) {
	DEBUGP("Error sending block (%d)", ret);
	return( ret );
      }
      DEBUGP("Block is on its way\n");

      continue;
    }

  }

  /* Ooops! Should never be here. */
  return( CJ_EXIT_UNDEF_ERROR );
}




