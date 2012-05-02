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
 * File: cjgeldkarte.c
 * CVS: $Id: cjgeldkarte.c 49 2007-01-09 19:55:23Z martin $
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <inttypes.h>

#define CT_API_AD_HOST		2
#define CT_API_RV_OK		0
#define CT_API_RV_ERR_INVALID	-1
#define CT_API_RV_ERR_CT	-8
#define CT_API_RV_ERR_TRANS	-10
#define CT_API_RV_ERR_MEMORY	-11
#define CT_API_RV_ERR_HOST	-127
#define CT_API_RV_ERR_HTSI	-128

#define CT_API_AD_CT		1
#define CT_API_AD_ICC1		0


/* Global function pointers. */
typedef signed char (*CT_INIT)( uint16_t ctn, uint16_t pn );
typedef signed char (*CT_DATA)( uint16_t ctn, uint8_t *dad, uint8_t *sad, uint16_t lenc,
	uint8_t *command, uint16_t *lenr, uint8_t *response );
typedef signed char (*CT_CLOSE)( uint16_t ctn );
static CT_INIT pCT_init = NULL;
static CT_DATA pCT_data = NULL;
static CT_CLOSE pCT_close = NULL;

#define CT_init pCT_init
#define CT_data pCT_data
#define CT_close pCT_close


static void *LibHandle = NULL;

static void usage( char *argv0 )
{
	printf("Usage: %s [-l lib] [-p num]\n",argv0);
	printf("   -l lib    CT-API library, default is %s.\n", LIBRARY_NAME);
	printf("   -p num    Port number (1-n), default is 1.\n");
	printf("   -c count  Number of consecutive reads, default is 1.\n");
	exit(1);
}

#define SENDCMD(dadv,cmdv,cmdlenv) \
	memcpy( cmd, cmdv, cmdlenv ); \
	lenc=cmdlenv; lenr=sizeof(rsp); sad=2; dad=dadv; \
	ret = CT_data( ctn, &dad, &sad, lenc, cmd, &lenr, rsp ); \
	if( ret!=CT_API_RV_OK ) goto err_return;

static int read_geldkarte(uint16_t ctn)
{
	uint8_t cmd[5+255+1], rsp[256+2];
	uint8_t sad, dad;
	uint16_t lenc, lenr=256+2;
	int8_t ret;

	/* Reset ICC */
	SENDCMD(CT_API_AD_CT, "\x20\x11\x01\x01\x00", 5 );
	if( (ret!=CT_API_RV_OK) || (sad!=CT_API_AD_CT) ) {
		printf( "Error sending command to reader. (Return code:%d)\n", ret );
		SENDCMD( CT_API_AD_CT, "\x20\x15\x01\x00", 5 );
		sleep(2);
		return 3;
	}

	if( lenr==24+2 ) {
		/* Geldkarte v2? */
		if( memcmp( rsp+7, "\x45\x65\x63", 3 ) ) {
			printf( "Unknown card.\n");
			goto err_icccmd;
		}

		/* Select AID */
		SENDCMD( CT_API_AD_ICC1, "\x00\xA4\x04\x0C\x09\xD2\x76\x00\x00\x25\x45\x50\x01\x00", 14 );
		if( sad!=CT_API_AD_ICC1 ) goto err_sad;
		if( (lenr!=2) || (rsp[0]!=0x90) || (rsp[1]!=0x00) ) {
			printf("Error selecting GeldKarte application.\n");
			goto err_icccmd;
		}

		/* Read Record */
		SENDCMD( CT_API_AD_ICC1, "\x00\xB2\x01\xC4\x09", 5 );
		if( sad!=CT_API_AD_ICC1 ) goto err_sad;
		if( (lenr<5) || (rsp[lenr-2]!=0x90) || (rsp[lenr-1]!=0x00) ) {
			printf("Error reading account balance.\n");
			goto err_icccmd;
		}
	} else if( lenr==25+2 ) {
		/* Geldkarte v3? */
		if( memcmp( rsp+8, "\x45\x65\x63", 3 ) ) {
			printf( "Unknown card.\n");
			goto err_icccmd;
		}

		/* Select AID */
		SENDCMD( CT_API_AD_ICC1, "\x00\xA4\x04\x0C\x09\xD2\x76\x00\x00\x25\x45\x50\x02\x00", 14 );
		if( sad!=CT_API_AD_ICC1 ) goto err_sad;
		if( (lenr!=2) || (rsp[0]!=0x90) || (rsp[1]!=0x00) ) {
			printf("Error selecting GeldKarte application.\n");
			goto err_icccmd;
		}

		/* Read Record */
		SENDCMD( CT_API_AD_ICC1, "\x00\xB2\x01\xC4\x00", 5 );
		if( sad!=CT_API_AD_ICC1 ) goto err_sad;
		if( (lenr<5) || (rsp[lenr-2]!=0x90) || (rsp[lenr-1]!=0x00) ) {
			printf("Error reading account balance.\n");
			goto err_icccmd;
		}
	} else if( (lenr>2) && (rsp[lenr-2]==0x90) ) {
		printf( "Unknown card.\n" );
		goto err_icccmd;
	} else {
		printf( "No card.\n" );
		sleep(2);
		return 0;
	}

	printf("Current account balance: %.2X%.2X.%.2X\n", rsp[0], rsp[1], rsp[2] );

err_icccmd:
	SENDCMD( CT_API_AD_CT, "\x20\x15\x01\x00", 5 );
		sleep(2);
	return 0;

err_return:
	printf( "Error sending command to card. (Return code:%d)\n", ret );
		sleep(2);
	return 3;
err_sad:
	printf( "Error sending command to card. (sad:%d)\n", sad );
		sleep(2);
	return 3;
}


