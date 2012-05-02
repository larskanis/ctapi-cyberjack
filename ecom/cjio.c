/***************************************************************************
 * CT-API library for the REINER SCT cyberJack pinpad/e-com USB.
 * Copyright (C) 2001-2005  REINER SCT
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
 * File: cjio.c
 * CVS: $Id: cjio.c 54 2007-03-13 22:16:21Z martin $
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
/* #include <sys/stat.h> */
#include <sys/time.h>
/* #include <sys/types.h> */
#include <time.h>

#include "cj.h"
#include "cjio.h"
#include "cjio_user.h"
#include "config_l.h"

#include "Debug.h"


#if defined(__linux__)
/* In Linux fcntl is used for device locking.
 * For mandatory locking, the permissions of the device must be set
 * to e.g. 2660. That means, there MUST be no x-bit set AND the setgid-bit
 * MUST be set.
 */
# define USE_FCNTL
#endif


#define cjio_log_bytes_out(cji, hdr, length, value) \
  rsct_log_bytes(cji->ctn, \
                 DEBUG_MASK_COMMUNICATION_OUT, \
                 __FILE__, __LINE__, __FUNCTION__, \
                 hdr, length, value);

#define cjio_log_bytes_in(cji, hdr, length, value) \
  rsct_log_bytes(cji->ctn, \
                 DEBUG_MASK_COMMUNICATION_IN, \
                 __FILE__, __LINE__, __FUNCTION__, \
                 hdr, length, value);




/* Set tty into raw mode.
 * This function is from Stevens' Advanced Programming in the UNIX
 * environment.
 */
static int tty_raw( int fd ){
  struct termios buf;

  if( tcgetattr( fd, &buf ) < 0 )
    return( -1 );

  buf.c_lflag &= ~( ECHO | ICANON | IEXTEN | ISIG );
  buf.c_iflag &= ~( BRKINT | ICRNL | INPCK | ISTRIP | IXON );
  buf.c_cflag &= ~( CSIZE | PARENB );
  buf.c_cflag |= CS8;
  buf.c_oflag &= ~( OPOST );
  buf.c_cc[VMIN] = 1;
  buf.c_cc[VTIME] = 0;

  if( tcsetattr( fd, TCSAFLUSH, &buf ) < 0 )
    return( -1 );

  return( 0 );
}



/* Read data from a file descript with a timeout. */
static ssize_t readt( int fd, void *buf, size_t count, struct timeval timeout ){
  fd_set readset;
  int ret;

  FD_ZERO( &readset );
  FD_SET( fd, &readset );
  ret = select( fd+1, &readset, NULL, NULL, &timeout );
  switch( ret ) {
  case 0:
  case -1:
    return( (ssize_t)ret );
  default:
    break;
  }

  return( read( fd, buf, count ) );
}



/* Open device.
 * ci is a pointer to a CJ_INFO struct.
 * Returns CJ_EXIT_*.
 */
