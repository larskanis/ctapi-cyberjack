/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: libtest.c 2007-04-23 10:27:17Z martin $
    begin       : Mon Apr 23 2007
    copyright   : (C) 2007 by Martin Preuss
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


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


#include "cyberjack_l.h"


#define CT   1
#define HOST 2


int test1( int argc, char **argv){
  unsigned char dad, sad, rsp[512];
  unsigned short ctn=1, lr;
  unsigned char apdu[256];
  unsigned int alen;
  char ret;
  int i;
  int pn;
  int x;
  int checkSum=-1;
  time_t t0;

  if (argc<2)
    pn=1;
  else
    pn=atoi(argv[1]);

  /* Initialise CT-API library */
  ret=CT_init(ctn, pn);
  printf( "CT_init: %d\n", ret);
  if (ret!=CT_API_RV_OK )
    return 1;

  /* REQUEST ICC (timeout 20s) */
  dad=CT;
  sad=HOST;
  alen=0;
  apdu[alen++]=0x20;
  apdu[alen++]=0x12;
  apdu[alen++]=0x01; /* Unit */
  apdu[alen++]=0x01; /* request ATR */

  apdu[alen++]=0x00;
  lr=sizeof(rsp);
  ret=CT_data( ctn, &dad, &sad,
	      alen, apdu,
	      &lr, rsp );
  printf( "CT_data: %d\n", ret );
  if(ret!=CT_API_RV_OK)
    return(1);
  printf("    sad: %d, dad: %d, rsp:", sad, dad );
  for( i=0; i<lr; i++ ) printf( " %.2X", rsp[i] );
  printf( "\n" );

  if (rsp[lr-2]!=0x90) {
      return 1;
  }

  t0=time(NULL);
  for (x=0; ; x++) {
    uint8_t c;
    time_t t1;
    int dt;

    memset(rsp, 0, sizeof(rsp));

    /* read KVK */
    dad=CT;
    sad=HOST;
    alen=0;
    apdu[alen++]=0x00;
    apdu[alen++]=0xb0;
    apdu[alen++]=0x00;
    apdu[alen++]=0x00;
    apdu[alen++]=0x00;
    lr=sizeof(rsp);

    ret=CT_data( ctn, &dad, &sad,
		alen, apdu,
		&lr, rsp );
    if(ret!=CT_API_RV_OK) {
      printf( "CT_data: %d\n", ret );
      return(1);
    }
    c=0;
    for( i=0; i<lr; i++ ) {
      c^=rsp[i];
    }
    if (checkSum==-1) {
      checkSum=(int)c;
    }
    else {
      if (checkSum!=(int)c) {
	fprintf(stderr, "CheckSum-Error!\n");
	for (i=0; i<lr; i++ ) printf( " %.2X", rsp[i] );
	printf( "\n" );
        return 2;
      }
    }

    t1=time(NULL);
    dt=difftime(t1, t0);
    fprintf(stderr, "%02d:%02d: checksum=%02x (%d bytes)\n",
	    dt/60, dt%60, c, lr);
    if (dt>(30*60))
      break;
  }

  CT_close(ctn);
  fprintf(stderr, "Success.\n");
  return 0;
}




int main( int argc, char **argv){
#if 1
  return test1(argc, argv);
#else
  unsigned char dad, sad, rsp[258];
  unsigned short ctn=1, lr;
  unsigned char apdu[256];
  unsigned int alen;
  char ret;
  int i;
  int x;
  int pn;

  if (argc<2)
    pn=1;
  else
    pn=atoi(argv[1]);

  for (x=0; x<2; x++) {
    /* Initialise CT-API library */
    ret=CT_init(ctn, pn);
    printf( "CT_init: %d\n", ret);
    if (ret!=CT_API_RV_OK )
      return 1;

#if 1
    /* RESET CT */
    dad=CT;
    sad=HOST;
    alen=0;
    apdu[alen++]=0x20;
    apdu[alen++]=0x11;
    apdu[alen++]=0x00;
    apdu[alen++]=0x00;
    apdu[alen++]=0x00;
    lr=sizeof(rsp);
    ret=CT_data(ctn, &dad, &sad, alen, apdu,
		&lr, rsp);
    printf("CT_data: %d\n", ret);
    if( ret!=CT_API_RV_OK ) return( 1 );
    printf( "    sad: %d, dad: %d, rsp:", sad, dad );
    for( i=0; i<lr; i++ ) printf( " %.2X", rsp[i] );
    printf( "\n" );
#endif

    /* REQUEST ICC (timeout 20s) */
    dad=CT;
    sad=HOST;
    alen=0;
    apdu[alen++]=0x20;
    apdu[alen++]=0x12;
    apdu[alen++]=0x01; /* Unit */
    apdu[alen++]=0x01; /* request ATR */
#if 0
    apdu[alen++]=0x03;
    apdu[alen++]=0x80;
    apdu[alen++]=0x01;
    apdu[alen++]=0x14;
#endif
    apdu[alen++]=0x00;
    lr=sizeof(rsp);
    ret=CT_data( ctn, &dad, &sad,
		alen, apdu, 
		&lr, rsp );
    printf( "CT_data: %d\n", ret );
    if(ret!=CT_API_RV_OK)
      return(1);
    printf("    sad: %d, dad: %d, rsp:", sad, dad );
    for( i=0; i<lr; i++ ) printf( " %.2X", rsp[i] );
    printf( "\n" );
  
    if (rsp[lr-2]!=0x90) {
      return 1;
    }

    /* modify pin */
#if 0
    dad=CT;
    sad=HOST;
    alen=0;
    apdu[alen++]=0x20;
    apdu[alen++]=0x19;
    apdu[alen++]=0x01;
    apdu[alen++]=0x00;
    apdu[alen++]=0x1a;
    apdu[alen++]=0x52;
    apdu[alen++]=0x18;
    apdu[alen++]=0x01;
    apdu[alen++]=0x06;
    apdu[alen++]=0x0e;
    apdu[alen++]=0x00;
    apdu[alen++]=0x24;
    apdu[alen++]=0x00;
    apdu[alen++]=0x90;
    apdu[alen++]=0x10;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    lr=sizeof(rsp);
#else
  
    /* verify pin */
    dad=CT;
    sad=HOST;
    alen=0;
    apdu[alen++]=0x20;
    apdu[alen++]=0x18;
    apdu[alen++]=0x01;
    apdu[alen++]=0x00;
    apdu[alen++]=0x11;
    apdu[alen++]=0x52;
    apdu[alen++]=0x0f;
    apdu[alen++]=0x01;
    apdu[alen++]=0x06;
    apdu[alen++]=0x00;
    apdu[alen++]=0x20;
    apdu[alen++]=0x00;
    apdu[alen++]=0x90;
    apdu[alen++]=0x08;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    apdu[alen++]=0x20;
    lr=sizeof(rsp);
#endif
  
    ret=CT_data( ctn, &dad, &sad,
		alen, apdu,
		&lr, rsp );
    printf( "CT_data: %d\n", ret );
    if(ret!=CT_API_RV_OK)
      return(1);
    printf("    sad: %d, dad: %d, rsp:", sad, dad );
    for( i=0; i<lr; i++ ) printf( " %.2X", rsp[i] );
    printf( "\n" );


    CT_close(ctn);

    fprintf(stderr, "Hit the ENTER key:\n");
    getchar();
  }

  return 0;
#endif
}