#if 0
static int test_ctapi( char *argv0, int pn, int count )
{
  uint16_t ctn=1;
  int8_t ret;

  int i;

  /* Open */
  ret = CT_init( ctn, pn );
  if( ret!=CT_API_RV_OK ) {
    printf("Error doing CT_init. (Return code:%d)\n",ret);
    return 3;
  }

  for (i = 0; i < count; i++) {
    ret = read_geldkarte(ctn);
    if (ret) {
      CT_close(ctn);
      return ret;
    }
  }
  CT_close(ctn);
  return 0;
}
#endif



static int test_ctapi2( char *argv0, int pn, int count )
{
  uint16_t ctn=1;

  int i;

  /* Open */
  for (i = 0; i < count; i++) {
    int8_t ret;

    ret = CT_init( ctn, pn );
    if( ret!=CT_API_RV_OK ) {
      printf("Error doing CT_init. (Return code:%d)\n",ret);
      return 3;
    }

    ret = read_geldkarte(ctn);
    if (ret) {
      CT_close(ctn);
      return ret;
    }
    ret=CT_close(ctn);
    if (ret) {
      return ret;
    }
  }
  return 0;
}



static int test_geldkarte(uint16_t ctn){
  uint8_t cmd[5+255+1], rsp[256+2];
  uint8_t sad, dad;
  uint16_t lenc, lenr=256+2;
  int8_t ret;

  /* Reset ICC */
  SENDCMD( CT_API_AD_CT, "\x20\x11\x01\x01\x00", 5 );
  if( (ret!=CT_API_RV_OK) || (sad!=CT_API_AD_CT) ) {
    printf( "  Error sending command to reader. (Return code:%d)\n", ret );
    SENDCMD( CT_API_AD_CT, "\x20\x15\x01\x00", 5 );
    sleep(2);
    return 3;
  }

  if( lenr==24+2 ) {
    /* Geldkarte v2? */
    if( memcmp( rsp+7, "\x45\x65\x63", 3 ) ) {
      printf( "  Unknown card.\n");
      goto err_icccmd;
    }

    /* Select AID */
    SENDCMD( CT_API_AD_ICC1, "\x00\xA4\x04\x0C\x09\xD2\x76\x00\x00\x25\x45\x50\x01\x00", 14 );
    if( sad!=CT_API_AD_ICC1 ) goto err_sad;
    if( (lenr!=2) || (rsp[0]!=0x90) || (rsp[1]!=0x00) ) {
      printf("  Error selecting GeldKarte application.\n");
      goto err_icccmd;
    }

    /* Read Record */
    SENDCMD( CT_API_AD_ICC1, "\x00\xB2\x01\xC4\x09", 5 );
    if( sad!=CT_API_AD_ICC1 ) goto err_sad;
    if( (lenr<5) || (rsp[lenr-2]!=0x90) || (rsp[lenr-1]!=0x00) ) {
      printf("  Error reading account balance.\n");
      goto err_icccmd;
    }
  } else if( lenr==25+2 ) {
    /* Geldkarte v3? */
    if( memcmp( rsp+8, "\x45\x65\x63", 3 ) ) {
      printf( "  Unknown card.\n");
      goto err_icccmd;
    }

    /* Select AID */
    SENDCMD( CT_API_AD_ICC1, "\x00\xA4\x04\x0C\x09\xD2\x76\x00\x00\x25\x45\x50\x02\x00", 14 );
    if( sad!=CT_API_AD_ICC1 ) goto err_sad;
    if( (lenr!=2) || (rsp[0]!=0x90) || (rsp[1]!=0x00) ) {
      printf("  Error selecting GeldKarte application.\n");
      goto err_icccmd;
    }

    /* Read Record */
    SENDCMD( CT_API_AD_ICC1, "\x00\xB2\x01\xC4\x00", 5 );
    if( sad!=CT_API_AD_ICC1 ) goto err_sad;
    if( (lenr<5) || (rsp[lenr-2]!=0x90) || (rsp[lenr-1]!=0x00) ) {
      printf("  Error reading account balance.\n");
      goto err_icccmd;
    }
  } else if( (lenr>2) && (rsp[lenr-2]==0x90) ) {
    printf( "  Unknown card.\n" );
    goto err_icccmd;
  } else {
    printf( "  No card.\n" );
    sleep(2);
    return 0;
  }

err_icccmd:
  SENDCMD( CT_API_AD_CT, "\x20\x15\x01\x00", 5 );
  sleep(2);
  return 0;

err_return:
  printf( "Error sending command to card. (Return code:%d)\n", ret );
  sleep(2);
  return 3;
err_sad:
  printf( "Error sending command to card. (sad:%d)\n", sad );
  sleep(2);
  return 3;
}