int cjIoOpen(CJ_INFO *ci, int reset_before){
  BYTE buffer[7];
  struct timeval tv;
#if defined(USE_FCNTL)
  struct flock lock;
#endif /* USE_FNCTL */
  int ret, len;

  DEBUGP("trying to open device\n");

  /* Check params */
  if (!ci)
    return CJ_EXIT_BAD_PARAM;

  if (ci->type == CJ_IO_TYPE_LIBUSB)
    return cjIoOpen_libusb(ci, reset_before);

  /* Currently only USB supported */
  if (ci->type != CJ_IO_TYPE_USB)
    return CJ_EXIT_BAD_PARAM;

  /* Open device */
  DEBUGP("Open Device: %s\n", ci->ll.kernel.device);
  ci->ll.kernel.fd=open(ci->ll.kernel.device,
			O_RDWR|O_NOCTTY|O_NONBLOCK);
  if (ci->ll.kernel.fd == -1) {
    DEBUGP("open: errno=%d\n", errno);
    switch (errno) {
    case ENOENT:
      return CJ_EXIT_FILE_NOT_EXIST;
    default:
      return CJ_EXIT_FILE_ERROR;
    }
  }

#if defined(USE_FCNTL)
  /* Lock device */
  DEBUGP("Using F_WRLCK.\n");
  lock.l_type = F_WRLCK;
  lock.l_start = 0;
  lock.l_whence = SEEK_SET;
  lock.l_len = 0;
  lock.l_pid = 0;
  if (fcntl(ci->ll.kernel.fd, F_SETLK, &lock)) {
    DEBUGP("fcntl(F_WRLCK): errno=%d\n", errno);
    close(ci->ll.kernel.fd);
    return CJ_EXIT_FILE_ERROR;
  }
#endif /* USE_FLOCK */

  /* Set to raw mode */
  if (tty_raw(ci->ll.kernel.fd) <0)
    return CJ_EXIT_FILE_ERROR;

  /* Fill CJ_INFO */
  ci->type = CJ_IO_TYPE_USB;
  DEBUGP("reseting ns/nr\n");
  ci->t1.ns = 0;
  ci->t1.nr = 0;
  ci->t1.ifsc = 0xFF;
  ci->t1.ifsd = 0xFF;
  ci->t1.ifsreq = FALSE;
  ci->t1.bwt = 8000000L;
  ci->t1.wtx = 0L;
  ci->t1.cwt = 100000L;	/* Not really CWT */

  usleep(200000);

  /* Flush line */
  do {
    tv.tv_sec=0; tv.tv_usec=10000;
  } while (readt(ci->ll.kernel.fd, buffer, 1, tv));

  ret = cjIoSendBlock(ci, (unsigned char *)"\xE2\xC1\x00\x23", 4);
  if (ret < 0)
    return ret;

  /* Send a S(RESYCH) to resychronise with reader and to test if there
   * is a appropriate reader at the other end of the line.
   */
  ret = cjIoSendBlock(ci, (unsigned char *)"\xE2\xC0\x00\x22", 4);
  if (ret<0)
    return ret ;

#if 1
  len=sizeof(buffer);
  ret = cjIoReceiveBlock(ci, buffer, &len);
  if (ret<0)
    return ret;
  if (len!=4)
    return CJ_EXIT_IO_ERROR;

  /* Correct response? */
  if (memcmp(buffer, "\x2E\xE0\x00\xCE", 4))
    return CJ_EXIT_IO_ERROR;
#endif
  return CJ_EXIT_OK;
}



/* Close device.
 * ci is a pointer to a CJ_INFO struct.
 * Returns CJ_EXIT_*.
 */
int cjIoClose(CJ_INFO *ci){
#if defined(USE_FCNTL)
  struct flock lock;
#endif /* USE_FNCTL */
  int ret;

  /* Check params */
  if (!ci)
    return CJ_EXIT_BAD_PARAM;

  if (ci->type == CJ_IO_TYPE_LIBUSB)
    return cjIoClose_libusb(ci);

  ret = cjIoSendBlock(ci,
		      (unsigned char *)"\xE2\xC1\x00\x23", 4);
  if (ret < 0)
    return ret;

#if defined(USE_FCNTL)
  /* Unlock device */
  lock.l_type = F_UNLCK;
  lock.l_start = 0;
  lock.l_whence = SEEK_SET;
  lock.l_len = 0;
  lock.l_pid = 0;
  if (fcntl(ci->ll.kernel.fd, F_SETLK, &lock))
    DEBUGP("fcntl(F_UNLCK): errno=%d\n", errno );
#endif /* USE_FLOCK */

  close(ci->ll.kernel.fd);

  return CJ_EXIT_OK;
}



/* Send block to reader.
 * ci is a pointer to a CJ_INFO struct.
 * data contains a pointer to the data to transmit.
 * datalen contains the length of the data to transmit.
 * Returns CJ_EXIT_*.
 */
