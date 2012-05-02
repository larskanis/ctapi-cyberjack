/***************************************************************************
    begin       : Wed Jun 16 2010
    copyright   : (C) 2010 by Martin Preuss
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


#include "Platform.h"


#include "network.h"
#include "message.h"

#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include <netdb.h>


/* enable or disable error messages */
#define ENABLE_DEBUGPE

/* enable or disable info messages */
/*#define ENABLE_DEBUGPI*/

/* enable or disable debug messages */
/*#define ENABLE_DEBUGPD*/


#ifdef ENABLE_DEBUGPE
# define DEBUGPE(format, args...) \
    fprintf(stderr, __FILE__":%5d:" format, __LINE__, ## args);
#else
# define DEBUGPE(format, args...)
#endif


#ifdef ENABLE_DEBUGPI
# define DEBUGPI(format, args...) \
    fprintf(stderr, __FILE__":%5d:" format, __LINE__, ## args);
#else
# define DEBUGPI(format, args...)
#endif


#ifdef ENABLE_DEBUGPD
# define DEBUGPD(format, args...) \
    fprintf(stderr, __FILE__":%5d:" format, __LINE__, ## args);
#else
# define DEBUGPD(format, args...)
#endif





int rsct_net_listen_by_ip(const char *ip, int port) {
  union {
    struct sockaddr raw;
    struct sockaddr_in in;
  } addr;
  int s;
  int fl;

  memset(&addr, 0, sizeof(addr));
#if defined(PF_INET)
  addr.raw.sa_family=PF_INET;
#elif defined (AF_INET)
  addr.raw.sa_family=AF_INET;
#endif
  if (!inet_aton(ip, &addr.in.sin_addr)) {
    DEBUGPE("ERROR: inet_aton(): %d=%s\n", errno, strerror(errno));
    return -1;
  }
  addr.in.sin_port=htons(port);

#if defined(PF_INET)
  s=socket(PF_INET, SOCK_STREAM,0);
#elif defined (AF_INET)
  s=socket(AF_INET, SOCK_STREAM,0);
#endif
  if (s==-1) {
    DEBUGPE("ERROR: socket(): %d=%s\n", errno, strerror(errno));
    return -1;
  }

  fl=1;
  if (setsockopt(s,
		 SOL_SOCKET,
		 SO_REUSEADDR,
		 &fl,
		 sizeof(fl))) {
    DEBUGPE("ERROR: setsockopt(): %s", strerror(errno));
    return -1;
  }

  if (bind(s, &addr.raw, sizeof(struct sockaddr_in))) {
    DEBUGPE("ERROR: bind(): %d=%s\n", errno, strerror(errno));
    close(s);
    return -1;
  }

  if (listen(s, 10)) {
    DEBUGPE("ERROR: listen(): %d=%s\n", errno, strerror(errno));
    close(s);
    return -1;
  }

  return s;
}



int rsct_net_listen_by_path(const char *name) {
  union {
    struct sockaddr addr;
    struct sockaddr_un un;
    char bin[512];
  } a;
  int s;

  memset(&a.bin, 0, sizeof(a.bin));
#if defined(PF_UNIX)
  a.addr.sa_family=PF_UNIX;
#elif defined (AF_UNIX)
  a.addr.sa_family=AF_UNIX;
#endif

#ifdef PF_UNIX
  a.un.sun_family=PF_UNIX;
#elif defined (AF_UNIX)
  a.un.sun_family=AF_UNIX;
#else
  fprintf(stderr, "No unix domain sockets available for this system\n");
  return -1;
#endif
  a.un.sun_path[0]=0;

  if (name) {
    /* ok, address to be set */
    if ((strlen(name)+1)>sizeof(a.un.sun_path)) {
      /* bad address */
      fprintf(stderr, "Path too long (%d>%d)\n",
	      (int)(strlen(name)+1),(int)(sizeof(a.un.sun_path)));
      return -1;
    }
    strcpy(a.un.sun_path, name);
  }

  /* create socket */
#if defined(PF_UNIX)
  s=socket(PF_UNIX, SOCK_STREAM, 0);
#elif defined (AF_UNIX)
  s=socket(AF_UNIX, SOCK_STREAM, 0);
#endif
  if (s==-1) {
    fprintf(stderr, "ERROR: socket(): %d=%s\n", errno, strerror(errno));
    return -1;
  }

  if (bind(s, &a.addr, sizeof(struct sockaddr_un))) {
    fprintf(stderr, "ERROR: bind(): %d=%s\n", errno, strerror(errno));
    close(s);
    return -1;
  }

  if (listen(s, 10)) {
    fprintf(stderr, "ERROR: listen(): %d=%s\n", errno, strerror(errno));
    close(s);
    return -1;
  }

  return s;
}



int rsct_net_accept(int sk) {
  socklen_t addrLen;
  int newS;
  struct sockaddr peerAddr;

  addrLen=sizeof(peerAddr);
  newS=accept(sk, &peerAddr, &addrLen);
  if (newS!=-1)
    return newS;
  else {
    if (errno!=EINTR) {
      DEBUGPE("ERROR: accept(): %d=%s\n", errno, strerror(errno));
    }
    return -1;
  }
}



int rsct_net_connect_by_ip(const char *ip, int port) {
  union {
    struct sockaddr raw;
    struct sockaddr_in in;
  } addr;
  int s;

  memset(&addr, 0, sizeof(addr));
#if defined(PF_INET)
  addr.raw.sa_family=PF_INET;
#elif defined (AF_INET)
  addr.raw.sa_family=AF_INET;
#endif
  if (!inet_aton(ip, &addr.in.sin_addr)) {
    struct hostent *he;

    he=gethostbyname(ip);
    if (!he) {
      DEBUGPE("ERROR: gethostbyname(%s): %d=%s\n", ip, errno, strerror(errno));
      return -1;
    }
    memcpy(&(addr.in.sin_addr),
	   he->h_addr_list[0],
	   sizeof(struct in_addr));
  }
  addr.in.sin_port=htons(port);

#if defined(PF_INET)
  s=socket(PF_INET, SOCK_STREAM,0);
#elif defined (AF_INET)
  s=socket(AF_INET, SOCK_STREAM,0);
#endif
  if (s==-1) {
    DEBUGPE("ERROR: socket(): %d=%s\n", errno, strerror(errno));
    return -1;
  }

  if (connect(s, &addr.raw, sizeof(struct sockaddr_in))) {
    DEBUGPE("ERROR: connect(): %d=%s\n", errno, strerror(errno));
    close(s);
    return -1;
  }

  return s;
}



int rsct_net_connect_by_path(const char *name) {
  union {
    struct sockaddr addr;
    struct sockaddr_un un;
    char bin[512];
  } a;
  int s;

  memset(&a.bin, 0, sizeof(a.bin));
#if defined(PF_UNIX)
  a.addr.sa_family=PF_UNIX;
#elif defined (AF_UNIX)
  a.addr.sa_family=AF_UNIX;
#endif

#ifdef PF_UNIX
  a.un.sun_family=PF_UNIX;
#elif defined (AF_UNIX)
  a.un.sun_family=AF_UNIX;
#else
  fprintf(stderr, "No unix domain sockets available for this system\n");
  return -1;
#endif
  a.un.sun_path[0]=0;

  if (name) {
    /* ok, address to be set */
    if ((strlen(name)+1)>sizeof(a.un.sun_path)) {
      /* bad address */
      fprintf(stderr, "Path too long (%d>%d)\n",
	      (int)(strlen(name)+1),(int)(sizeof(a.un.sun_path)));
      return -1;
    }
    strcpy(a.un.sun_path,name);
  }

  /* create socket */
#if defined(PF_UNIX)
  s=socket(PF_UNIX, SOCK_STREAM, 0);
#elif defined (AF_UNIX)
  s=socket(AF_UNIX, SOCK_STREAM, 0);
#endif
  if (s==-1) {
    fprintf(stderr, "ERROR: socket(): %d=%s\n", errno, strerror(errno));
    return -1;
  }

  if (connect(s, &a.addr, sizeof(struct sockaddr_un))) {
    fprintf(stderr, "ERROR: connect(): %d=%s\n", errno, strerror(errno));
    close(s);
    return -1;
  }

  return s;
}



int rsct_net_recv(int sk, RSCT_MESSAGE *msg) {
  char *p;
  int bytesRead;

  assert(msg);

  bytesRead=0;
  p=(char*) msg;

  /* read header */
  while(bytesRead<sizeof(RSCT_MESSAGE_HEADER)) {
    ssize_t i;

    i=sizeof(RSCT_MESSAGE_HEADER)-bytesRead;
    i=read(sk, p, i);
    if (i<0) {
      if (errno!=EINTR) {
	DEBUGPE("ERROR: read(): %d=%s\n", errno, strerror(errno));
	return -1;
      }
    }
    else if (i==0) {
      if (bytesRead==0) {
	DEBUGPI("INFO: peer disconnected (header)\n");
        return 0;
      }
      else {
	DEBUGPE("ERROR: eof met prematurely (header, %d bytes)\n", bytesRead);
      }
      return -1;
    }
    bytesRead+=i;
    p+=i;
    DEBUGPI("INFO: Received %d bytes (header)\n", bytesRead);
  }

  /* check length */
  if (msg->header.len>=RSCT_MAX_MESSAGE_LEN) {
    DEBUGPE("ERROR: Request too long (%d bytes)\n", msg->header.len);
    return -1;
  }
  DEBUGPI("INFO: Message has %d bytes (type: %s)\n",
	 msg->header.len,
	  RSCT_MessageTypeToString(msg->header.type));

  /* read payload */
  while(bytesRead<msg->header.len) {
    ssize_t i;

    i=msg->header.len-bytesRead;
    i=read(sk, p, i);
    if (i<0) {
      DEBUGPE("ERROR: read(): %d=%s\n", errno, strerror(errno));
      return -1;
    }
    else if (i==0) {
      DEBUGPE("ERROR: eof met prematurely (payload, %d bytes)\n",
	     bytesRead);
      return -1;
    }
    bytesRead+=i;
    p+=i;
    DEBUGPI("INFO: Received %d bytes (payload)\n", bytesRead);
  }

  return bytesRead;
}



int rsct_net_send(int sk, const RSCT_MESSAGE *msg) {
  int bytesLeft;
  const uint8_t *p;

  /* send */
  DEBUGPI("INFO: Sending %d bytes (type %s)\n",
	 msg->header.len,
	 RSCT_MessageTypeToString(msg->header.type));

  bytesLeft=msg->header.len;
  p=(const uint8_t*) msg;

  while(bytesLeft) {
    ssize_t i;

    i=send(sk, (const void*)p, bytesLeft, 0);

    /* evaluate */
    if (i<0) {
      if (errno!=EINTR) {
	DEBUGPE("ERROR: send(): %d=%s\n", errno, strerror(errno));
	return -1;
      }
    }
    else if (i==0) {
    }
    else {
      bytesLeft-=(int) i;
      p+=i;
    }
  }

  return 0;
}