static int multi_test(char *argv0, int pn,
                      int readers,
		      int count ) {
  int i;
  uint16_t ctn=1;
  int errs=0;

  for (i=1; i<=readers; i++) {
    int8_t ret;
    int err=0;

    ret=CT_init(ctn, i);
    if( ret!=CT_API_RV_OK ) {
      printf("  Error doing CT_init. (Return code:%d)\n",ret);
      err=1;
    }
    else {
      int j;

      for (j = 0; j < count; j++) {
	ret = test_geldkarte(ctn);
	if (ret) {
	  err=1;
	  break;
	}
      }
      ret=CT_close(ctn);
      if( ret!=CT_API_RV_OK ) {
	printf("  Error doing CT_init. (Return code:%d)\n",ret);
	err=1;
      }
    }

    if (err) {
      fprintf(stderr, "-> Reader %d: FAILED\n", i);
      errs++;
    }
    else {
      fprintf(stderr, "-> Reader %d: Success\n", i);
    }
  }


  if (errs) {
    if (errs==readers) {
      fprintf(stderr, "FAILED: All tests failed.\n");
    }
    else {
      fprintf(stderr, "FAILED: Some tests failed.\n");
    }
    return 2;
  }

  fprintf(stderr, "SUCCESS: All tests ok.\n");
  return 0;
}



int main(int argc, char **argv){
  uint16_t pn=1;
  int c;
  int ret;
  int count = 1;
  char *buffer=NULL;
  char *ctlib=LIBRARY_NAME;
  int readers=1;

  while ((c = getopt(argc, argv, "l:p:c:r:")) != -1) {
    switch (c) {
    case 'l':
      buffer=strdup(optarg);
      ctlib=buffer;
      break;
    case 'p':
      pn = atoi(optarg);
      break;
    case 'c':
      count = atoi(optarg);
      break;
    case 'r':
      readers = atoi(optarg);
      break;
    default:
      printf("invalid option `%c' (%d)\n", c, c);
      usage( argv[0] );
      if (buffer)
	free(buffer);
      return 1;
    }
  }

  LibHandle = dlopen( ctlib, RTLD_LAZY );
  if( !LibHandle ) {
    printf("Error loading CT-API library.\n");
    if (buffer)
      free(buffer);
    return 1;
  }

  pCT_init = (CT_INIT) dlsym( LibHandle, "CT_init" );
  if( dlerror()!=NULL ) {
    dlclose( LibHandle );
    if (buffer)
      free(buffer);
    printf("Error getting function handle.\n");
    return 1;
  }
  pCT_data = (CT_DATA) dlsym( LibHandle, "CT_data" );
  if( dlerror()!=NULL ) {
    dlclose( LibHandle );
    if (buffer)
      free(buffer);
    printf("Error getting function handle.\n");
    return 1;
  }
  pCT_close = (CT_CLOSE) dlsym( LibHandle, "CT_close" );
  if( dlerror()!=NULL ) {
    dlclose( LibHandle );
    if (buffer)
      free(buffer);
    printf("Error getting function handle.\n");
    return 1;
  }

  if (readers>1)
    ret=multi_test(argv[0], pn, readers, count);
  else
    ret = test_ctapi2(argv[0], pn, count);

  dlclose( LibHandle );
  if (buffer)
    free(buffer);
  return ret;
}