int cjIoSendBlock(CJ_INFO *ci, BYTE *data, int datalen){
  BYTE buffer[3+3+255+1];
  int ptr, size, ret;
  struct timeval tv;

  /* Check params */
  if (!ci || !data || (datalen>(3+255+1)))
    return CJ_EXIT_BAD_PARAM;

  if (ci->type == CJ_IO_TYPE_LIBUSB)
    return cjIoSendBlock_libusb(ci, data, datalen);

  cjio_log_bytes_out(ci, "Send", datalen, data);

  /* Flush line */
  do {
    tv.tv_sec=0; tv.tv_usec=10000;
  } while (readt(ci->ll.kernel.fd, buffer, 1, tv) );

  buffer[0] = 0x00;
  /* Quick fix. 1-byte blocks are not possible with T=1. */
  if( datalen==1 ) buffer[0]=0xFF;
  buffer[1] = datalen & 0xFF;
  buffer[2] = (datalen>>8) & 0xFF;

  memcpy( buffer+3, data, datalen );

  datalen+=3;

  size=datalen;
  ptr=0;

  /* send data in packets of 64 bytes */
  while(size>0) {
    DEBUGP("write(buffer+%d, %d)\n", ptr, size);
    ret=write(ci->ll.kernel.fd, buffer+ptr, size);
    if (ret<0) {
      DEBUGP("Could not write: %s\n", strerror(errno));
      if (errno!=EINTR)
	/* only abort if not EINTR */
	return CJ_EXIT_IO_ERROR;
    }
    else if (ret==0) {
      DEBUGP("Nothing written, will abort\n");
      return CJ_EXIT_IO_ERROR;
    }
    else {
      DEBUGP("%d bytes written\n", ret);
      size-=ret;
      ptr+=ret;
    }
  } /* while */


#if 0 /* disabled, we now send all in one block */
  for( ptr=0; ptr<datalen; ptr+=64 ) {
    size=min(64,datalen-ptr);

    DEBUGP("write(buffer+%d,%d)\n", ptr, size);
    if ((ret = write(ci->ll.kernel.fd, buffer+ptr, size ))!=size ) {
      DEBUGP("write(buffer+%d,%d) sent %d bytes\n",
	     ptr, size, ret );
      if (ret<0) {
	DEBUGP("Could not write: %d\n", strerror(errno));
      }
      return CJ_EXIT_IO_ERROR;
    }
  }
#endif

  return( CJ_EXIT_OK );
}



/* Receive block from reader.
 * ci is a pointer to a CJ_INFO struct.
 * data contains a pointer to the buffer, which should receive the data.
 * datalen contains a pointer to an int, which must contain on calling the
 * size of the buffer and which returns the length of data received.
 * Returns CJ_EXIT_*.
 */
int cjIoReceiveBlock( CJ_INFO *ci, BYTE *data, int *datalen ) {
  struct timeval tv;
  BYTE buffer[5*64];
  int ret, len=0;
  LONG tmpbwt;

  /* Check params */
  if (!ci || !data || !datalen)
    return CJ_EXIT_BAD_PARAM;

  if (ci->type == CJ_IO_TYPE_LIBUSB)
    return cjIoReceiveBlock_libusb(ci, data, datalen);

  usleep(20);

  if (ci->t1.wtx)
    tmpbwt = ci->t1.bwt*ci->t1.wtx;
  else
    tmpbwt = ci->t1.bwt;
  ci->t1.wtx = 0;

  /* Wait for firt data with BWT */
  tv.tv_sec=tmpbwt/1000000L;
  tv.tv_usec=tmpbwt%1000000L;
  DEBUGP("tv_sec=%d, tv_usec=%d\n", (int)tv.tv_sec, (int)tv.tv_usec);

  /* Read */
  DEBUGP("read(buffer,1)\n");
  while( (ret=readt( ci->ll.kernel.fd, buffer+len, 1, tv ))==1 ) {
    len+=ret;
    if (len>(3+3+255+1))
      return CJ_EXIT_PROTOCOL_ERROR;

    if (len >= 3) {
      short size = ((short)buffer[2]<<8)+buffer[1]+3;

      if (len >= size)
	break;
    }

    /* Wait for next data with CWT (Is not really CWT.) */
    tv.tv_sec=ci->t1.cwt/1000000L;
    tv.tv_usec=ci->t1.cwt%1000000L;

    DEBUGP("read(buffer+%d,1)\n", len);
    /* Read rest of data */
  }

  len+=ret;

  if (len == 0)
    return CJ_EXIT_TIMEOUT;

  /* Data length */
  *datalen = ((int)buffer[2]<<8) + buffer[1];
  if ((*datalen)>(3+255+1))
    return CJ_EXIT_PROTOCOL_ERROR;

  /* Copy data */
  memcpy(data, buffer+3, *datalen);

  cjio_log_bytes_in(ci, "Received", *datalen, data );

  return CJ_EXIT_OK;
}



